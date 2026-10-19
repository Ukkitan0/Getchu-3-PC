#include "ae3/gltf_scene.hpp"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace ae3 {
namespace {
constexpr std::size_t maxGlbBytes=512ull*1024*1024;
constexpr std::size_t maxHostVertices=3'000'000;
using Point=std::array<float,3>;
float clean(float x){
    if(!std::isfinite(x)||std::abs(x)>1e8f)
        throw std::runtime_error("GLB contains invalid/out-of-range world coordinates");
    return x;
}
Point worldPosition(const cgltf_float* m,const Point& p){
    // glTF uses column-major right-handed coordinates. D3D11 uses a
    // host-native LH camera; invert z after applying full node transforms.
    return {clean(m[0]*p[0]+m[4]*p[1]+m[8]*p[2]+m[12]),
            clean(m[1]*p[0]+m[5]*p[1]+m[9]*p[2]+m[13]),
            clean(-(m[2]*p[0]+m[6]*p[1]+m[10]*p[2]+m[14]))};
}
Point cross(const Point& a,const Point& b){
    return {a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],
            a[0]*b[1]-a[1]*b[0]};
}
Point diff(const Point& a,const Point& b){
    return {a[0]-b[0],a[1]-b[1],a[2]-b[2]};
}
float dot(const Point& a,const Point& b){
    return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}
float materialColor(const float* values,std::size_t channel) {
    if(!values)return 1.0f;
    return std::clamp(values[channel],0.0f,1.0f);
}
void addTriangle(StaticScene& out,Point a,Point b,Point c,
                 const std::array<float,4>& material,
                 const std::array<float,4>& ca,
                 const std::array<float,4>& cb,
                 const std::array<float,4>& cc){
    Point n=cross(diff(b,a),diff(c,a));
    const float length=std::sqrt(dot(n,n));
    if(!std::isfinite(length)||length<1e-8f)return;
    const Point sunlight={0.35f,0.85f,0.39f};
    const float irradiance=std::clamp(
       0.26f+0.74f*std::abs(dot(n,sunlight)/(length*0.99f)),0.26f,1.f);
    for(const auto& record : {std::pair{a,ca},std::pair{c,cc},std::pair{b,cb}}){
        const auto& pos=record.first;
        const auto& col=record.second;
        out.triangles.push_back({
            pos[0],pos[1],pos[2],
            materialColor(material.data(),0)*materialColor(col.data(),0)*irradiance,
            materialColor(material.data(),1)*materialColor(col.data(),1)*irradiance,
            materialColor(material.data(),2)*materialColor(col.data(),2)*irradiance,
            materialColor(material.data(),3)*materialColor(col.data(),3)
        });
    }
    if(out.triangles.size()>maxHostVertices)
        throw std::runtime_error("GLB scene exceeds native vertex resource limit");
}
Point positionOf(const cgltf_accessor* attr,cgltf_size index,const cgltf_float* matrix){
    float p[3]={};
    if(!cgltf_accessor_read_float(attr,index,p,3))
        throw std::runtime_error("invalid GLB POSITION accessor");
    return worldPosition(matrix,{p[0],p[1],p[2]});
}
std::array<float,4> colorOf(const cgltf_accessor* accessor,cgltf_size index){
    std::array<float,4> out{1,1,1,1};
    if(accessor){
        float v[4]={1,1,1,1};
        if(!cgltf_accessor_read_float(accessor,index,v,4))
            throw std::runtime_error("invalid GLB COLOR_0 accessor");
        out={v[0],v[1],v[2],accessor->type==cgltf_type_vec4?v[3]:1.f};
    }
    return out;
}
void appendMesh(const cgltf_node* node,StaticScene& out){
    if(!node->mesh)return;
    ++out.sourceNodes;
    cgltf_float matrix[16]{};
    cgltf_node_transform_world(node,matrix);
    for(cgltf_size p=0;p<node->mesh->primitives_count;++p){
        const auto& primitive=node->mesh->primitives[p];
        if(primitive.type!=cgltf_primitive_type_triangles){
            ++out.unsupportedPrimitives;
            continue;
        }
        const cgltf_accessor* pos=nullptr;
        const cgltf_accessor* col=nullptr;
        for(cgltf_size a=0;a<primitive.attributes_count;++a){
            const auto& attribute=primitive.attributes[a];
            if(attribute.type==cgltf_attribute_type_position)pos=attribute.data;
            if(attribute.type==cgltf_attribute_type_color &&
               attribute.index==0)col=attribute.data;
        }
        if(!pos || pos->type!=cgltf_type_vec3){
            ++out.unsupportedPrimitives;
            continue;
        }
        const cgltf_size count=primitive.indices?primitive.indices->count:pos->count;
        if(count%3!=0 || count>maxHostVertices ||
            out.triangles.size()+count>maxHostVertices)
            throw std::runtime_error("GLB primitive too large or invalid TRIANGLES index count");
        const float* rgba=nullptr;
        if(primitive.material && primitive.material->has_pbr_metallic_roughness)
            rgba=primitive.material->pbr_metallic_roughness.base_color_factor;
        std::array<float,4> color{};
        for(int i=0;i<4;++i)color[std::size_t(i)]=materialColor(rgba,std::size_t(i));
        for(cgltf_size i=0;i<count;i+=3){
            std::array<cgltf_size,3> idx{};
            for(cgltf_size k=0;k<3;++k){
                idx[k]=primitive.indices?
                    cgltf_accessor_read_index(primitive.indices,i+k):i+k;
                if(idx[k]>=pos->count)
                    throw std::runtime_error("GLB triangle index outside POSITION bounds");
                if(col && idx[k]>=col->count)
                    throw std::runtime_error("GLB COLOR_0 count does not cover vertex");
            }
            addTriangle(out,
              positionOf(pos,idx[0],matrix),
              positionOf(pos,idx[1],matrix),
              positionOf(pos,idx[2],matrix),
              color,colorOf(col,idx[0]),colorOf(col,idx[1]),colorOf(col,idx[2]));
        }
        if(node->skin)++out.skinnedPrimitives;
        ++out.sourcePrimitives;
    }
}
void visit(const cgltf_node* node,StaticScene& out,unsigned depth,
           std::vector<const cgltf_node*>& ancestry){
    if(depth>128||std::find(ancestry.begin(),ancestry.end(),node)!=ancestry.end())
        throw std::runtime_error("GLB scene graph has a cycle or excessive depth");
    ancestry.push_back(node);
    appendMesh(node,out);
    for(cgltf_size i=0;i<node->children_count;++i)
        visit(node->children[i],out,depth+1,ancestry);
    ancestry.pop_back();
}
void bounds(StaticScene& out){
    if(out.triangles.empty())
        throw std::runtime_error("GLB contains no supported triangles");
    const auto& first=out.triangles.front();
    out.bounds.min={first.x,first.y,first.z};
    out.bounds.max=out.bounds.min;
    for(const auto& v:out.triangles){
        for(int k=0;k<3;++k){
            const float value=k==0?v.x:k==1?v.y:v.z;
            out.bounds.min[std::size_t(k)]=std::min(out.bounds.min[std::size_t(k)],value);
            out.bounds.max[std::size_t(k)]=std::max(out.bounds.max[std::size_t(k)],value);
        }
    }
    double sq=0;
    for(int k=0;k<3;++k){
        out.bounds.center[std::size_t(k)]=
            (out.bounds.min[std::size_t(k)]/2.f+out.bounds.max[std::size_t(k)]/2.f);
        const double r=double(out.bounds.max[std::size_t(k)])-
                       double(out.bounds.min[std::size_t(k)]);
        sq+=r*r;
    }
    out.bounds.radius=clean(float(std::sqrt(sq)/2.));
}
} // namespace

