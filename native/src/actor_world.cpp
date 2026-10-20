#include "ae3/actor_world.hpp"
#include "ae3/level_scene.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ae3 {
namespace {
namespace fs=std::filesystem;
constexpr std::size_t kMaxActors=64,kMaxVerts=3'000'000,kMaxTextures=512;
std::string lowered(std::string text){
    std::transform(text.begin(),text.end(),text.begin(),[](unsigned char v){
        return static_cast<char>(std::tolower(v));
    });
    return text;
}
void warn(ActorBindingReport& r,const std::string& s){
    if(r.diagnostics.size()<32)r.diagnostics.push_back(s);
}
bool within(const fs::path& path,const fs::path& root){
    auto at=path.begin();
    for(const auto& element:root){
        if(at==path.end()||lowered(at->string())!=lowered(element.string()))
            return false;
        ++at;
    }
    return true;
}
bool validPosition(float n){return std::isfinite(n)&&std::abs(n)<1e7f;}
bool hasExplicitPosition(const StageEntity& e){
    for(const char* name:{"pos","position","xyz","x","y","z"})
        if(e.fields.member(name))return true;
    return false;
}
std::array<float,3> positionFrom(const StageEntity& e){
    const ScriptValue* point=nullptr;
    for(const char* name:{"pos","position","xyz"}){
        if((point=e.fields.member(name)))break;
    }
    std::array<float,3> raw{0,0,0};
    if(point){
        if(point->kind==ScriptValue::Kind::array && point->array.size()>=3){
            for(int i=0;i<3;++i)
                raw[std::size_t(i)]=float(point->array[std::size_t(i)].numberOr());
        }else if(point->kind==ScriptValue::Kind::object){
            for(int i=0;i<3;++i){
                const auto* n=point->member(std::string(1,"xyz"[i]));
                if(n)raw[std::size_t(i)]=float(n->numberOr());
            }
        }
    }else{
        for(int i=0;i<3;++i){
            const auto* n=e.fields.member(std::string(1,"xyz"[i]));
            if(n)raw[std::size_t(i)]=float(n->numberOr());
        }
    }
    if(!validPosition(raw[0])||!validPosition(raw[1])||!validPosition(raw[2]))
        throw std::runtime_error("script actor has non-finite/out-of-range spawn coordinates");
    return {raw[0],raw[1],-raw[2]}; // game RH XYZ to D3D LH
}
float actorYaw(const StageEntity& entity){
    const auto* y=entity.fields.member("yaw");
    // Do not guess degrees-vs-radians for unknown original fields.
    if(!y)return 0;
    const float angle=float(y->numberOr());
    if(!std::isfinite(angle)||std::abs(angle)>1000)
        throw std::runtime_error("script actor yaw out of range");
    return angle;
}
std::map<std::string,std::string> readBindings(const fs::path& file){
    std::map<std::string,std::string> out;
    if(file.empty())return out;
    std::ifstream input(file);
    if(!input)throw std::runtime_error("private actor model bindings file not found");
    std::string line;
    std::size_t n=0;
    while(std::getline(input,line)){
        if(++n>512||line.size()>1024)
            throw std::runtime_error("private actor binding resource budget exceeded");
        if(line.empty()||line[0]=='#')continue;
        const auto equal=line.find('=');
        if(equal==std::string::npos||equal==0||equal+1>=line.size())
            throw std::runtime_error("actor mapping requires key=relative_model.glb");
        auto key=line.substr(0,equal),value=line.substr(equal+1);
        if(key.size()>256||value.size()>512||
           lowered(fs::path(value).extension().string())!=".glb")
            throw std::runtime_error("actor mapping must contain a safe GLB path");
        out[lowered(key)]=value;
    }
    return out;
}
std::map<std::string,std::vector<fs::path>> indexModels(const fs::path& root){
    if(!fs::is_directory(root))throw std::runtime_error(
        "Lua actors need a PRIVATE GLB asset folder (choose Open GLB level folder first)");
    std::map<std::string,std::vector<fs::path>> candidates;
    std::size_t count=0;
    for(auto it=fs::recursive_directory_iterator(
         root,fs::directory_options::skip_permission_denied);
        it!=fs::recursive_directory_iterator();++it){
        if(it->is_symlink()){
            if(it->is_directory())it.disable_recursion_pending();
            continue;
        }
        if(!it->is_regular_file()||
           lowered(it->path().extension().string())!=".glb")continue;
        if(++count>2048)
            throw std::runtime_error("private actor model directory contains too many GLBs");
        candidates[lowered(it->path().stem().string())].push_back(it->path());
    }
    return candidates;
}
fs::path bindModel(
    const StageEntity& entity,const fs::path& root,
    const std::map<std::string,std::string>& overrides,
    const std::map<std::string,std::vector<fs::path>>& candidates,
    ActorBindingReport& report){
    std::string requested;
    const auto ent=overrides.find(lowered(entity.name));
    const auto cls=overrides.find("class:"+lowered(entity.entityClass));
    if(ent!=overrides.end())requested=ent->second;
    else if(cls!=overrides.end())requested=cls->second;
    if(!requested.empty()){
        fs::path relative=fs::u8path(requested);
        if(relative.is_absolute()||relative.has_root_path())
            throw std::runtime_error("absolute actor binding paths rejected");
        std::error_code ec;
        const auto path=fs::weakly_canonical(root/relative,ec);
        if(ec||!within(path,root)||!fs::is_regular_file(path))
            throw std::runtime_error("actor mapping references absent/out-of-root model");
        return path;
    }
    if(const auto* model=entity.fields.member("model_name"))
        requested=model->stringOr();
    if(requested.empty())requested=entity.name;
    if(requested.empty())return {};
    const auto stem=lowered(fs::path(requested).stem().string());
    const auto result=candidates.find(stem);
    if(result==candidates.end())return {};
    if(result->second.size()!=1){
        warn(report,"Ambiguous GLB names for "+requested+"; use explicit local actor_bindings.txt");
        return {};
    }
    return result->second.front();
}
void appendMesh(StaticScene& dst,const StaticScene& src){
    if(src.triangles.size()>kMaxVerts-dst.triangles.size()||
       src.textures.size()>kMaxTextures-dst.textures.size())
        throw std::runtime_error("native stage actor scene exceeds GPU resource budget");
    const auto first=static_cast<std::uint32_t>(dst.triangles.size());
    const auto tex=static_cast<int>(dst.textures.size());
    dst.triangles.insert(dst.triangles.end(),src.triangles.begin(),src.triangles.end());
    dst.textures.insert(dst.textures.end(),src.textures.begin(),src.textures.end());
    for(const auto& batch:src.batches)
        dst.batches.push_back({first+batch.firstVertex,batch.vertexCount,
            batch.textureIndex<0?-1:tex+batch.textureIndex});
    dst.sourceNodes+=src.sourceNodes;
    dst.sourcePrimitives+=src.sourcePrimitives;
    dst.skinnedPrimitives+=src.skinnedPrimitives;
    dst.unsupportedPrimitives+=src.unsupportedPrimitives;
    dst.missingTextures+=src.missingTextures;
}
} // namespace

