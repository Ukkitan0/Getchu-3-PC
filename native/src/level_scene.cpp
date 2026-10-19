#include "ae3/level_scene.hpp"
#include <algorithm>
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
std::uint32_t u32(const std::vector<std::uint8_t>& b,std::size_t o){
    if(o>b.size()||b.size()-o<4)throw std::runtime_error("truncated AE3 PLC header");
    return std::uint32_t(b[o]) | (std::uint32_t(b[o+1])<<8) |
           (std::uint32_t(b[o+2])<<16) | (std::uint32_t(b[o+3])<<24);
}
float f32(const std::vector<std::uint8_t>& b,std::size_t o){
    const std::uint32_t bits=u32(b,o);
    float value=0;
    std::memcpy(&value,&bits,sizeof(value));
    return value;
}
std::string asciiName(const std::vector<std::uint8_t>& b,std::size_t o){
    if(o>b.size()||b.size()-o<32)
        throw std::runtime_error("truncated PLC name field");
    std::size_t n=0;
    while(n<32&&b[o+n]!=0){
        if(b[o+n]<32||b[o+n]>126)return {};
        ++n;
    }
    return std::string(reinterpret_cast<const char*>(b.data()+o),n);
}
std::string lower(std::string s){
    std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){
        return char(std::tolower(c));
    });
    return s;
}
bool backgroundAsset(const std::filesystem::path& p){
    for(const auto& component:p){
        auto name=lower(component.string());
        if(name.find("_bg")!=std::string::npos||name.find("background")!=std::string::npos)
            return true;
    }
    return false;
}
void merge(StaticScene& dst,const StaticScene& src,
           const std::array<float,3>& delta){
    constexpr std::size_t limit=3'000'000;
    if(dst.triangles.size()>limit||src.triangles.size()>limit-dst.triangles.size())
        throw std::runtime_error("native AE3 level exceeds safe D3D11 vertex budget");
    if(dst.textures.size()+src.textures.size()>512)
        throw std::runtime_error("native AE3 level has too many texture resources");
    const auto vertexBase=static_cast<std::uint32_t>(dst.triangles.size());
    const auto texBase=static_cast<int>(dst.textures.size());
    dst.triangles.reserve(dst.triangles.size()+src.triangles.size());
    for(auto v:src.triangles){
        v.x+=delta[0];v.y+=delta[1];
        // Original PLC uses the original game coordinate convention.
        // Our GLB geometry is already converted to D3D left-handed space.
        v.z-=delta[2];
        dst.triangles.push_back(v);
    }
    for(const auto& batch:src.batches){
        const int index=batch.textureIndex<0?-1:texBase+batch.textureIndex;
        dst.batches.push_back({
            static_cast<std::uint32_t>(vertexBase+batch.firstVertex),
            batch.vertexCount,index
        });
    }
    dst.textures.insert(dst.textures.end(),src.textures.begin(),src.textures.end());
    dst.sourceNodes+=src.sourceNodes;
    dst.sourcePrimitives+=src.sourcePrimitives;
    dst.skinnedPrimitives+=src.skinnedPrimitives;
    dst.unsupportedPrimitives+=src.unsupportedPrimitives;
    dst.missingTextures+=src.missingTextures;
}
void computeBounds(StaticScene& dst){
    if(dst.triangles.empty())throw std::runtime_error("level produced no GLB triangles");
    auto& bounds=dst.bounds;
    bounds.min={dst.triangles.front().x,dst.triangles.front().y,dst.triangles.front().z};
    bounds.max=bounds.min;
    for(const auto& v:dst.triangles){
        const float p[3]={v.x,v.y,v.z};
        for(int k=0;k<3;++k){
            bounds.min[std::size_t(k)]=std::min(bounds.min[std::size_t(k)],p[k]);
            bounds.max[std::size_t(k)]=std::max(bounds.max[std::size_t(k)],p[k]);
        }
    }
    double d=0;
    for(int k=0;k<3;++k){
        bounds.center[std::size_t(k)]=
            bounds.min[std::size_t(k)]/2.f+bounds.max[std::size_t(k)]/2.f;
        const double side=double(bounds.max[std::size_t(k)])-bounds.min[std::size_t(k)];
        d+=side*side;
    }
    bounds.radius=static_cast<float>(std::sqrt(d)/2);
}
} // namespace

