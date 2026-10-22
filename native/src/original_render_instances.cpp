#include "ae3/original_render_instances.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace ae3 {
namespace {
constexpr std::size_t maxObjects=4096,maxVertices=3'000'000,maxTextures=512;
bool finiteMatrix(const HostRenderMatrix& m){
    return std::all_of(m.begin(),m.end(),[](float x){
        return std::isfinite(x)&&std::abs(x)<1e8f;
    }) && m[3]==0.f&&m[7]==0.f&&m[11]==0.f&&m[15]==1.f;
}
void updateBounds(StaticScene& scene){
    if(scene.triangles.empty())return;
    auto& b=scene.bounds;
    const auto& first=scene.triangles.front();
    b.min={first.x,first.y,first.z};
    b.max=b.min;
    for(const auto& v:scene.triangles){
        const float p[]={v.x,v.y,v.z};
        for(unsigned k=0;k<3;++k){
            b.min[k]=std::min(b.min[k],p[k]);
            b.max[k]=std::max(b.max[k],p[k]);
        }
    }
    double square=0;
    for(unsigned k=0;k<3;++k){
        b.center[k]=b.min[k]*.5f+b.max[k]*.5f;
        const double d=double(b.max[k])-double(b.min[k]);
        square+=d*d;
    }
    b.radius=static_cast<float>(std::sqrt(square)*.5);
}
void transformVertex(SceneVertex& v,const HostRenderMatrix& m){
    const float x=v.x,y=v.y,z=v.z;
    v.x=m[0]*x+m[4]*y+m[8]*z+m[12];
    v.y=m[1]*x+m[5]*y+m[9]*z+m[13];
    v.z=m[2]*x+m[6]*y+m[10]*z+m[14];
    if(!std::isfinite(v.x)||!std::isfinite(v.y)||!std::isfinite(v.z)||
       std::abs(v.x)>1e8f||std::abs(v.y)>1e8f||std::abs(v.z)>1e8f)
        throw std::runtime_error("render instance transformed outside supported world coordinates");
}
void validateSource(const StaticScene& src){
    if(src.triangles.size()>maxVertices||src.triangles.size()%3)
        throw std::runtime_error("original render source has invalid triangle count");
    for(const auto& b:src.batches)
        if(b.firstVertex>src.triangles.size()||
           b.vertexCount>src.triangles.size()-b.firstVertex||
           b.vertexCount%3||b.textureIndex< -1||
           (b.textureIndex>=0&&static_cast<std::size_t>(b.textureIndex)>=src.textures.size()))
            throw std::runtime_error("original render material batch invalid");
}
} // namespace

