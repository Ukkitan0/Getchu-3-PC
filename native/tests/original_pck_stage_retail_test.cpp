#include "ae3/pck_archive.hpp"
#include "ae3/original_resource_package.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace fs=std::filesystem;
using namespace ae3;
static void ck(bool condition,const char* message){
    if(!condition)throw std::runtime_error(message);
}
struct RetailCase {
    const char* originalFilename;
    std::size_t members,meshes,textures,animations,collisions;
};
// Counts observed directly from the owner's SHA-pinned original USA
// vfi/debug/us/stage/woods_c PCK resources. Commercial bytes NOT embedded.
static constexpr std::array<RetailCase,5> cases{{
    {"apelodtex.pck",4,0,4,0,0},
    {"bg.pck",73,25,27,5,9},
    {"character.pck",79,11,24,32,0},
    {"exdb.pck",2,0,0,0,0},
    {"navigation.pck",2,0,0,0,0}
}};
int main(int argc,char**argv){
    try{
        if(argc!=2)
            throw std::invalid_argument(
                "Pass owner-private vfi/debug/us/stage/woods_c folder; validate pinned file SHA-256 externally");
        const fs::path originalStage(argv[1]);
        std::size_t grandTotal=0;
        for(const auto& sample:cases){
            const fs::path path=originalStage/sample.originalFilename;
            std::ifstream stream(path,std::ios::binary);
            ck(bool(stream),"Original owner-private retail stage package missing");
            const std::vector<char> raw((std::istreambuf_iterator<char>(stream)),{});
            ck(!raw.empty(),"Owner-private original PCK is empty");
            std::vector<std::byte> data;
            data.reserve(raw.size());
            for(const char value:raw)data.push_back(std::byte(static_cast<unsigned char>(value)));
            OriginalPckResourcePackage knownRetailTable(std::move(data));
            const PckArchive hostUtility(path);
            const auto& parsed=hostUtility.entries();
            const auto original=knownRetailTable.entries();
            ck(parsed.size()==sample.members && original.size()==sample.members,
               "Retail PCK original member count mismatch");
            std::size_t mesh=0,tex=0,animation=0,collision=0;
            for(std::size_t i=0;i<parsed.size();++i){
                const auto& row=parsed[i];
                const auto& expected=original[i];
                ck(row.index==expected.sourceIndex &&
                   row.name==expected.name && row.originalType==expected.type &&
                   row.type<raw.size() && row.offset==expected.payloadOffset &&
                   row.size==expected.payload.size(),
                   "Original PCK utility table differs from independent retail parser");
                const auto payload=hostUtility.read(row);
                ck(payload.size()==expected.payload.size(),"Original resource payload size changed");
                for(std::size_t j=0;j<payload.size();++j)
                    ck(payload[j]==std::to_integer<std::uint8_t>(expected.payload[j]),
                       "Original retail resource payload byte changed");
                switch(row.kind){
                    case PckAssetKind::mesh:++mesh;break;
                    case PckAssetKind::texture:++tex;break;
                    case PckAssetKind::animation:++animation;break;
                    case PckAssetKind::collision:++collision;break;
                    case PckAssetKind::other:break;
                }
            }
            ck(mesh==sample.meshes && tex==sample.textures &&
               animation==sample.animations && collision==sample.collisions,
               "Original retail resource signature classification changed");
            grandTotal+=parsed.size();
        }
        ck(grandTotal==160,"The five verified original PCKs contain 160 entries");
        std::cout<<"ORIGINAL_RETAIL_STAGE_PCK_DIFFERENTIAL_PASSED "
                 <<"woods_c_packages=5 source_members="<<grandTotal
                 <<" meshes=36 textures=55 animations=37 collisions=9\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"ORIGINAL_RETAIL_STAGE_PCK_DIFFERENTIAL_FAILED: "<<e.what()<<'\n';
        return 1;
    }
}