StaticScene loadStaticGlbBytes(const std::vector<std::uint8_t>& bytes){
    if(bytes.size()<20||bytes.size()>maxGlbBytes)
        throw std::runtime_error("invalid/oversized native GLB input");
    cgltf_options options{};
    cgltf_data* raw=nullptr;
    auto result=cgltf_parse(&options,bytes.data(),bytes.size(),&raw);
    if(result!=cgltf_result_success)
        throw std::runtime_error("GLB parsing failed; file is not valid glTF 2.0");
    const std::unique_ptr<cgltf_data,decltype(&cgltf_free)> data(raw,cgltf_free);
    if(data->file_type!=cgltf_file_type_glb)
        throw std::runtime_error("only standalone embedded .glb is supported");
    if(cgltf_validate(data.get())!=cgltf_result_success)
        throw std::runtime_error("GLB structural validation failed");
    for(cgltf_size i=0;i<data->buffers_count;++i)
        if(data->buffers[i].uri && data->buffers[i].uri[0])
            throw std::runtime_error("external GLB buffer URI rejected");
    if(cgltf_load_buffers(&options,data.get(),nullptr)!=cgltf_result_success)
        throw std::runtime_error("GLB native binary buffer unavailable");
    const cgltf_scene* scene=data->scene;
    if(!scene && data->scenes_count)scene=&data->scenes[0];
    if(!scene)throw std::runtime_error("GLB has no scene hierarchy");
    StaticScene out;
    std::vector<const cgltf_node*> ancestry;
    for(cgltf_size i=0;i<scene->nodes_count;++i)
        visit(scene->nodes[i],out,0,ancestry);
    bounds(out);
    return out;
}
StaticScene loadStaticGlb(const std::filesystem::path& path){
    std::error_code error;
    const auto fileLength=std::filesystem::file_size(path,error);
    if(error||fileLength<20||fileLength>maxGlbBytes)
        throw std::runtime_error("native GLB file missing, malformed, or too large");
    std::ifstream stream(path,std::ios::binary);
    if(!stream)throw std::runtime_error("cannot open native GLB");
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(fileLength));
    if(!stream.read(reinterpret_cast<char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size())))
        throw std::runtime_error("short GLB filesystem read");
    return loadStaticGlbBytes(bytes);
}
} // namespace ae3