HostRenderMatrix identityRenderMatrix() noexcept{
    return {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
}
HostRenderMatrix gamePlacementMatrix(const std::array<float,3>& xyz){
    for(float p:xyz)if(!std::isfinite(p)||std::abs(p)>1e7f)
        throw std::runtime_error("nonfinite or out-of-range original placement");
    auto m=identityRenderMatrix();
    m[12]=xyz[0];m[13]=xyz[1];m[14]=-xyz[2];
    return m;
}
OriginalRenderObject* OriginalRenderWorld::find(OriginalRenderId id) noexcept{
    for(auto& entry:entries_)if(entry.object.id==id)return &entry.object;
    return nullptr;
}
const OriginalRenderObject* OriginalRenderWorld::find(OriginalRenderId id) const noexcept{
    for(const auto& entry:entries_)if(entry.object.id==id)return &entry.object;
    return nullptr;
}
void OriginalRenderWorld::addStatic(OriginalRenderObject object,std::shared_ptr<const StaticScene> source){
    if(!source)throw std::invalid_argument("original static model source is null");
    if(!object.id||find(object.id)||entries_.size()>=maxObjects||!finiteMatrix(object.world))
        throw std::invalid_argument("duplicate/invalid static render object or transform");
    validateSource(*source);
    entries_.push_back({std::move(object),std::move(source),{}});
}
void OriginalRenderWorld::addAnimated(OriginalRenderObject object,OriginalSceneSampler sampler){
    if(!sampler||!object.id||find(object.id)||entries_.size()>=maxObjects||!finiteMatrix(object.world))
        throw std::invalid_argument("duplicate/invalid animated render object or transform");
    entries_.push_back({std::move(object),{},std::move(sampler)});
}
bool OriginalRenderWorld::remove(OriginalRenderId id){
    const auto it=std::find_if(entries_.begin(),entries_.end(),[id](const Entry& e){return e.object.id==id;});
    if(it==entries_.end())return false;
    entries_.erase(it);
    return true;
}
OriginalRenderFrame OriginalRenderWorld::composeFrame() const{
    std::vector<const Entry*> order;
    order.reserve(entries_.size());
    for(const auto& e:entries_)if(e.object.visible)order.push_back(&e);
    // No guessed occlusion/LOD. The gameplay engine owns flags and layers.
    std::stable_sort(order.begin(),order.end(),[](const Entry* a,const Entry* b){
        return a->object.renderLayer<b->object.renderLayer;
    });
    return composeOrdered(order);
}
OriginalRenderFrame OriginalRenderWorld::composeFrameInOriginalOrder(
    const std::vector<OriginalRenderId>& ids) const {
    if(ids.size()>entries_.size())
        throw std::invalid_argument("original render list has more actor IDs than owner instances");
    std::unordered_map<OriginalRenderId,const Entry*> indexed;
    indexed.reserve(entries_.size());
    for(const auto& e:entries_)indexed.emplace(e.object.id,&e);
    std::vector<const Entry*> order;
    order.reserve(ids.size());
    for(const OriginalRenderId id:ids){
        const auto it=indexed.find(id);
        if(it==indexed.end())throw std::out_of_range("retail render list references missing actor");
        const Entry* actor=it->second;
        if(std::find(order.begin(),order.end(),actor)!=order.end())
            throw std::invalid_argument("duplicate original actor in render list");
        if(actor->object.visible)order.push_back(actor);
    }
    return composeOrdered(order);
}
OriginalRenderFrame OriginalRenderWorld::composeOrdered(
    const std::vector<const Entry*>& order) const{
    OriginalRenderFrame out;
    struct TextureKey {
        const StaticScene* scene;
        int index;
        bool operator==(const TextureKey& rhs)const noexcept{
            return scene==rhs.scene&&index==rhs.index;
        }
    };
    struct Hash {
        std::size_t operator()(TextureKey k)const noexcept{
            return (std::hash<const void*>{}(k.scene)>>2)^std::hash<int>{}(k.index);
        }
    };
    std::unordered_map<TextureKey,int,Hash> textureMap;
    for(const Entry* entry:order){
        const auto& object=entry->object;
        if(!finiteMatrix(object.world)||!std::isfinite(object.animationSeconds))
            throw std::runtime_error("render object transform/timeline invalid");
        const StaticScene& source=entry->sampler?
            entry->sampler(object.clip,object.animationSeconds):*entry->staticScene;
        validateSource(source);
        if(source.triangles.size()>maxVertices-out.scene.triangles.size())
            throw std::runtime_error("original scene exceeds native D3D vertex budget");
        const auto first=static_cast<std::uint32_t>(out.scene.triangles.size());
        for(SceneVertex v:source.triangles){
            transformVertex(v,object.world);
            out.scene.triangles.push_back(v);
        }
        for(const auto& batch:source.batches){
            int texture=-1;
            if(batch.textureIndex>=0){
                TextureKey key{&source,batch.textureIndex};
                const auto it=textureMap.find(key);
                if(it==textureMap.end()){
                    if(out.scene.textures.size()>=maxTextures)
                        throw std::runtime_error("original frame exceeds D3D texture budget");
                    texture=static_cast<int>(out.scene.textures.size());
                    out.scene.textures.push_back(source.textures[static_cast<std::size_t>(batch.textureIndex)]);
                    textureMap.emplace(key,texture);
                }else texture=it->second;
            }
            const auto start=first+batch.firstVertex;
            out.scene.batches.push_back({start,batch.vertexCount,texture});
            out.submissions.push_back({object.id,object.sourceName,start,batch.vertexCount,texture,object.renderLayer});
        }
        out.scene.sourceNodes+=source.sourceNodes;
        out.scene.sourcePrimitives+=source.sourcePrimitives;
        out.scene.skinnedPrimitives+=source.skinnedPrimitives;
        out.scene.unsupportedPrimitives+=source.unsupportedPrimitives;
        out.scene.missingTextures+=source.missingTextures;
        ++out.visibleObjects;
        if(entry->sampler)++out.animatedObjects;
    }
    updateBounds(out.scene);
    return out;
}
} // namespace ae3
