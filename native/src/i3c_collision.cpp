#include "ae3/i3c_collision.hpp"
#include "ae3/pck_archive.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ae3 {
namespace {
constexpr std::size_t kMaxFile=128ull*1024*1024;
constexpr std::size_t kMaxTriangles=2'000'000;
constexpr std::size_t kMaxVertices=2'000'000;
constexpr std::size_t kMaxNodes=4'000'000;
struct Reader {
    const std::vector<std::uint8_t>& b;
    bool within(std::uint64_t at,std::uint64_t length) const {
        return at<=b.size() && length<=b.size()-at;
    }
    void need(std::uint64_t at,std::uint64_t length) const {
        if(!within(at,length))throw std::runtime_error("I3C field exceeds original collision file");
    }
    std::uint16_t u16(std::uint64_t at) const {
        need(at,2);
        return std::uint16_t(b[std::size_t(at)]) |
               (std::uint16_t(b[std::size_t(at+1)])<<8);
    }
    std::uint32_t u32(std::uint64_t at) const {
        return std::uint32_t(u16(at)) | (std::uint32_t(u16(at+2))<<16);
    }
    float f32(std::uint64_t at) const {
        const auto bits=u32(at);
        float v=0;std::memcpy(&v,&bits,sizeof(v));
        if(!std::isfinite(v)||std::abs(v)>1e8f)
            throw std::runtime_error("I3C coordinate or bound is nonfinite/out-of-range");
        return v;
    }
    std::string name(std::uint64_t offset) const {
        if(!offset)return {};
        need(offset,1);
        std::string result;
        for(std::uint64_t at=offset;at<b.size()&&result.size()<256;++at){
            const auto value=b[std::size_t(at)];
            if(value==0)return result;
            if(value<32||value>126)throw std::runtime_error("I3C non-ASCII metadata");
            result.push_back(static_cast<char>(value));
        }
        throw std::runtime_error("I3C missing collision name terminator");
    }
};
struct Vec {float x,y,z;};
Vec at(const SceneVertex& v){return {v.x,v.y,v.z};}
Vec sub(Vec a,Vec b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
Vec cross(Vec a,Vec b){
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}
bool validTriangle(Vec a,Vec b,Vec c){
    const auto n=cross(sub(b,a),sub(c,a));
    return std::isfinite(n.x+n.y+n.z) &&
           double(n.x)*n.x+double(n.y)*n.y+double(n.z)*n.z>1e-16;
}
void emit(OriginalCollision& output,const std::vector<std::array<float,3>>& verts,
          unsigned x,unsigned y,unsigned z){
    if(x>=verts.size()||y>=verts.size()||z>=verts.size())
        throw std::runtime_error("I3C triangle index outside decoded vertex array");
    // Original I3C winding is inward in the original RH space. The
    // native D3D LH transform negates Z; keep the original triangle
    // index order to preserve its geometric two-sided walk tests.
    const auto& a=verts[x];const auto& b=verts[y];const auto& c=verts[z];
    const Vec v0{a[0],a[1],-a[2]},v1{b[0],b[1],-b[2]},v2{c[0],c[1],-c[2]};
    if(!validTriangle(v0,v1,v2))return;
    for(const auto& v:{v0,v1,v2})
        output.geometry.triangles.push_back({v.x,v.y,v.z,0.75f,0.85f,0.9f,1,0,0});
    if(output.geometry.triangles.size()/3>kMaxTriangles)
        throw std::runtime_error("I3C collision triangle budget exceeded");
}
void bounds(StaticScene& scene){
    if(scene.triangles.empty())
        throw std::runtime_error("I3C collision contains no valid triangle surfaces");
    auto& box=scene.bounds;
    box.min={scene.triangles[0].x,scene.triangles[0].y,scene.triangles[0].z};
    box.max=box.min;
    for(const auto& v:scene.triangles){
        const float p[3]={v.x,v.y,v.z};
        for(int k=0;k<3;++k){
            box.min[std::size_t(k)]=std::min(box.min[std::size_t(k)],p[k]);
            box.max[std::size_t(k)]=std::max(box.max[std::size_t(k)],p[k]);
        }
    }
    double r=0;
    for(int k=0;k<3;++k){
        box.center[std::size_t(k)]=(box.min[std::size_t(k)]+box.max[std::size_t(k)])/2;
        const double span=double(box.max[std::size_t(k)])-box.min[std::size_t(k)];
        r+=span*span;
    }
    box.radius=static_cast<float>(std::sqrt(r)/2);
}
// Traverse original I3C BVH only as an OFFLINE DATA decoder. The native
// gameplay collision walker later operates on the resulting triangles;
// no Sony BVH traversal code or EE machine instructions run at gameplay time.
void walk(const Reader& r,OriginalCollision& out,
          const std::vector<std::array<float,3>>& verts,
          std::uint32_t offset,unsigned depth){
    if(depth>64 || out.bvhNodes>=kMaxNodes)
        throw std::runtime_error("I3C collision BVH exceeds safe recursion/node limit");
    r.need(offset,12);
    ++out.bvhNodes;
    const auto count=r.u16(offset);
    const auto pointer=r.u32(std::uint64_t(offset)+8);
    if(count&0x8000){
        r.need(pointer,8);
        emit(out,verts,r.u16(pointer),r.u16(std::uint64_t(pointer)+2),
             r.u16(std::uint64_t(pointer)+4));
        return;
    }
    const unsigned n=count&0x7fff;
    if(!n||n>512)
        throw std::runtime_error("I3C BVH internal node has unreasonable branching");
    r.need(pointer,std::uint64_t(n)*12);
    for(unsigned i=0;i<n;++i)
        walk(r,out,verts,static_cast<std::uint32_t>(
            std::uint64_t(pointer)+std::uint64_t(i)*12),depth+1);
}
void appendScene(OriginalCollision& dst,const OriginalCollision& src){
    if(src.geometry.triangles.size()>kMaxTriangles*3-dst.geometry.triangles.size())
        throw std::runtime_error("combined I3C collision exceeds triangle budget");
    dst.geometry.triangles.insert(dst.geometry.triangles.end(),
          src.geometry.triangles.begin(),src.geometry.triangles.end());
    dst.groupNames.insert(dst.groupNames.end(),src.groupNames.begin(),src.groupNames.end());
    dst.materialNames.insert(dst.materialNames.end(),src.materialNames.begin(),src.materialNames.end());
    dst.originalVertices+=src.originalVertices;
    dst.bvhNodes+=src.bvhNodes;
    dst.aabbs+=src.aabbs;
}
} // namespace

OriginalCollision parseOriginalI3c(const std::vector<std::uint8_t>& source){
    const Reader r{source};
    if(source.size()<0x20||source.size()>kMaxFile||
       std::memcmp(source.data(),"I3D_I3C\0",8)!=0)
        throw std::runtime_error("not a complete original Ape Escape 3 I3D_I3C");
    if(r.u32(8)!=0x00030000||r.u32(12)!=source.size()||source[0x10]!=1)
        throw std::runtime_error("unsupported I3C version, size or fixup flag");
    const auto groups=r.u16(0x14);
    const auto materials=r.u16(0x16);
    const auto aOffset=r.u32(0x18);
    const auto namesOffset=r.u32(0x1c);
    if(!groups||groups>256||materials>512)
        throw std::runtime_error("I3C unsupported group or material count");
    r.need(aOffset,std::uint64_t(groups)*12);
    r.need(namesOffset,std::uint64_t(materials)*4);
    OriginalCollision output;
    for(unsigned i=0;i<materials;++i)
        output.materialNames.push_back(
            r.name(r.u32(std::uint64_t(namesOffset)+std::uint64_t(i)*4)));
    for(unsigned i=0;i<groups;++i){
        const std::uint64_t record=std::uint64_t(aOffset)+std::uint64_t(i)*12;
        output.groupNames.push_back(r.name(r.u32(record)));
        const auto parts=r.u16(record+4);
        const auto partOffset=r.u32(record+8);
        if(!parts||parts>512)
            throw std::runtime_error("I3C group has invalid part count");
        r.need(partOffset,std::uint64_t(parts)*0x30);
        for(unsigned j=0;j<parts;++j){
            const std::uint64_t at=std::uint64_t(partOffset)+std::uint64_t(j)*0x30;
            for(int axis=0;axis<3;++axis){
                const auto center=r.f32(at+axis*4);
                const auto half=r.f32(at+0x10+axis*4);
                if(half<0||std::abs(center)>1e7f||half>1e7f)
                    throw std::runtime_error("I3C part AABB is invalid");
            }
            const auto tree=r.u32(at+0x28);
            const auto start=r.u32(at+0x2c);
            if(!tree||!start||tree<=start || ((tree-start)%16)!=0)
                throw std::runtime_error("I3C vertices do not precede BVH root");
            const auto count=(tree-start)/16;
            if(!count || count>kMaxVertices ||
               count>kMaxVertices-output.originalVertices)
                throw std::runtime_error("I3C vertex count violates native resource budget");
            r.need(start,std::uint64_t(count)*16);
            std::vector<std::array<float,3>> verts;
            verts.reserve(count);
            for(unsigned k=0;k<count;++k) {
                const auto p=std::uint64_t(start)+std::uint64_t(k)*16;
                verts.push_back({r.f32(p),r.f32(p+4),r.f32(p+8)});
            }
            output.originalVertices+=count;
            ++output.aabbs;
            walk(r,output,verts,tree,0);
        }
    }
    bounds(output.geometry);
    output.geometry.sourcePrimitives=static_cast<std::uint32_t>(
        output.geometry.triangles.size()/3);
    return output;
}
OriginalCollision parseOriginalI3c(const std::filesystem::path& file){
    std::error_code ec;
    const auto n=std::filesystem::file_size(file,ec);
    if(ec||n<0x20||n>kMaxFile)
        throw std::runtime_error("I3C file not found or exceeds resource limit");
    std::ifstream f(file,std::ios::binary);
    if(!f)throw std::runtime_error("cannot read original I3C data");
    std::vector<std::uint8_t> data(static_cast<std::size_t>(n));
    if(!f.read(reinterpret_cast<char*>(data.data()),static_cast<std::streamsize>(n)))
        throw std::runtime_error("short read of original I3C collision data");
    return parseOriginalI3c(data);
}
OriginalCollision loadOriginalI3cDirectory(const std::filesystem::path& folder,
                                           std::size_t maxFiles){
    namespace fs=std::filesystem;
    if(!fs::is_directory(folder)||!maxFiles||maxFiles>1000)
        throw std::runtime_error("invalid I3C collision directory or scan budget");
    std::vector<fs::path> files;
    std::vector<fs::path> packs;
    for(auto it=fs::recursive_directory_iterator(
         folder,fs::directory_options::skip_permission_denied);
        it!=fs::recursive_directory_iterator();++it){
        if(it->is_symlink()||!it->is_regular_file())continue;
        auto extension=it->path().extension().string();
        std::transform(extension.begin(),extension.end(),extension.begin(),
                       [](unsigned char c){return static_cast<char>(std::tolower(c));});
        if(extension==".pck"){
            packs.push_back(it->path());
            if(packs.size()+files.size()>maxFiles)
                throw std::runtime_error("too many original collision source files");
            continue;
        }
        if(extension!=".i3d"&&extension!=".i3c")continue;
        std::ifstream stream(it->path(),std::ios::binary);
        char magic[8]{};
        if(stream.read(magic,sizeof(magic)) && std::memcmp(magic,"I3D_I3C\0",8)==0){
            files.push_back(it->path());
            if(packs.size()+files.size()>maxFiles)
                throw std::runtime_error("too many original collision source files");
        }
    }
    std::sort(files.begin(),files.end());
    std::sort(packs.begin(),packs.end());
    OriginalCollision output;
    std::size_t decoded=0;
    for(const auto& file:files){
        appendScene(output,parseOriginalI3c(file));
        ++decoded;
    }
    // Authentic game collision may be INSIDE bg.pck rather than alongside
    // the extracted stage's .i3d files. Decode PCK members in memory,
    // preserving user-owned asset bytes on disk and requiring real I3C magic.
    for(const auto& file:packs){
        const PckArchive archive(file);
        for(const auto& item:archive.entries()){
            if(item.kind!=PckAssetKind::collision)continue;
            if(decoded>=maxFiles)
                throw std::runtime_error("too many original I3C collision members");
            appendScene(output,parseOriginalI3c(archive.read(item)));
            ++decoded;
        }
    }
    if(!decoded)
        throw std::runtime_error("no original I3D_I3C collision assets found in stage files or PCK");
    bounds(output.geometry);
    return output;
}
} // namespace ae3
