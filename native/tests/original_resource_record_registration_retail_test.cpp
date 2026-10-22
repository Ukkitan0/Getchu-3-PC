#include "ae3/original_resource_record_registration.hpp"
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
static void ck(bool ok,const char* msg){if(!ok)throw std::runtime_error(msg);}
struct OwnedFile {
    std::string name;
    std::vector<std::byte> original;
};
int main(int argc,char**argv){
 try{
    if(argc!=2)throw std::invalid_argument(
        "Pass private, SHA-checked vfi/debug/us/stage/woods_c; NEVER commit original files");
    const fs::path folder(argv[1]);
    // Actual filenames observed in the owner's USA woods_c original source.
    // This is a private DATA test, not proof of 0x25182C's original directory order.
    constexpr std::array<std::string_view,9> names{{
        "placeholder.plc","navigation.pck","actionseq.asq","monkeydb.xml",
        "area.luc","bg.pck","character.pck","exdb.pck","apelodtex.pck"
    }};
    std::vector<OwnedFile> originalFiles;
    originalFiles.reserve(names.size());
    for(const auto filename:names){
        std::ifstream source(folder/filename,std::ios::binary);
        ck(bool(source),"Genuine extracted original stage file missing");
        const std::vector<char> bytes((std::istreambuf_iterator<char>(source)),{});
        ck(!bytes.empty(),"Unexpected empty original game resource");
        originalFiles.push_back({std::string(filename),{}});
        auto& data=originalFiles.back().original;
        data.reserve(bytes.size());
        for(char value:bytes)data.push_back(std::byte(static_cast<unsigned char>(value)));
    }
    std::vector<OriginalStageSourceRecordInput_25182c> input;
    input.reserve(originalFiles.size());
    for(std::size_t i=0;i<originalFiles.size();i++)
        input.push_back({originalFiles[i].name,originalFiles[i].original,i});
    OriginalStageResourceRegistration_25182c all(input,"*");
    ck(all.records().size()==names.size(),"Original wildcard registration entry count");
    const std::array<std::string_view,4> expected{"placeholder","navigation","actionseq","monkeydb"};
    for(std::size_t i=0;i<expected.size();i++){
        const auto* found=all.originalFind_24d4f0(expected[i]);
        ck(found && found->originalSourceIndex==i,"Original stage initialization key lookup");
        ck(found->borrowedOriginalPayload.data()==input[i].originalBytes.data(),
           "Source 0x24D858 must preserve exact original allocation pointer");
        ck(found->borrowedOriginalPayload.size()==input[i].originalBytes.size(),
           "Source 0x24D858 must preserve exact original allocation length");
    }
    ck(all.originalFind_24d4f0("navigation")->borrowedOriginalPayload[0]==std::byte{'P'} &&
       all.originalFind_24d4f0("navigation")->borrowedOriginalPayload[1]==std::byte{'C'},
       "Original navigation.pck raw source bytes");
    OriginalStageResourceRegistration_25182c onlyPck(input,"pck");
    ck(onlyPck.records().size()==5,"Original five source PCK files selected by type");
    ck(onlyPck.originalFind_24d4f0("navigation")!=nullptr &&
       onlyPck.originalFind_24d4f0("monkeydb")==nullptr,
       "Original 0x2519FC selector filters type not resource name");
    OriginalStageResourceRegistration_25182c onlyXml(input,"xml");
    ck(onlyXml.records().size()==1 &&
       onlyXml.originalFind_24d4f0("monkeydb")!=nullptr,
       "Original XML type dispatch for monkeydb");
    OriginalStageResourceRegistration_25182c onlyAsq(input,"asq");
    ck(onlyAsq.records().size()==1 &&
       onlyAsq.originalFind_24d4f0("actionseq")!=nullptr,
       "Original action sequence type");
    ck(all.originalFind_24d4f0("not_on_disc")==nullptr,
       "Original missing source key must yield NULL");
    std::cout<<"ORIGINAL_RETAIL_STAGE_REGISTRATION_TEST_PASSED source=woods_c "
             <<"original_files=9 source_registered=9 pck=5 init_keys=4\n";
    return 0;
 }catch(const std::exception& e){
    std::cerr<<"ORIGINAL_RETAIL_STAGE_REGISTRATION_TEST_FAILED: "<<e.what()<<'\n';
    return 1;
 }
}
