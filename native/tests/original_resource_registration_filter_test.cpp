#include "ae3/original_resource_registration_filter.hpp"
#include <iostream>
#include <stdexcept>
#include <string_view>
using namespace ae3;
static void check(bool yes,const char* why){if(!yes)throw std::runtime_error(why);}
int main(){
 try{
    check(originalResourceAcceptSelector_2519d4("placeholder",nullptr),"Retail null selector");
    check(originalResourceAcceptSelector_2519d4("navigation","*"),"Original source wildcard bypass");
    check(originalResourceAcceptSelector_2519d4("actionseq","*not-a-special-token"),"Original checks ONLY first wildcard byte");
    check(originalResourceAcceptSelector_2519d4("navigation","navigation"),"Original exact-name compare");
    check(!originalResourceAcceptSelector_2519d4("navigation","nav"),"Original source prefix rejected");
    check(!originalResourceAcceptSelector_2519d4("navigation","Navigation"),"Source name comparison case-sensitive");
    check(!originalResourceAcceptSelector_2519d4("navigation",""),"Empty selector not wildcard");
    check(!originalResourceAcceptSelector_2519d4("actionseq","navigation"),"Different source name rejected");
    std::cout<<"ORIGINAL_RESOURCE_REGISTRATION_FILTER_TEST_PASSED original_va=0x25182C wildcard=1 exact=1\n";
    return 0;
 }catch(const std::exception& e){
    std::cerr<<"ORIGINAL_RESOURCE_REGISTRATION_FILTER_TEST_FAILED: "<<e.what()<<'\n';return 1;
 }
}