PlacementFile parsePlacements(const std::vector<std::uint8_t>& b){
    if(b.size()<192||b.size()>64*1024*1024)
        throw std::runtime_error("PLC source too small or oversized");
    const auto n=u32(b,0);
    if(n>100000)throw std::runtime_error("PLC record count unreasonable");
    PlacementFile output;
    output.entries.reserve(std::min(n,100000u));
    constexpr std::uint64_t start=0xc0,stride=0x80;
    if(std::uint64_t(n)>((std::numeric_limits<std::uint64_t>::max()-start)/stride))
        throw std::runtime_error("PLC count overflow");
    for(std::uint64_t i=0;i<n;++i){
        const auto o=start+i*stride;
        if(o>b.size()||stride>b.size()-o){
            output.skipped+=n-static_cast<std::uint32_t>(i);
            break;
        }
        const std::string name=asciiName(b,static_cast<std::size_t>(o));
        const float x=f32(b,static_cast<std::size_t>(o)+0x40);
        const float y=f32(b,static_cast<std::size_t>(o)+0x44);
        const float z=f32(b,static_cast<std::size_t>(o)+0x48);
        if(name.empty()||!std::isfinite(x)||!std::isfinite(y)||!std::isfinite(z)||
           std::abs(x)>=1e5f||std::abs(y)>=1e5f||std::abs(z)>=1e5f) {
            ++output.skipped;
            continue;
        }
        output.entries.push_back({name,{x,y,z}});
    }
    return output;
}
PlacementFile parsePlacements(const std::filesystem::path& path){
    std::error_code ec;
    const auto n=std::filesystem::file_size(path,ec);
    if(ec||n<192||n>64*1024*1024)
        throw std::runtime_error("PLC file missing, corrupt or oversized");
    std::ifstream stream(path,std::ios::binary);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(n));
    if(!stream.read(reinterpret_cast<char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size())))
        throw std::runtime_error("short original PLC file read");
    return parsePlacements(bytes);
}
bool matchesPlacement(const std::string& base,const std::string& name){
    const auto a=lower(base),b=lower(name);
    return a==b||(a.size()>b.size()+1 &&
                   a.compare(a.size()-b.size()-1,b.size()+1,"_"+b)==0);
}
LevelLoadResult loadNativeLevelFolder(const std::filesystem::path& directory,
                                     const std::filesystem::path& plc,
                                     unsigned maxFiles){
    namespace fs=std::filesystem;
    if(!fs::is_directory(directory))
        throw std::runtime_error("native level GLB folder not found");
    if(maxFiles==0||maxFiles>1000)
        throw std::runtime_error("unsafe level model file budget");
    PlacementFile placement;
    const bool positioned=!plc.empty();
    if(positioned)placement=parsePlacements(plc);
    LevelLoadResult result;
    std::vector<fs::path> files;
    for(auto it=fs::recursive_directory_iterator(
              directory,fs::directory_options::skip_permission_denied);
        it!=fs::recursive_directory_iterator();++it){
        if(it->is_symlink() || !it->is_regular_file())continue;
        if(lower(it->path().extension().string())!=".glb")continue;
        files.push_back(it->path());
        if(files.size()>maxFiles)throw std::runtime_error("native level contains too many GLBs");
    }
    std::sort(files.begin(),files.end());
    for(const auto& file:files){
        const auto base=file.stem().string();
        std::vector<std::array<float,3>> instances;
        if(positioned){
            for(const auto& item:placement.entries)
                if(matchesPlacement(base,item.name))instances.push_back(item.xyz);
            if(instances.empty()&&backgroundAsset(file))
                instances.push_back({0,0,0});
        }else instances.push_back({0,0,0});
        if(instances.empty()){
            ++result.unplaced;
            continue;
        }
        try{
            auto source=loadStaticGlb(file);
            ++result.glbSourceFiles;
            for(const auto& p:instances){
                merge(result.scene,source,p);
                ++result.placedInstances;
            }
        }catch(const std::exception& e){
            ++result.modelFailures;
            if(result.warnings.size()<32)
                result.warnings.push_back(file.filename().string()+": "+e.what());
        }
    }
    computeBounds(result.scene);
    return result;
}
} // namespace ae3
