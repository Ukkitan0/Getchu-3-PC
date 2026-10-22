#include "ae3/original_resource_stage_catalog.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
const char* signatureName(ae3::OriginalPckPayloadSignature signature) {
    switch(signature) {
        case ae3::OriginalPckPayloadSignature::i3d_mesh:return "I3D_BIN";
        case ae3::OriginalPckPayloadSignature::i3d_animation:return "I3D_I3M";
        case ae3::OriginalPckPayloadSignature::i3d_collision:return "I3D_I3C";
        case ae3::OriginalPckPayloadSignature::tim2:return "TIM2";
        case ae3::OriginalPckPayloadSignature::exdb:return "EDB";
        case ae3::OriginalPckPayloadSignature::unknown:return "unclassified";
    }
    return "unclassified";
}
}
int main(int argc,char** argv) {
    try {
        if(argc<2||argc>4){
            std::cerr<<"Usage: Getchu3OriginalStageResourceProbe <owned-USA.iso|DATA.BIN> [stage-name] [--inspect-pck]\n";
            return 2;
        }
        const std::string stage=argc>=3 ? argv[2]:"";
        const bool inspect=argc==4 && std::string_view(argv[3])=="--inspect-pck";
        if(argc==4&&!inspect)throw std::invalid_argument("Unknown option: expected --inspect-pck");
        if(!stage.empty()&&!ae3::originalResourceValidStageName(stage))
            throw std::invalid_argument("Invalid original stage name");
        ae3::OriginalResourceVfiMount mount{std::filesystem::path(argv[1])};
        const ae3::OriginalStageResourceCatalog catalog(mount.vfiEntries());
        std::cout<<"Original USA ISO/VFI stage folders: "<<catalog.stageNames().size()
                 <<"; stage file entries: "<<catalog.entries().size()<<"\n";
        if(stage.empty()) {
            for(const auto& name:catalog.stageNames())
                std::cout<<name<<" vfi_members="<<catalog.entriesForStage(name).size()<<'\n';
            return 0;
        }
        const auto paths=catalog.entriesForStage(stage);
        if(paths.empty())throw std::out_of_range("No original stage directory in owned USA VFI index");
        std::size_t compressedCount=0,packageCount=0,packageMembers=0;
        for(const auto index:paths) {
            const auto& member=catalog.entries()[index];
            compressedCount+=member.originalSzCompressed;
            std::cout<<member.sourceEntry.path
                     <<" VFI_DATA_BIN_offset="<<member.sourceEntry.offset
                     <<" original_stored_bytes="<<member.sourceEntry.size
                     <<" original_sz="<<member.originalSzCompressed<<'\n';
            if(!inspect)continue;
            if(!std::string_view(ae3::originalResourceAsciiFold(member.uncompressedName)).ends_with(".pck"))continue;
            const auto package=catalog.openExactPck(mount,stage,member.stageRelativePath);
            ++packageCount;
            for(const auto& resource:package->entries()) {
                ++packageMembers;
                std::cout<<"  source_item="<<resource.sourceIndex
                         <<" name="<<resource.name<<" type="<<resource.type
                         <<" package_offset="<<resource.payloadOffset
                         <<" bytes="<<resource.payload.size()
                         <<" header="<<signatureName(ae3::originalPckPayloadSignature(resource.payload))
                         <<'\n';
            }
        }
        std::cout<<"Original stage inventory: stage="<<stage<<" vfi_files="<<paths.size()
                 <<" SZ_files="<<compressedCount<<" PCK_inspected="<<packageCount
                 <<" PCK_members="<<packageMembers<<"\n"
                 <<"File inventory is not a reconstructed retail load order or playable game.\n";
        return 0;
    }catch(const std::exception& e){
        std::cerr<<"ORIGINAL_STAGE_RESOURCE_PROBE_FAILED: "<<e.what()<<'\n';
        return 1;
    }
}
