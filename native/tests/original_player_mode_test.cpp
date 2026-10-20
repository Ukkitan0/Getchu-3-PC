#include "ae3/original_player_mode.hpp"
#include <array>
#include <stdexcept>
static void check(bool b){if(!b)throw std::runtime_error("Original Player routing mismatch");}
#include <cstdint>
#include <limits>
#include <string>
struct Capture final:ae3::OriginalPlayerModeCalls {
    std::string call;
    void call_33fce4(std::uint32_t n) override {call="33fce4:"+std::to_string(n);}
    void call_34675c(float n) override {call="34675c:"+std::to_string(int(n));}
    void call_3509ac(std::uint32_t n,std::uint32_t one) override {
        check(one==1);call="3509ac:"+std::to_string(n);
    }
    void call_33fd90() override {call="33fd90";}
    void call_324260(bool b) override {call=std::string("324260:")+(b?"1":"0");}
};
int main(){
    constexpr std::array<const char*,12> expected={
      "33fce4:1","33fce4:5","33fce4:10","", "34675c:20",
      "34675c:100","","3509ac:1","3509ac:2","33fd90",
      "324260:0","324260:1"
    };
    Capture trace;
    for(std::uint32_t i=0;i<expected.size();i++){
        trace.call.clear();
        const bool result=ae3::dispatchOriginalPlayerMode(i,trace);
        check(trace.call==expected[i]);
        check(result==(expected[i][0]!='\0'));
    }
    trace.call.clear();
    check(!ae3::dispatchOriginalPlayerMode(12,trace));
    check(!ae3::dispatchOriginalPlayerMode(std::numeric_limits<std::uint32_t>::max(),trace));
    check(trace.call.empty());
}
