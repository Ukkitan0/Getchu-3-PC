#include "ae3/original_resource_stage_catalog.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void ck(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F> static void rejects(F f) {
    bool thrown=false;try{f();}catch(const std::exception&){thrown=true;}
    ck(thrown,"Malformed original stage VFI path was accepted");
}
int main(){
 try {
    const std::vector<VfiEntry> fixtures{
        {"debug/us/stage/woods_c/area.luc",0x800,0x100},
        {"debug/us/stage/woods_c/navigation.pck.sz",0xa00,0x900},
        {"debug/us/stage/woods_c/exdb.pck",0x2000,0x1660},
        {"debug/us/stage/woods_c/bg.pck.sz",0x6000,0x400},
        {"debug/us/stage/other_b/area.luc",0x7000,0x100},
        {"debug/us/stage/other_b/animation/clip.i3m",0x8000,0x120},
        {"debug/us/stages/woods_c/area.luc",0x9000,0x20},
        {"debug/us/stage/woods_c",0x9100,0x10},
        {"debug/us/common/bg.pck",0xa000,0x900}
    };
    OriginalStageResourceCatalog catalog(fixtures);
    ck(catalog.stageNames().size()==2,"Actual stage count");
    ck(catalog.entries().size()==6,"Only original stage resources are included");
    ck(catalog.entriesForStage("woods_c").size()==4,"Original stage file count");
    ck(catalog.entriesForStage("OTHER_B").size()==2,"Original stage ASCII folding");
    ck(catalog.findExact("woods_c","area.luc")==0,"Exact original stage area script");
    ck(catalog.findExact("woods_c","navigation.pck.sz")==1,"Exact original navigation source");
    ck(!catalog.findExact("woods_c","navigation.pck"),"No generated SZ-less alias");
    ck(!catalog.findExact("woods_c","nonexistent.pck"),"No invented source files");
    ck(catalog.entries()[1].sourceEntry.offset==0xa00 &&
       catalog.entries()[1].sourceEntry.size==0x900,"Original VFI offsets remain intact");
    ck(catalog.entries()[1].originalSzCompressed &&
       catalog.entries()[1].uncompressedName=="navigation.pck","Original SZ logical basename");
    ck(!catalog.entries()[0].originalSzCompressed &&
       catalog.entries()[0].uncompressedName=="area.luc","Original Lua member name");
    ck(catalog.entries()[3].uncompressedName=="bg.pck","Actual background package basename");
    rejects([&]{(void)catalog.findExact("woods_c","../exdb.pck");});
    rejects([&]{(void)catalog.findExact("woods_c","navigation.pck.sz/../exdb.pck");});
    rejects([&]{(void)catalog.entriesForStage("../woods_c");});
    rejects([&]{const std::array<VfiEntry,1> bad{{{"debug/us/stage/woods_c/../oops.pck",1,2}}};OriginalStageResourceCatalog x(bad);});
    rejects([&]{const std::array<VfiEntry,1> bad{{{"debug/us/stage/woods_c/dir\\escape.pck",1,2}}};OriginalStageResourceCatalog x(bad);});
    rejects([&]{const std::array<VfiEntry,2> bad{{{"debug/us/stage/woods_c/bg.pck",1,2},{"DEBUG/US/STAGE/WOODS_C/BG.PCK",3,4}}};OriginalStageResourceCatalog x(bad);});
    std::cout<<"ORIGINAL_RESOURCE_STAGE_CATALOG_TEST_PASSED stages=2 members=6 malicious=6\n";
    return 0;
 } catch(const std::exception& e) {
    std::cerr<<"ORIGINAL_RESOURCE_STAGE_CATALOG_TEST_FAILED "<<e.what()<<'\n';return 1;
 }
}
