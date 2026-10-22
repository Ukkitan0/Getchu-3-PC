#include "ae3/original_resource_stage_scope.hpp"
#include "ae3/original_attribute_string.hpp"
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
using namespace ae3;
static void ck(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv){
 try {
  if(argc!=2)throw std::invalid_argument(
    "Pass PRIVATE owner-verified original USA ISO or DATA.BIN (never commit the file)");
  // Source evidence: original USA vfi/debug/us/stage/woods_c/exdb.pck;
  // SHA-256 185db8cfd710760d9c715e0d1cce8ac4ecf6b18201673ce00bc6815078ae4615.
  // Check the retail ISO/ELF hashes separately with existing owner-private verifiers.
  OriginalStageResourceScope stage(std::filesystem::path(argv[1]),"woods_c");
  ck(stage.loadedPackageCount()==0 && stage.decodedExdbCount()==0,
     "Original stage scope must not allocate packages before first access");
  const auto& package=stage.package("exdb.pck.sz");
  ck(package.entries().size()==2,"Retail woods_c exdb.pck contains 2 entries");
  ck(stage.loadedPackageCount()==1,"Original package allocation is not reused");
  const auto& monkey=stage.requireMember("exdb.pck.sz","monkeyprofile.exdb","exdb");
  const auto& camera=stage.requireMember("exdb.pck.sz","MainCameraParam.exdb","exdb");
  ck(monkey.payloadOffset==0x60 && monkey.payload.size()==0x1400,
     "Retail monkey source payload offset/size");
  ck(camera.payloadOffset==0x1460 && camera.payload.size()==0x200,
     "Retail camera source payload offset/size");
  ck(originalPckPayloadSignature(monkey.payload)==OriginalPckPayloadSignature::exdb &&
     originalPckPayloadSignature(camera.payload)==OriginalPckPayloadSignature::exdb,
     "Original EXDB payload signatures changed after ISO/VFI loading");
  ck(stage.filterMembers_26d24c("exdb.pck.sz","exdb").size()==2,
     "Retail 0x26D24C type-select source count");
  const auto& cameraDoc=stage.exdb("exdb.pck.sz","MainCameraParam.exdb");
  ck(stage.decodedExdbCount()==1,"Original camera EXDB document owner missing");
  ck(&cameraDoc==&stage.exdb("EXDB.PCK.SZ","MainCameraParam.exdb"),
     "Decoded original EXDB owner must be stable and canonically cached");
  const auto cameraIndex=cameraDoc.originalRecordIndex();
  ck(cameraIndex.sourceCountAt20==1 && cameraIndex.sourceStrideAt24==160 &&
     cameraIndex.sourceRecordBaseAt04==352,
     "Retail camera record index");
  const auto cameraRecord=cameraDoc.record(0);
  ck(originalAttributeString_2bb3bc(cameraRecord,cameraDoc.decodedAttributeRoot(),
      "name","missing")=="camparam_default","Retail camera resource name");
  ck(originalAttributeString_2bb3bc(cameraRecord,cameraDoc.decodedAttributeRoot(),
      "area","missing")=="woods_c","Retail camera source area");
  ck(originalAttributeFloat_2bb1d0(cameraRecord,cameraDoc.decodedAttributeRoot(),
      "lkdown_ang",-1.f)==30.f,"Original camera angle");
  const auto& monkeyDoc=stage.exdb("exdb.pck.sz","monkeyprofile.exdb");
  const auto monkeyIndex=monkeyDoc.originalRecordIndex();
  ck(monkeyIndex.sourceCountAt20==4 && monkeyIndex.sourceStrideAt24==1024 &&
     monkeyIndex.sourceRecordBaseAt04==1024,
     "Original monkey profile count/stride");
  ck(monkeyDoc.record(3).size()==1024,"Original fourth monkey profile unreadable");
  ck(stage.decodedExdbCount()==2 && stage.loadedPackageCount()==1,
     "Stage scope duplicated PCK or leaked decoded EXDB documents");
  const auto* stillValid=camera.payload.data();
  // Original nav graph is an independently recovered subsystem. If this
  // stage actually includes its original navigation.pck.sz member, verify
  // authentic source type/name and preserve the EXDB source lifetime.
  if(stage.catalog().findExact("woods_c","navigation.pck.sz")){
     const auto& nav=stage.package("navigation.pck.sz");
     const auto selected=nav.selectType_26d24c("nav");
     ck(!selected.empty(),"Original navigation package has no original nav entries");
     const auto& source=stage.requireMember("navigation.pck.sz","nav_monkey.nav","nav");
     ck(source.payload.size()>=0x28,"Original nav_monkey.nav missing +0x24 count field");
     ck(stage.loadedPackageCount()==2,"Original nav package should load once");
  }
  ck(camera.payload.data()==stillValid &&
     originalAttributeString_2bb3bc(cameraDoc.record(0),cameraDoc.decodedAttributeRoot(),
       "area","missing")=="woods_c",
       "Original EXDB source invalidated by loading independent stage resources");
  std::cout<<"ORIGINAL_STAGE_RESOURCE_SCOPE_RETAIL_TEST_PASSED stage=woods_c"
           <<" original_exdb=2 owned_packages="<<stage.loadedPackageCount()
           <<" source_data_unchanged=1\n";
  return 0;
 }catch(const std::exception& e){
  std::cerr<<"ORIGINAL_STAGE_RESOURCE_SCOPE_RETAIL_TEST_FAILED: "<<e.what()<<'\n';
  return 1;
 }
}