NativeActorWorld::NativeActorWorld(StaticScene backdrop)
    :backdrop_(std::move(backdrop)),combined_(backdrop_){}
void NativeActorWorld::bind(const NativeStageWorld& stage,
                             const std::filesystem::path& privateGlbDirectory,
                             const std::filesystem::path& optionalBindingFile,
                             const std::filesystem::path& originalPlacementFile){
    const auto canonical=fs::canonical(privateGlbDirectory);
    const auto index=indexModels(canonical);
    const auto overrides=readBindings(optionalBindingFile);
    actors_.clear();
    report_={};
    report_.stageEntities=stage.entities().size();
    // Recover original script tag_name -> exact original level-object
    // placement only when the tag is present in the local PLC file. The
    // retail Lua script examples supply spawn tags instead of inline XYZ.
    // Unsupported original collision-placement revisions may still render
    // models: a failed PLC probe must not block all native actors.
    std::map<std::string,std::vector<std::array<float,3>>> originalTags;
    const fs::path originalPlc=originalPlacementFile.empty()?
        canonical/"placeholder.plc":originalPlacementFile;
    if(fs::is_regular_file(originalPlc)){
        try{
            const auto plc=parsePlacements(originalPlc);
            for(const auto& entry:plc.entries)
                originalTags[lowered(entry.name)].push_back(entry.xyz);
        }catch(const std::exception& e){
            warn(report_,"Original PLC tag matching unavailable: "+std::string(e.what()));
        }
    }
    std::set<std::string> unique;
    for(const auto& entity:stage.entities()){
        fs::path modelPath;
        try {
            modelPath=bindModel(entity,canonical,overrides,index,report_);
            if(modelPath.empty()){
                ++report_.unboundEntities;
                continue;
            }
            if(actors_.size()>=kMaxActors) {
                ++report_.unboundEntities;
                warn(report_,"Actor count limit reached; remaining Lua entities not instantiated");
                continue;
            }
            NativeActor actor;
            actor.entityClass=entity.entityClass;
            actor.name=entity.name.empty()?entity.entityClass+
                "#"+std::to_string(actors_.size()):entity.name;
            if(!unique.insert(lowered(actor.name)).second){
                ++report_.modelsRejected;
                warn(report_,"Duplicate Lua entity name: "+actor.name);
                continue;
            }
            actor.modelName=modelPath.filename().string();
            actor.tagName=entity.tagName;
            actor.position=positionFrom(entity);
            actor.positionSource=hasExplicitPosition(entity)?
                "explicit_lua_coordinates":"unresolved_original_position";
            if(!hasExplicitPosition(entity)&&!entity.tagName.empty()){
                const auto tag=originalTags.find(lowered(entity.tagName));
                if(tag!=originalTags.end()&&tag->second.size()==1){
                    const auto& xyz=tag->second.front();
                    actor.position={xyz[0],xyz[1],-xyz[2]};
                    actor.positionSource="original_plc_exact_tag";
                    ++report_.matchedOriginalPlcTags;
                }else if(tag!=originalTags.end()){
                    warn(report_,"Ambiguous original level PLC tag "+entity.tagName+
                         "; actor stays at unresolved origin");
                }
            }
            if(actor.positionSource=="unresolved_original_position"){
                ++report_.unresolvedSpawnPositions;
                warn(report_,"Lua "+entity.entityClass+" "+actor.name+
                     " has no explicit coordinates or exact original PLC tag; "
                     "origin is NOT a verified spawn position");
            }
            actor.yaw=actorYaw(entity);
            actor.model=std::make_unique<NativeSkinnedGlb>(modelPath);
            actor.hasAnimation=actor.model->clipCount()>0 &&
                               actor.model->skinnedVertexCount()>0;
            if(actor.hasAnimation)++report_.animatedActors;
            actors_.push_back(std::move(actor));
        }catch(const std::exception& e){
            ++report_.modelsRejected;
            warn(report_,"Actor "+entity.name+": "+e.what());
        }
    }
    report_.actorsCreated=actors_.size();
    rebuildLayout();
}
void NativeActorWorld::rebuildLayout(){
    combined_=backdrop_;
    if(combined_.triangles.size()>kMaxVerts ||
       combined_.textures.size()>kMaxTextures)
        throw std::runtime_error("original level backdrop exceeds native actor resource budget");
    for(auto& actor:actors_){
        const auto& source=actor.model->restScene();
        actor.firstVertex=combined_.triangles.size();
        actor.vertexCount=source.triangles.size();
        appendMesh(combined_,source);
        updateGeometry(actor);
    }
}
void NativeActorWorld::updateGeometry(NativeActor& actor){
    if(!actor.model)return;
    const auto& source=actor.hasAnimation?
        actor.model->sample(actor.activeClip,actor.elapsed):actor.model->restScene();
    if(source.triangles.size()!=actor.vertexCount ||
       actor.firstVertex>combined_.triangles.size() ||
       actor.vertexCount>combined_.triangles.size()-actor.firstVertex)
        throw std::runtime_error("actor skeleton changed original model topology at runtime");
    const float c=std::cos(actor.yaw),s=std::sin(actor.yaw);
    for(std::size_t i=0;i<actor.vertexCount;++i){
        auto vertex=source.triangles[i];
        const float x=vertex.x,z=vertex.z;
        vertex.x=c*x+s*z+actor.position[0];
        vertex.y+=actor.position[1];
        vertex.z=-s*x+c*z+actor.position[2];
        combined_.triangles[actor.firstVertex+i]=vertex;
    }
}
void NativeActorWorld::tick(double seconds){
    if(!std::isfinite(seconds)||seconds<0||seconds>1)
        throw std::runtime_error("native actor tick must use finite frame delta");
    for(auto& actor:actors_){
        if(actor.hasAnimation){
            actor.elapsed+=seconds;
            updateGeometry(actor);
        }
    }
}
bool NativeActorWorld::moveActor(const std::string& name,float x,float y,float z,float facing){
    if(!validPosition(x)||!validPosition(y)||!validPosition(z)||
       !std::isfinite(facing))return false;
    for(auto& actor:actors_)if(lowered(actor.name)==lowered(name)){
        actor.position={x,y,z};
        actor.yaw=facing;
        updateGeometry(actor);
        return true;
    }
    return false;
}
bool NativeActorWorld::setActorClip(const std::string& name,std::size_t clip){
    for(auto& actor:actors_)if(lowered(actor.name)==lowered(name)){
        if(!actor.hasAnimation||clip>=actor.model->clipCount())return false;
        actor.activeClip=clip;
        actor.elapsed=0;
        updateGeometry(actor);
        return true;
    }
    return false;
}
const NativeActor* NativeActorWorld::actor(const std::string& name) const{
    for(const auto& actor:actors_)
        if(lowered(actor.name)==lowered(name))return &actor;
    return nullptr;
}
} // namespace ae3
