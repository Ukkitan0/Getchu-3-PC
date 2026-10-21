#include "ae3/original_nav_route_dispatch.hpp"
#include <cfloat>
#include <stdexcept>
namespace ae3 {
namespace {
std::uint16_t le16(std::span<const std::byte> b,std::size_t i){
 if(i>b.size()||b.size()-i<2)throw std::out_of_range("Original NavGraph source truncated");
 return static_cast<std::uint16_t>(
  std::to_integer<unsigned char>(b[i])|
  (unsigned(std::to_integer<unsigned char>(b[i+1]))<<8));
}
std::uint32_t le32(std::span<const std::byte> b,std::size_t i){
 return std::uint32_t(le16(b,i))|(std::uint32_t(le16(b,i+2))<<16);
}
std::size_t sourceIndex(std::span<const OriginalSpatialRecord70> records,
                        const OriginalSpatialRecord70* ptr){
 for(std::size_t i=0;i<records.size();i++)
  if(&records[i]==ptr)return i;
 throw std::invalid_argument("Original source-linked list contains a foreign polygon");
}
OriginalVU0Vec4 centroid(std::span<const OriginalSpatialRecord70> records,
                        std::span<const std::byte> nav,
                        const OriginalSpatialRecord70* polygon){
 const auto count=static_cast<std::size_t>(le32(nav,0x24));
 const auto table=static_cast<std::size_t>(le32(nav,0x3c));
 if(records.size()!=count||table>nav.size()||count>(nav.size()-table)/0x1e)
  throw std::out_of_range("Original NavGraph descriptor array truncated");
 const auto i=sourceIndex(records,polygon);
 const auto n=le16(nav,table+i*0x1e + 0x16);
 if(n==0||n>3)throw std::out_of_range("Original polygon vertex count not reconstructed");
 const auto points=originalSpatialVertices_21b468(*polygon);
 return originalVU0Mean_21b468(std::span(points).first(n));
}
}
const OriginalSpatialRecord70* originalNavListNearest_4e3dec(
 std::span<const OriginalSpatialRecord70> records,
 std::span<const std::byte> nav,
 std::span<const OriginalSpatialRecord70* const> list,
 const OriginalVU0Vec4& query){
 const OriginalSpatialRecord70* best=nullptr;
 float distance=FLT_MAX; // original retail float at 0x6E3F00
 for(const auto* p:list){
  const auto position=centroid(records,nav,p);
  const float d=originalVU0DistanceXYZ_21b234(position,query);
  if(d<distance){distance=d;best=p;} // original strict C.OLT.S
 }
 return best;
}
OriginalNavDispatchResult originalNavRouteDispatch_2207d0(
 std::span<const OriginalSpatialRecord70> records,
 std::span<const std::byte> nav,
 const OriginalNavDispatchRequest& req,
 OriginalNavRouteSplineCalls* spline){
 if(req.sourceSeedIndex>=records.size())
  throw std::out_of_range("Original route seed outside source graph");
 // Original $7 is owner, not candidate. 0x220870 passes seed ($5)
 // to 0x21F89C, which reads owner+4 and calls 0x4E3DAC.
 const bool normal=!req.sourceMembershipListExists||
  originalNavMembership_4e3dac(&records[req.sourceSeedIndex],
                                req.orderedOriginalMembershipList);
 OriginalNavDispatchResult result;
 result.fallbackUsed=!normal;
 OriginalVU0Vec4 reference=req.originalReference;
 if(!normal){
  // 0x21F8B8 reads graph+8: the SAME $7 owner stored at 0x22085C;
  // 0x4E3DEC chooses first strict-nearest polygon.
  result.fallbackPolygon=originalNavListNearest_4e3dec(
   records,nav,req.orderedOriginalMembershipList,req.originalReference);
  if(!result.fallbackPolygon)
   throw std::runtime_error("Original route fallback linked list is unresolved/empty");
  reference=centroid(records,nav,result.fallbackPolygon);
 }
 OriginalNavAdmissionRequest admission;
 admission.startRecordIndex=req.sourceSeedIndex; // unchanged in fallback
 admission.query=req.originalReference;
 admission.maximumDistance=req.maximumDistance;
 // 0x220888 a3=1 normal, 0x220960 a3=0 fallback;
 // original 0x220B00 also requires the $7 pointer at work+8.
 admission.filterEnabled=normal && req.sourceMembershipListExists;
 admission.originalFilterListExists=req.sourceMembershipListExists;
 admission.originalFilterList=req.orderedOriginalMembershipList;
 OriginalNavEpochState epoch;
 const auto accepted=originalNavAdmit_220aa4(records,nav,admission,epoch);
 result.originalAdmittedRecordIndices=accepted.sourceRecordIndicesInDepthFirstOrder;
 auto workspace=originalNavBuildAdmittedWorkspace_220c00(records,nav,accepted);
 originalNavRouteRelax_220d5c(workspace,spline);
 if(!workspace.nodes.empty()){
  // 0x220924 / 0x220938 / 0x2208AC: nearest, farthest or -1.
  // 0x22098C fallback always goes through 0x220924 (nearest).
  if(!normal||req.routeMode==1)
   result.selectedEndpoint=originalNavRouteNearest_220f60(workspace,reference);
  else if(req.routeMode==0)
   result.selectedEndpoint=originalNavRouteFarthest_221030(workspace,reference);
  else {
   // 0x2208B8: independent $8 search index, -1 is no-route.
   if(req.explicitDestinationSearchIndex < -1 ||
      (req.explicitDestinationSearchIndex >= 0 &&
       std::size_t(req.explicitDestinationSearchIndex)>=workspace.nodes.size()))
    throw std::out_of_range("Original explicit route search index out of range");
   result.selectedEndpoint=req.explicitDestinationSearchIndex;
  }
  result.forwardPath=originalNavRouteExtract_220994(workspace,result.selectedEndpoint);
 }
 return result;
}
void originalNavRouteRelease_220a4c(void* p,OriginalNavRouteReleaseCalls& call){
 if(p)call.call_original_41a3d8(p);
}
void originalNavRouteRelease_220a78(void* p,OriginalNavRouteReleaseCalls& call){
 if(p)call.call_original_41a3d8(p);
}
} // namespace ae3
