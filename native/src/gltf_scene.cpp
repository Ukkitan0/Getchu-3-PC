#include "ae3/gltf_scene.hpp"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include <algorithm>
#include <cctype>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <map>
#include <utility>
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
                 const std::array<float,4>& cc,
                 const std::array<float,2>& ua,
                 const std::array<float,2>& ub,
                 const std::array<float,2>& uc){
    Point n=cross(diff(b,a),diff(c,a));
    const float length=std::sqrt(dot(n,n));
    if(!std::isfinite(length)||length<1e-8f)return;
    // Preserve exported original asset base-color and vertex-color factors.
    // The previous hard-coded host sunlight was fabricated, not recovered
    // from the original ELF; genuine stage lighting belongs to the future
    // retail-derived material/light runtime and D3D shader pipeline.
    // Flip triangle orientation when changing from glTF RH to D3D LH.
    const std::array<Point,3> coords{a,c,b};
    const std::array<std::array<float,4>,3> colors{ca,cc,cb};
    const std::array<std::array<float,2>,3> uvs{ua,uc,ub};
    for(std::size_t i=0;i<3;++i){
        const auto& pos=coords[i];
        const auto& col=colors[i];
        out.triangles.push_back({
            pos[0],pos[1],pos[2],
            materialColor(material.data(),0)*materialColor(col.data(),0),
            materialColor(material.data(),1)*materialColor(col.data(),1),
            materialColor(material.data(),2)*materialColor(col.data(),2),
            materialColor(material.data(),3)*materialColor(col.data(),3),
            uvs[i][0],uvs[i][1]
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
std::array<float,2> texcoordOf(const cgltf_accessor* accessor,cgltf_size at){
    if(!accessor)return {0,0};
    float uv[2]={0,0};
    if(!cgltf_accessor_read_float(accessor,at,uv,2))
        throw std::runtime_error("GLB TEXCOORD_0 accessor cannot be read");
    return {clean(uv[0]),clean(uv[1])};
}
bool withinRoot(const std::filesystem::path& file,const std::filesystem::path& root){
    auto a=root.begin(),b=file.begin();
    for(;a!=root.end();++a,++b){
        if(b==file.end()||*a!=*b)return false;
    }
    return true;
}
std::filesystem::path allowedTextureRoot(const std::filesystem::path& glb){
    namespace fs=std::filesystem;
    fs::path directory=glb.parent_path();
    for(fs::path cursor=directory;!cursor.empty();cursor=cursor.parent_path()){
        std::string component=cursor.filename().string();
        std::transform(component.begin(),component.end(),component.begin(),
                       [](unsigned char c){return char(std::tolower(c));});
        if(component=="glb")return cursor.parent_path();
        if(cursor==cursor.root_path()||cursor.parent_path()==cursor)break;
    }
    return directory;
}
std::vector<std::uint8_t> originalTextureBytes(const cgltf_image* image,
                                                const std::filesystem::path* source){
    namespace fs=std::filesystem;
    constexpr std::size_t maxTexBytes=32*1024*1024;
    if(!image)return {};
    if(image->buffer_view){
        const auto* view=image->buffer_view;
        const auto* buffer=view->buffer;
        if(!buffer||!buffer->data||view->offset>buffer->size ||
            view->size>buffer->size-view->offset || view->size>maxTexBytes)
            return {};
        const auto* bytes=static_cast<const std::uint8_t*>(buffer->data);
        return std::vector<std::uint8_t>(bytes+view->offset,
                                         bytes+view->offset+view->size);
    }
    if(!source||!image->uri)return {};
    const std::string uri=image->uri;
    if(uri.empty()||uri.size()>1024||uri.find(':')!=std::string::npos||
       uri.find('%')!=std::string::npos||uri.find('?')!=std::string::npos||
       uri.find('#')!=std::string::npos)return {};
    const fs::path relative=fs::u8path(uri);
    if(relative.is_absolute()||relative.has_root_path())return {};
    std::error_code error;
    const auto root=fs::weakly_canonical(allowedTextureRoot(*source),error);
    if(error)return {};
    const auto resolved=fs::weakly_canonical(source->parent_path()/relative,error);
    if(error||!withinRoot(resolved,root))return {};
    if(!fs::is_regular_file(resolved,error)||error)return {};
    const auto length=fs::file_size(resolved,error);
    if(error||length<16||length>maxTexBytes)return {};
    std::ifstream stream(resolved,std::ios::binary);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length));
    if(!stream.read(reinterpret_cast<char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size())))return {};
    return bytes;
}
struct TextureCache {
    StaticScene& out;
    const std::filesystem::path* file=nullptr;
    std::map<const cgltf_image*,int> entries;
    int index(const cgltf_image* image){
        if(!image)return -1;
        auto existing=entries.find(image);
        if(existing!=entries.end())return existing->second;
        auto bytes=originalTextureBytes(image,file);
        if(bytes.empty()){
            ++out.missingTextures;
            entries[image]=-1;
            return -1;
        }
        if(out.textures.size()>=512)
            throw std::runtime_error("GLB too many native image resources");
        const int index=static_cast<int>(out.textures.size());
        out.textures.push_back({std::move(bytes)});
        entries[image]=index;
        return index;
    }
};
void appendMesh(const cgltf_node* node,StaticScene& out,TextureCache& texCache){
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
        const cgltf_accessor* uv=nullptr;
        for(cgltf_size a=0;a<primitive.attributes_count;++a){
            const auto& attribute=primitive.attributes[a];
            if(attribute.type==cgltf_attribute_type_position)pos=attribute.data;
            if(attribute.type==cgltf_attribute_type_color &&
               attribute.index==0)col=attribute.data;
            if(attribute.type==cgltf_attribute_type_texcoord &&
               attribute.index==0)uv=attribute.data;
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
        int textureIndex=-1;
        if(primitive.material && primitive.material->has_pbr_metallic_roughness){
            const auto* tex=primitive.material->pbr_metallic_roughness.base_color_texture.texture;
            if(tex)textureIndex=texCache.index(tex->image);
        }
        const auto firstVertex=static_cast<std::uint32_t>(out.triangles.size());
        for(cgltf_size i=0;i<count;i+=3){
            std::array<cgltf_size,3> idx{};
            for(cgltf_size k=0;k<3;++k){
                idx[k]=primitive.indices?
                    cgltf_accessor_read_index(primitive.indices,i+k):i+k;
                if(idx[k]>=pos->count)
                    throw std::runtime_error("GLB triangle index outside POSITION bounds");
                if(col && idx[k]>=col->count)
                    throw std::runtime_error("GLB COLOR_0 count does not cover vertex");
                if(uv && idx[k]>=uv->count)
                    throw std::runtime_error("GLB TEXCOORD_0 does not cover vertex");
            }
            addTriangle(out,
              positionOf(pos,idx[0],matrix),
              positionOf(pos,idx[1],matrix),
              positionOf(pos,idx[2],matrix),
              color,colorOf(col,idx[0]),colorOf(col,idx[1]),colorOf(col,idx[2]),
              texcoordOf(uv,idx[0]),texcoordOf(uv,idx[1]),texcoordOf(uv,idx[2]));
        }
        const auto emitted=static_cast<std::uint32_t>(out.triangles.size())-firstVertex;
        if(emitted)out.batches.push_back({firstVertex,emitted,textureIndex});
        if(node->skin)++out.skinnedPrimitives;
        ++out.sourcePrimitives;
    }
}
void visit(const cgltf_node* node,StaticScene& out,TextureCache& textures,
           unsigned depth,std::vector<const cgltf_node*>& ancestry){
    if(depth>128||std::find(ancestry.begin(),ancestry.end(),node)!=ancestry.end())
        throw std::runtime_error("GLB scene graph has a cycle or excessive depth");
    ancestry.push_back(node);
    appendMesh(node,out,textures);
    for(cgltf_size i=0;i<node->children_count;++i)
        visit(node->children[i],out,textures,depth+1,ancestry);
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

static StaticScene loadImpl(const std::vector<std::uint8_t>& bytes,
                            const std::filesystem::path* source){
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
    TextureCache textureCache{out,source,{}};
    std::vector<const cgltf_node*> ancestry;
    for(cgltf_size i=0;i<scene->nodes_count;++i)
        visit(scene->nodes[i],out,textureCache,0,ancestry);
    bounds(out);
    return out;
}
StaticScene loadStaticGlbBytes(const std::vector<std::uint8_t>& bytes){
    return loadImpl(bytes,nullptr);
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
    return loadImpl(bytes,&path);
}
} // namespace ae3
