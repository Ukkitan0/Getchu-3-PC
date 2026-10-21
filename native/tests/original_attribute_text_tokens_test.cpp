#include "ae3/original_attribute_text_tokens.hpp"
#include <array>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
using namespace ae3;
static void ck(bool b) {if(!b)throw std::runtime_error("retail attribute source token regression");}
static void caseSplit(std::string_view s,char sep,std::vector<std::string> expected){
    ck(originalSplitDelimited_1f28c4(s,sep)==expected);
}
int main(){
    caseSplit("",':',{});
    caseSplit(":",':',{ "" });
    caseSplit("::",':',{"",""});
    caseSplit("A",':',{"A"});
    caseSplit("A:B",':',{"A","B"});
    caseSplit("A:B:",':',{"A","B"});
    caseSplit(":A:",':',{"","A"});
    caseSplit("A::B",':',{"A","","B"});
    caseSplit(std::string_view("AA\0BB:CC",8),':',{std::string("AA\0BB",5),"CC"});
    caseSplit("A\r\nB\nC\n",'\n',{"A\r","B","C"});
    auto attrs=originalAttributeSplitColon_2ba5f4("node:track:4:1024");
    ck(attrs.colonFields==std::vector<std::string>({"node","track","4","1024"}));
    auto tokens=originalAttributeTokenizeSource_2bb03c("x:y:12\n\nq::57\n");
    ck(tokens.physicalSegmentCount==3);
    ck(tokens.sourceLines[0].colonFields==std::vector<std::string>({"x","y","12"}));
    ck(tokens.sourceLines[1].sourceLine.empty());
    ck(tokens.sourceLines[1].colonFields.empty());
    ck(tokens.sourceLines[2].colonFields==std::vector<std::string>({"q","","57"}));
    std::mt19937 rng(0x198e4c3u);
    for(int k=0;k<25000;++k){
        const int len=int(rng()%128);
        std::string str;str.reserve(size_t(len));
        std::vector<std::string> expected;
        std::string accum;
        for(int i=0;i<len;++i){
            char c=char(rng()%10==0?':':(rng()%26+'a'));
            str.push_back(c);
            if(c==':') {expected.push_back(accum);accum.clear();}
            else accum.push_back(c);
        }
        if(!accum.empty())expected.push_back(accum);
        ck(originalSplitDelimited_1f28c4(str,':')==expected);
    }
}
