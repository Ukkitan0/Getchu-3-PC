#include "ae3/original_resource_entry_vector.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
using namespace ae3;
static void ck(bool yes,const char* why){if(!yes)throw std::runtime_error(why);}
static std::vector<std::byte> readFile(const char* filename){
    std::ifstream in(filename,std::ios::binary);
    if(!in)throw std::runtime_error("Private original woods_c/exdb.pck missing");
    std::vector<char> bytes((std::istreambuf_iterator<char>(in)),{});
    std::vector<std::byte> result;result.reserve(bytes.size());
    for(unsigned char c:bytes)result.push_back(std::byte{c});
    return result;
}
int main(int argc,char**argv){
    ck(argc==2,"Supply original private woods_c/exdb.pck path");
    OriginalResourceEntryVector_26d138 empty(0);
    ck(empty.originalCount_26d120()==0,"Original zero entry count");
    bool failed=false;
    try{empty.originalAppend_26d138({});}catch(const std::length_error&){failed=true;}
    ck(failed,"Original full-vector guard");
    OriginalResourceEntryVector_26d138 duplicate(2);
    duplicate.originalAppend_26d138({"test","exdb",{std::byte{0x45}},0});
    auto* prior=&duplicate.originalAt_26d214(0);
    duplicate.originalAppend_26d138({"test","exdb",{std::byte{0x44}},1});
    ck(prior==&duplicate.originalAt_26d214(0),"Reserve keeps entry address stable");
    ck(duplicate.originalAt_26d214(1).data()[0]==std::byte{0x44},"Original entry order");
    failed=false;
    try{duplicate.originalAppend_26d138({});}catch(const std::length_error&){failed=true;}
    ck(failed,"Original capacity error");
    failed=false;
    try{(void)duplicate.originalAt_26d214(2);}catch(const std::out_of_range&){failed=true;}
    ck(failed,"Host checked entry access");

    ck(originalResourceTypeMatch_26d2b8("exdb","exdb"),"Original exact type accepted");
    ck(originalResourceTypeMatch_26d2b8("exdb extra","exdb"),"Original source space suffix accepted");
    ck(originalResourceTypeMatch_26d2b8(std::string_view("exdb\0ignored",12),"exdb"),"Original source nul suffix accepted");
    ck(!originalResourceTypeMatch_26d2b8("exdb-extra","exdb"),"Source only permits space or NUL after type");
    ck(!originalResourceTypeMatch_26d2b8("exdb","ex"),"Original non-boundary prefix rejected");
    ck(!originalResourceTypeMatch_26d2b8("EXDB","exdb"),"Original type case preserved");
    ck(!originalResourceTypeMatch_26d2b8("exdb","exdb "),"Long query rejected");
    duplicate.originalClear_26d1b0();
    ck(duplicate.originalCount_26d120()==0&&duplicate.originalCapacity()==2,"Source vector clear without capacity change");
    // Original source package fixture is private and never committed.
    auto originalPck=readFile(argv[1]);
    auto resources=originalResourceEntryVectorFromPck(originalPck);
    ck(resources.originalCount_26d120()==2,"Original two PCK members");
    const auto& monkey=resources.originalAt_26d214(0);
    const auto& camera=resources.originalAt_26d214(1);
    ck(monkey.sourceName=="monkeyprofile.exdb" && monkey.sourceType=="exdb", "Original monkey PCK metadata");
    ck(monkey.originalPckPayloadOffset==0x60 && monkey.data().size()==0x1400,"Original monkey PCK offset");
    ck(camera.sourceName=="MainCameraParam.exdb" && camera.sourceType=="exdb", "Original camera PCK metadata");
    ck(camera.originalPckPayloadOffset==0x1460 && camera.data().size()==0x200,"Original camera PCK offset");
    ck(originalBgmDescriptorResource_35fd20(resources)==nullptr,"No bgm_desc.exdb in woods_c PCK");
    auto filtered=originalFilterResourceEntriesFromPck_26d24c(originalPck,"exdb");
    ck(filtered.originalCount_26d120()==2,"Original requested resource class selects both entries");
    ck(filtered.originalAt_26d214(0).sourceName=="monkeyprofile.exdb","Source original PCK traversal preserves order");
    ck(originalFilterResourceEntriesFromPck_26d24c(originalPck,"ex").originalCount_26d120()==0,
       "Original class filter is NOT arbitrary prefix match");
    ck(originalFilterResourceEntriesFromPck_26d24c(originalPck,"tm2").originalCount_26d120()==0,
       "Original class filter excludes unrelated types");
    OriginalExdbDecodedDocument doc;
    doc.decode(camera.data());
    ck(doc.originalRecordIndex().sourceCountAt20==1 && doc.originalRecordIndex().sourceStrideAt24==160,"Original camera data source");
    const auto rec=doc.record(0);
    ck(originalAttributeString_2bb3bc(rec,doc.decodedAttributeRoot(),"name","?")=="camparam_default","Actual retail camera name");
    ck(originalAttributeFloat_2bb1d0(rec,doc.decodedAttributeRoot(),"lkdown_ang",-1.f)==30.f,"Actual retail camera angle");
    // Native deep-owned source entry retains genuine game data after
    // the package's original raw allocation is freed/unmapped.
    originalPck.clear();originalPck.shrink_to_fit();
    doc.decode(resources.originalAt_26d214(1).data());
    ck(originalAttributeString_2bb3bc(doc.record(0),doc.decodedAttributeRoot(),"area","?")=="woods_c","PCK lifetime independence");
    OriginalResourceEntryVector_26d138 manual(1);
    manual.originalAppend_26d138({"bgm_desc.exdb","exdb",{std::byte{'E'},std::byte{'D'},std::byte{'B'},std::byte{'\n'}},0});
    const auto* bgm=originalBgmDescriptorResource_35fd20(manual);
    ck(bgm && bgm->data().size()==4,"Authentic source-name/type filter logic");
}
