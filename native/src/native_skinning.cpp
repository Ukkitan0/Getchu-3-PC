#include "ae3/native_skinning.hpp"
#include "cgltf.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ae3 {
namespace {
using V3=std::array<float,3>;
using V4=std::array<float,4>;
using Mat=std::array<float,16>;
constexpr std::size_t maxGlb=512ull*1024*1024;
constexpr std::size_t maxVertices=3'000'000;
Mat identity(){return {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};}
Mat product(const Mat& a,const Mat& b){
    Mat r{};
    for(int col=0;col<4;++col)
        for(int row=0;row<4;++row)
            for(int k=0;k<4;++k)
                r[std::size_t(col*4+row)]+=
                    a[std::size_t(k*4+row)]*b[std::size_t(col*4+k)];
    return r;
}
V3 transform(const Mat& m,V3 p){
    const float w=m[3]*p[0]+m[7]*p[1]+m[11]*p[2]+m[15];
    if(!std::isfinite(w)||std::abs(w)<1e-6f)
        throw std::runtime_error("native glTF skin matrix has invalid homogeneous W");
    return {(m[0]*p[0]+m[4]*p[1]+m[8]*p[2]+m[12])/w,
            (m[1]*p[0]+m[5]*p[1]+m[9]*p[2]+m[13])/w,
            (m[2]*p[0]+m[6]*p[1]+m[10]*p[2]+m[14])/w};
}
float dot(V4 a,V4 b){
    return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3];
}
V4 normalize(V4 a){
    float n=std::sqrt(dot(a,a));
    if(n<1e-6f||!std::isfinite(n))
        throw std::runtime_error("zero/nonfinite native glTF rotation");
    for(float& v:a)v/=n;
    return a;
}
V4 interpolateQuat(V4 a,V4 b,float f){
    a=normalize(a);b=normalize(b);
    float c=dot(a,b);
    if(c<0){c=-c;for(float& v:b)v=-v;}
    c=std::clamp(c,0.f,1.f);
    V4 out{};
    if(c>.9995f){
        for(int i=0;i<4;++i)
            out[std::size_t(i)]=a[std::size_t(i)]*(1-f)+b[std::size_t(i)]*f;
    }else{
        const float theta=std::acos(c),den=std::sin(theta);
        const float w0=std::sin((1-f)*theta)/den,w1=std::sin(f*theta)/den;
        for(int i=0;i<4;++i)
            out[std::size_t(i)]=a[std::size_t(i)]*w0+b[std::size_t(i)]*w1;
    }
    return normalize(out);
}
Mat fromTrs(V3 t,V4 q,V3 s){
    q=normalize(q);
    const float x=q[0],y=q[1],z=q[2],w=q[3];
    Mat m={
        (1-2*(y*y+z*z))*s[0],(2*(x*y+z*w))*s[0],(2*(x*z-y*w))*s[0],0,
        (2*(x*y-z*w))*s[1],(1-2*(x*x+z*z))*s[1],(2*(y*z+x*w))*s[1],0,
        (2*(x*z+y*w))*s[2],(2*(y*z-x*w))*s[2],(1-2*(x*x+y*y))*s[2],0,
        t[0],t[1],t[2],1
    };
    return m;
}
V3 subtract(V3 a,V3 b){return {a[0]-b[0],a[1]-b[1],a[2]-b[2]};}
V3 cross(V3 a,V3 b){
    return {a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};
}
float squared(V3 a){return a[0]*a[0]+a[1]*a[1]+a[2]*a[2];}
int nodeIndex(const cgltf_data* data,const cgltf_node* n){
    if(!n||n<data->nodes||n>=data->nodes+data->nodes_count)
        throw std::runtime_error("glTF node pointer not in validated node set");
    return static_cast<int>(n-data->nodes);
}
std::vector<std::uint8_t> readBytes(const std::filesystem::path& p){
    std::error_code ec;
    auto length=std::filesystem::file_size(p,ec);
    if(ec||length<20||length>maxGlb)
        throw std::runtime_error("animated native GLB file missing or oversized");
    std::ifstream stream(p,std::ios::binary);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
    if(!stream.read(reinterpret_cast<char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size())))
        throw std::runtime_error("animated native GLB short file read");
    return bytes;
}
} // namespace

