#include "ae3/original_nav_route_dispatch.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>
using namespace ae3;
static void check(bool x,const char* msg){if(!x)throw std::runtime_error(msg);}
static void word16(std::vector<std::byte>& b,std::size_t at,std::uint16_t n){
 b.at(at)=std::byte(n&255u);b.at(at+1)=std::byte(n>>8);
}
static void word32(std::vector<std::byte>& b,std::size_t at,std::uint32_t n){
 for(int j=0;j<4;j++)b.at(at+j)=std::byte((n>>(8*j))&255u);
}
struct Cleanup:OriginalNavRouteReleaseCalls {
 void* expected=nullptr;int count=0;
 void call_original_41a3d8(void* p)override{
  check(p==expected,"Original route allocator argument mismatch");count++;
 }
};
int main(){
 const std::array<float,5> x{{0,2,4,6,20}};
 std::vector<OriginalSpatialRecord70> records(5);
 std::vector<std::byte> nav(0x80+5*0x1e);
 word32(nav,0x24,5);word32(nav,0x3c,0x80);
 for(std::size_t i=0;i<x.size();i++){
  const auto bits=std::bit_cast<std::uint32_t>(x[i]);
  for(int j=0;j<4;j++)
   records[i].originalFields[0x40+j]=std::byte((bits>>(8*j))&255u);
  word16(nav,0x80+0x1e*i+0x16,1);
 }
 const auto edge=[&](std::size_t src,int slot,std::uint16_t target){
  const auto at=0x80+0x1e*src;
  word16(nav,at+2,static_cast<std::uint16_t>(slot+1));
  word16(nav,at+4+2*slot,target);
 };
 edge(0,0,1);edge(0,1,4);edge(1,0,2);edge(2,0,3);
 OriginalNavDispatchRequest query{};
 query.sourceSeedIndex=0;query.maximumDistance=10;
 query.originalReference={0,0,0,0};query.routeMode=0;
 const auto far=originalNavRouteDispatch_2207d0(records,nav,query);
 check(!far.fallbackUsed&&far.selectedEndpoint==3,"Original farthest mode");
 check(far.forwardPath.size()==4&&far.forwardPath.back()==&records[3],"Original farthest predecessor path");
 check(far.originalAdmittedRecordIndices==std::vector<std::uint32_t>({0,1,2,3}),"Original DFS order");
 query.routeMode=1;
 const auto near=originalNavRouteDispatch_2207d0(records,nav,query);
 check(near.selectedEndpoint==0&&near.forwardPath.size()==1,"Original nearest mode");
 query.routeMode=2;
 const auto none=originalNavRouteDispatch_2207d0(records,nav,query);
 check(none.selectedEndpoint==-1&&none.forwardPath.empty(),"Original unset $8 must pass -1");
 query.explicitDestinationSearchIndex=2;
 const auto explicitRoute=originalNavRouteDispatch_2207d0(records,nav,query);
 check(explicitRoute.selectedEndpoint==2&&explicitRoute.forwardPath.size()==3&&
       explicitRoute.forwardPath.back()==&records[2],"Original independent $8 index");
 query.explicitDestinationSearchIndex=99;
 bool badDestination=false;
 try{(void)originalNavRouteDispatch_2207d0(records,nav,query);}
 catch(const std::out_of_range&){badDestination=true;}
 check(badDestination,"Original explicit index bounds");
 query.explicitDestinationSearchIndex=-1;
 const std::array<const OriginalSpatialRecord70*,1> filter{{&records[0]}};
 query.sourceMembershipListExists=true;
 query.orderedOriginalMembershipList=filter;
 query.routeMode=0;
 const auto kept=originalNavRouteDispatch_2207d0(records,nav,query);
 check(!kept.fallbackUsed&&kept.originalAdmittedRecordIndices.size()==1,"Original membership filter");
 const std::array<const OriginalSpatialRecord70*,2> fallbackMembers{{&records[3],&records[2]}};
 query.orderedOriginalMembershipList=fallbackMembers; // original seed 0 is not in list
 const auto alternate=originalNavRouteDispatch_2207d0(records,nav,query);
 check(alternate.fallbackUsed&&alternate.fallbackPolygon==&records[2],"Original fallback list nearest");
 check(alternate.selectedEndpoint==2&&alternate.forwardPath.size()==3,"Original fallback nearest endpoint");
 check(alternate.originalAdmittedRecordIndices.size()==4,"Original fallback must disable admission filtering");
 const std::array<const OriginalSpatialRecord70*,2> ties{{&records[2],&records[3]}};
 check(originalNavListNearest_4e3dec(records,nav,ties,{5,0,0,0})==&records[2],
       "Original linked-list strict tie must keep first");
 check(originalNavListNearest_4e3dec(records,nav,{},{0,0,0,0})==nullptr,"Original empty list");
 query.orderedOriginalMembershipList={};
 bool caught=false;
 try{(void)originalNavRouteDispatch_2207d0(records,nav,query);}
 catch(const std::runtime_error&){caught=true;}
 check(caught,"Original missing fallback must not fabricate a polygon");
 OriginalSpatialRecord70 foreign{};
 const std::array<const OriginalSpatialRecord70*,1> unknown{{&foreign}};
 caught=false;
 try{(void)originalNavListNearest_4e3dec(records,nav,unknown,{0,0,0,0});}
 catch(const std::invalid_argument&){caught=true;}
 check(caught,"Unrecognized original source polygon not rejected");
 Cleanup free;free.expected=&records[0];
 originalNavRouteRelease_220a4c(nullptr,free);
 originalNavRouteRelease_220a78(nullptr,free);
 check(free.count==0,"Original null route allocation freed");
 originalNavRouteRelease_220a4c(&records[0],free);
 originalNavRouteRelease_220a78(&records[0],free);
 check(free.count==2,"Original both route destructors did not call 0x41A3D8");
}
