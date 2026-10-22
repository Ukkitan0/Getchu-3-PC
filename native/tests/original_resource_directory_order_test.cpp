#include "ae3/original_resource_directory_order.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>
using namespace ae3;
static void ck(bool good,const char* reason){if(!good)throw std::runtime_error(reason);}
int main(){
 try{
    const std::vector<VfiEntry> source{
      {"debug/us/stage/woods_c/z.pck",100,10},
      {"debug/us/stage/woods_c/b.pck",120,11},
      {"debug/us/stage/woods_c/A.plc",130,15},
      {"debug/us/stage/woods_c/a.asq",150,18},
      {"debug/us/stage/woods_c/monkeydb.xml",170,20},
      {"debug/us/stage/woods_c/sub/Z.nav",190,15},
      {"debug/us/stage/woods_c/sub/another.nav",210,15},
      {"debug/us/stage/woods_c/sub/deeper/not_a_direct_file.nav",230,15},
      {"debug/us/stage/other_c/here.pck",250,10}
    };
    OriginalStageResourceCatalog actual(source);
    const auto root=originalStageDirectoryVfiView_251958(actual,"woods_c");
    ck(root.size()==5,"Original root directory member count");
    const std::array<std::string_view,5> order{
      "A.plc","a.asq","b.pck","monkeydb.xml","z.pck"
    };
    for(std::size_t i=0;i<order.size();++i){
        ck(root[i].originalLeafFilename==order[i],"Original case-sensitive source tree order");
        ck(root[i].originalStageRelativePath==order[i],"Original VFI leaf path");
        ck(actual.entries()[root[i].originalCatalogIndex].sourceEntry.offset>0,
           "Original source VFI entry index/offset identity");
    }
    const auto sub=originalStageDirectoryVfiView_251958(actual,"woods_c","sub");
    ck(sub.size()==2 && sub[0].originalLeafFilename=="Z.nav" &&
       sub[1].originalLeafFilename=="another.nav","Original nested dir key order");
    ck(originalResourceDirectoryKeyLess_627e60("abc","abcd"),"Shorter source prefix comes first");
    ck(!originalResourceDirectoryKeyLess_627e60("abcd","abc"),"Longer source suffix comes last");
    ck(originalResourceDirectoryKeyLess_627e60("A","a"),"Unsigned source byte comparison");
    ck(originalResourceDirectoryKeyLess_627e60("a","b"),"Original source ascending keys");
    ck(originalStageDirectoryVfiView_251958(actual,"other_c").size()==1,
       "Original stage directory names do not leak across stages");
    ck(originalStageDirectoryVfiView_251958(actual,"woods_c","absent").empty(),
       "Original absent folder cannot fabricate VFI resources");
    bool rejected=false;
    try{(void)originalStageDirectoryVfiView_251958(actual,"woods_c","../other_c");}
    catch(const std::invalid_argument&){rejected=true;}
    ck(rejected,"Original directory traversal must reject unsafe source directory");
    std::cout<<"ORIGINAL_STAGE_DIRECTORY_ORDER_TEST_PASSED root=5 nested=2"
             <<" comparator=source_bytewise order=rb_tree_inorder\n";
    return 0;
 }catch(const std::exception& e){
    std::cerr<<"ORIGINAL_STAGE_DIRECTORY_ORDER_TEST_FAILED: "<<e.what()<<'\n';
    return 1;
 }
}