NativeSkinnedGlb::NativeSkinnedGlb(const std::filesystem::path& file) {
    // Preserve owner-private ae3-sdk PNG URI references with the same
    // original static scene loader; animation reads embedded GLB only.
    const auto bytes=readBytes(file);
    scene_=loadStaticGlb(file);
    load(bytes);
}
NativeSkinnedGlb::NativeSkinnedGlb(const std::vector<std::uint8_t>& data) {
    scene_=loadStaticGlbBytes(data);
    load(data);
}
void NativeSkinnedGlb::load(const std::vector<std::uint8_t>& bytes){
    cgltf_options opts{};
    cgltf_data* raw=nullptr;
    if(cgltf_parse(&opts,bytes.data(),bytes.size(),&raw)!=cgltf_result_success)
        throw std::runtime_error("cannot parse glTF skin/animation GLB");
    std::unique_ptr<cgltf_data,decltype(&cgltf_free)> data(raw,cgltf_free);
    if(data->file_type!=cgltf_file_type_glb ||
       cgltf_validate(data.get())!=cgltf_result_success)
        throw std::runtime_error("native animation source failed structural GLB validation");
    for(cgltf_size i=0;i<data->buffers_count;++i)
        if(data->buffers[i].uri&&data->buffers[i].uri[0])
            throw std::runtime_error("external glTF animation buffers not accepted");
    if(cgltf_load_buffers(&opts,data.get(),nullptr)!=cgltf_result_success)
        throw std::runtime_error("native animation GLB buffer unavailable");
    if(data->nodes_count>30000 || data->skins_count>1000 ||
       data->animations_count>1000)
        throw std::runtime_error("native animation resource budget exceeded");
    nodes_.reserve(data->nodes_count);
    for(cgltf_size i=0;i<data->nodes_count;++i){
        const auto& n=data->nodes[i];
        Node node;
        if(n.parent)node.parent=nodeIndex(data.get(),n.parent);
        for(int k=0;k<3;++k){
            node.position[std::size_t(k)]=n.translation[k];
            node.scale[std::size_t(k)]=n.scale[k];
        }
        for(int k=0;k<4;++k)node.rotation[std::size_t(k)]=n.rotation[k];
        node.hasMatrix=n.has_matrix;
        if(n.has_matrix)
            for(int k=0;k<16;++k)node.explicitMatrix[std::size_t(k)]=n.matrix[k];
        nodes_.push_back(node);
    }
    skins_.reserve(data->skins_count);
    for(cgltf_size s=0;s<data->skins_count;++s){
        const auto& cgskin=data->skins[s];
        if(cgskin.joints_count>1024)
            throw std::runtime_error("native original game skeleton has too many joints");
        Skin skin;
        for(cgltf_size j=0;j<cgskin.joints_count;++j){
            skin.nodes.push_back(nodeIndex(data.get(),cgskin.joints[j]));
            Mat inverse=identity();
            if(cgskin.inverse_bind_matrices){
                float tmp[16]{};
                if(j>=cgskin.inverse_bind_matrices->count||
                   !cgltf_accessor_read_float(cgskin.inverse_bind_matrices,j,tmp,16))
                    throw std::runtime_error("original skin inverse bind matrix invalid");
                std::copy(tmp,tmp+16,inverse.begin());
            }
            skin.inverseBind.push_back(inverse);
        }
        totalJointCount_+=skin.nodes.size();
        skins_.push_back(std::move(skin));
    }
    // Build a vertex correspondence in exactly the same traversal and
    // triangle order as native/src/gltf_scene.cpp (GLB conversion flips
    // glTF RH winding to D3D LH and omits zero-area triangles).
    cgltf_scene* root=data->scene?data->scene:
        (data->scenes_count?&data->scenes[0]:nullptr);
    if(!root)throw std::runtime_error("animated GLB has no scene");
    std::size_t outputVertex=0;
    std::function<void(const cgltf_node*,unsigned)> visit=
        [&](const cgltf_node* node,unsigned depth){
        if(depth>128)throw std::runtime_error("animation scene hierarchy too deep");
        const int meshNode=nodeIndex(data.get(),node);
        if(node->mesh){
            cgltf_float matrix[16]{};
            cgltf_node_transform_world(node,matrix);
            Mat rest{};
            std::copy(matrix,matrix+16,rest.begin());
            const int skin=node->skin?
                static_cast<int>(node->skin-data->skins):-1;
            if(skin>=0 && static_cast<std::size_t>(skin)>=skins_.size())
                throw std::runtime_error("original skin index outside buffer");
            for(cgltf_size pi=0;pi<node->mesh->primitives_count;++pi){
                const auto& p=node->mesh->primitives[pi];
                if(p.type!=cgltf_primitive_type_triangles)continue;
                const cgltf_accessor *pos=nullptr,*joint=nullptr,*weight=nullptr;
                for(cgltf_size a=0;a<p.attributes_count;++a){
                    const auto& at=p.attributes[a];
                    if(at.type==cgltf_attribute_type_position)pos=at.data;
                    if(at.type==cgltf_attribute_type_joints&&at.index==0)joint=at.data;
                    if(at.type==cgltf_attribute_type_weights&&at.index==0)weight=at.data;
                }
                if(!pos||pos->type!=cgltf_type_vec3)continue;
                const cgltf_size count=p.indices?p.indices->count:pos->count;
                if(count%3 || count>maxVertices)
                    throw std::runtime_error("animated glTF primitive index count malformed");
                if(skin>=0 && (!joint||!weight||
                   joint->type!=cgltf_type_vec4||
                   weight->type!=cgltf_type_vec4))
                    throw std::runtime_error("original skinned mesh is missing JOINTS_0/WEIGHTS_0");
                for(cgltf_size idx=0;idx<count;idx+=3){
                    std::array<V3,3> positions{};
                    std::array<std::array<unsigned,4>,3> joints{};
                    std::array<std::array<float,4>,3> weights{};
                    for(cgltf_size k=0;k<3;++k){
                        cgltf_size vi=p.indices?
                            cgltf_accessor_read_index(p.indices,idx+k):idx+k;
                        if(vi>=pos->count)
                            throw std::runtime_error("animated primitive POSITION out of bounds");
                        float xyz[3]{};
                        if(!cgltf_accessor_read_float(pos,vi,xyz,3))
                            throw std::runtime_error("animated primitive vertex cannot be decoded");
                        positions[std::size_t(k)]={xyz[0],xyz[1],xyz[2]};
                        if(skin>=0){
                            if(vi>=joint->count||vi>=weight->count)
                                throw std::runtime_error("original vertex skin weights out of bounds");
                            cgltf_uint js[4]{};
                            float ws[4]{};
                            if(!cgltf_accessor_read_uint(joint,vi,js,4)||
                               !cgltf_accessor_read_float(weight,vi,ws,4))
                                throw std::runtime_error("original vertex skin influence inaccessible");
                            for(int c=0;c<4;++c){
                                if(js[c]>=skins_[std::size_t(skin)].nodes.size() ||
                                   !std::isfinite(ws[c])||ws[c]<0)
                                    throw std::runtime_error("original game joint weight/index invalid");
                                joints[std::size_t(k)][std::size_t(c)]=js[c];
                                weights[std::size_t(k)][std::size_t(c)]=ws[c];
                            }
                        }
                    }
                    const V3 a=transform(rest,positions[0]);
                    const V3 b=transform(rest,positions[1]);
                    const V3 c=transform(rest,positions[2]);
                    if(std::sqrt(squared(cross(subtract(b,a),subtract(c,a))))<1e-8f)
                        continue;
                    for(int k:{0,2,1}){
                        Binding binding;
                        binding.vertex=static_cast<std::uint32_t>(outputVertex++);
                        binding.local=positions[std::size_t(k)];
                        binding.node=meshNode;binding.skin=skin;
                        binding.joint=joints[std::size_t(k)];
                        binding.weights=weights[std::size_t(k)];
                        if(skin>=0)++skinnedVertexCount_;
                        bindings_.push_back(std::move(binding));
                    }
                }
            }
        }
        for(cgltf_size ch=0;ch<node->children_count;++ch)visit(node->children[ch],depth+1);
    };
    for(cgltf_size i=0;i<root->nodes_count;++i)visit(root->nodes[i],0);
    if(outputVertex!=scene_.triangles.size())
        throw std::runtime_error("native animation/static mesh vertex ordering mismatch");
    for(cgltf_size ci=0;ci<data->animations_count;++ci){
        const auto& anim=data->animations[ci];
        Clip clip;
        for(cgltf_size ch=0;ch<anim.channels_count;++ch){
            const auto& c=anim.channels[ch];
            if(!c.target_node||!c.sampler||!c.sampler->input||!c.sampler->output)
                throw std::runtime_error("native glTF animation channel missing target/sampler");
            if(c.sampler->interpolation==cgltf_interpolation_type_cubic_spline)
                throw std::runtime_error("CUBICSPLINE original animation not supported yet");
            if(c.target_node->has_matrix)
                throw std::runtime_error("glTF animated joint must have TRS, not matrix");
            Channel channel;
            channel.node=nodeIndex(data.get(),c.target_node);
            if(c.target_path==cgltf_animation_path_type_rotation)
                channel.path=Path::rotation;
            else if(c.target_path==cgltf_animation_path_type_translation)
                channel.path=Path::position;
            else if(c.target_path==cgltf_animation_path_type_scale)
                channel.path=Path::scale;
            else throw std::runtime_error("unsupported original animation channel path");
            channel.step=c.sampler->interpolation==cgltf_interpolation_type_step;
            const int dims=channel.path==Path::rotation?4:3;
            const auto* time=c.sampler->input;
            const auto* values=c.sampler->output;
            if(time->count!=values->count || time->count==0 ||
               time->count>100000 || cgltf_num_components(values->type)!=dims)
                throw std::runtime_error("original animation key accessor count/type mismatch");
            channel.times.reserve(time->count);
            channel.values.reserve(time->count);
            float last=-1.f;
            for(cgltf_size ki=0;ki<time->count;++ki){
                float t=0,v[4]{0,0,0,1};
                if(!cgltf_accessor_read_float(time,ki,&t,1) ||
                   !cgltf_accessor_read_float(values,ki,v,dims) ||
                   !std::isfinite(t)||t<0||t>3600||t<=last)
                    throw std::runtime_error("original animation key time invalid or unsorted");
                for(int cidx=0;cidx<dims;++cidx)
                    if(!std::isfinite(v[cidx]) || std::abs(v[cidx])>1e8f)
                        throw std::runtime_error("original animation key contains NaN/outlier");
                channel.times.push_back(t);
                channel.values.push_back({v[0],v[1],v[2],v[3]});
                last=t;
            }
            clip.duration=std::max(clip.duration,static_cast<double>(last));
            clip.channels.push_back(std::move(channel));
        }
        clips_.push_back(std::move(clip));
        clipNames_.push_back(anim.name&&anim.name[0]?anim.name:
                             "native_clip_"+std::to_string(ci));
    }
}
double NativeSkinnedGlb::clipDuration(std::size_t index) const {
    if(index>=clips_.size())throw std::out_of_range("native animation clip index");
    return clips_[index].duration;
}
const StaticScene& NativeSkinnedGlb::sample(std::size_t clipIndex,double time){
    if(clipIndex>=clips_.size()||!std::isfinite(time))
        throw std::runtime_error("invalid native animation clip/time");
    const auto& clip=clips_[clipIndex];
    const float t=clip.duration>0?
        static_cast<float>(std::fmod(std::max(0.0,time),clip.duration)):0.f;
    auto pose=nodes_;
    for(const auto& channel:clip.channels){
        V4 v=channel.values.front();
        if(t>=channel.times.back())
            v=channel.values.back();
        else if(t>channel.times.front()){
            const auto it=std::upper_bound(channel.times.begin(),channel.times.end(),t);
            const std::size_t upper=static_cast<std::size_t>(it-channel.times.begin());
            const std::size_t lower=upper-1;
            const float a=channel.times[lower],b=channel.times[upper];
            float f=channel.step?0.f:(t-a)/(b-a);
            f=std::clamp(f,0.f,1.f);
            if(channel.path==Path::rotation)
                v=interpolateQuat(channel.values[lower],channel.values[upper],f);
            else for(int k=0;k<4;++k)
                v[std::size_t(k)]=channel.values[lower][std::size_t(k)]*(1-f)+
                                    channel.values[upper][std::size_t(k)]*f;
        }
        auto& n=pose[std::size_t(channel.node)];
        if(channel.path==Path::rotation)n.rotation=normalize(v);
        else for(int k=0;k<3;++k){
            if(channel.path==Path::position)n.position[std::size_t(k)]=v[std::size_t(k)];
            else n.scale[std::size_t(k)]=v[std::size_t(k)];
        }
    }
    std::vector<Mat> world(pose.size());
    std::vector<std::uint8_t> visit(pose.size(),0);
    std::function<const Mat&(std::size_t)> resolve=[&](std::size_t i)->const Mat&{
        if(visit[i]==2)return world[i];
        if(visit[i]==1)throw std::runtime_error("animated original skeleton has hierarchy cycle");
        visit[i]=1;
        const auto& n=pose[i];
        Mat local=n.hasMatrix?n.explicitMatrix:fromTrs(n.position,n.rotation,n.scale);
        world[i]=n.parent>=0?
            product(resolve(static_cast<std::size_t>(n.parent)),local):local;
        visit[i]=2;
        return world[i];
    };
    for(std::size_t i=0;i<pose.size();++i)resolve(i);
    std::vector<std::vector<Mat>> jointMatrices;
    jointMatrices.reserve(skins_.size());
    for(const auto& skin:skins_){
        std::vector<Mat> matrices;
        for(std::size_t j=0;j<skin.nodes.size();++j)
            matrices.push_back(product(world[static_cast<std::size_t>(skin.nodes[j])],
                                       skin.inverseBind[j]));
        jointMatrices.push_back(std::move(matrices));
    }
    for(const auto& binding:bindings_){
        V3 p{};
        if(binding.skin<0)
            p=transform(world[std::size_t(binding.node)],binding.local);
        else{
            const auto& skin=jointMatrices[std::size_t(binding.skin)];
            float total=0.f;
            for(int k=0;k<4;++k){
                float w=binding.weights[std::size_t(k)];
                if(w<=0)continue;
                const auto value=transform(skin[binding.joint[std::size_t(k)]],
                                           binding.local);
                for(int a=0;a<3;++a)p[std::size_t(a)]+=value[std::size_t(a)]*w;
                total+=w;
            }
            if(total>1e-6f)for(float& v:p)v/=total;
            else p=transform(world[std::size_t(binding.node)],binding.local);
        }
        auto& dst=scene_.triangles[binding.vertex];
        dst.x=p[0];dst.y=p[1];dst.z=-p[2]; // glTF RH -> D3D LH
        if(!std::isfinite(dst.x)||!std::isfinite(dst.y)||!std::isfinite(dst.z))
            throw std::runtime_error("animated skin vertex position became nonfinite");
    }
    return scene_;
}
} // namespace ae3
