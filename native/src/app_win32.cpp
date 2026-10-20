#include "ae3/archive.hpp"
#include "ae3/tim2.hpp"
#include "ae3/lua_stage.hpp"
#include "ae3/controller_timeline.hpp"
#include "ae3/gltf_scene.hpp"
#include "ae3/level_scene.hpp"
#include "ae3/native_walking.hpp"
#include "ae3/i3c_collision.hpp"
#include "ae3/native_skinning.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include <objbase.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cwctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
namespace fs = std::filesystem;

namespace {
constexpr int kList=100, kOpen=101, kExport=102, kStatus=103, kLuaStage=104, kSignalTrigger=105, kOpenGlb=106, kOpenLevel=107, kOriginalCollision=108;
HWND gMain=nullptr,gList=nullptr,gView=nullptr,gInfo=nullptr;
std::unique_ptr<ae3::Archive> gArchive;
std::unique_ptr<ae3::NativeStageWorld> gNativeWorld;
std::unique_ptr<ae3::LuaStage> gNativeStage;
std::vector<ae3::NativeControllerTimeline> gControllerTimelines;
std::chrono::steady_clock::time_point gLastScriptTick=std::chrono::steady_clock::now();
std::size_t gScriptEventCount=0;
std::wstring gSource, gError, gProgress;
float gYaw=.4f,gX=0,gZ=-5,gHeight=2.5f;
bool gKeys[256]={};
ae3::StaticScene gWalkCollisionScene;
bool gOriginalI3cActive=false;
ae3::NativeWalkingController gNativeWalker;
bool gNativeWalkMode=false;
bool gF5HeldLastFrame=false;
bool gF7HeldLastFrame=false;
bool gF8HeldLastFrame=false;
bool gF9HeldLastFrame=false;
std::unique_ptr<ae3::NativeSkinnedGlb> gAnimatedModel;
std::size_t gActiveClip=0;
double gAnimationSeconds=0;
bool gAnimationPaused=false;
struct Vertex { float x,y,z; float r,g,b,a; float u=0,v=0; };
static_assert(sizeof(Vertex)==sizeof(ae3::SceneVertex));
class NativeRenderer {
public:
    bool init(HWND hwnd) {
        hwnd_=hwnd;
        RECT rc{};GetClientRect(hwnd_,&rc);
        DXGI_SWAP_CHAIN_DESC desc{};
        desc.BufferCount=2;
        desc.BufferDesc.Width=std::max(1L,rc.right);
        desc.BufferDesc.Height=std::max(1L,rc.bottom);
        desc.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow=hwnd_;
        desc.SampleDesc.Count=1;
        desc.Windowed=TRUE;
        desc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
        D3D_FEATURE_LEVEL feature{};
        HRESULT hr=D3D11CreateDeviceAndSwapChain(
            nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,0,nullptr,0,
            D3D11_SDK_VERSION,&desc,&swap_,&device_,&feature,&context_);
        // Hardware GPU through Direct3D only; no fallback to console simulation.
        if(FAILED(hr))return false;
        const char* hlsl=R"(
            cbuffer Scene:register(b0){row_major float4x4 mvp;};
            Texture2D Diffuse:register(t0);
            SamplerState LinearSampler:register(s0);
            struct Vin {float3 pos:POSITION; float4 color:COLOR; float2 uv:TEXCOORD0;};
            struct Vout {float4 pos:SV_POSITION; float4 color:COLOR; float2 uv:TEXCOORD0;};
            Vout VSMain(Vin i){
                Vout o;o.pos=mul(float4(i.pos,1),mvp);
                o.color=i.color;o.uv=i.uv;return o;
            }
            float4 PSMain(Vout i):SV_TARGET{
                return i.color*Diffuse.Sample(LinearSampler,i.uv);
            }
        )";
        ComPtr<ID3DBlob> vs,ps,errors;
        if(FAILED(D3DCompile(hlsl,strlen(hlsl),nullptr,nullptr,nullptr,
            "VSMain","vs_4_0",0,0,&vs,&errors)))return false;
        if(FAILED(D3DCompile(hlsl,strlen(hlsl),nullptr,nullptr,nullptr,
            "PSMain","ps_4_0",0,0,&ps,&errors)))return false;
        if(FAILED(device_->CreateVertexShader(vs->GetBufferPointer(),vs->GetBufferSize(),
            nullptr,&vs_)))return false;
        if(FAILED(device_->CreatePixelShader(ps->GetBufferPointer(),ps->GetBufferSize(),
            nullptr,&ps_)))return false;
        // Original game TIM2 pixels are converted into a regular D3D11 texture.
        // Preview runs a native shader; no PlayStation graphics commands exist.
        const char* previewShader=R"(
            Texture2D Texture:register(t0);
            SamplerState Sampler:register(s0);
            struct PreviewOut {float4 position:SV_POSITION;float2 uv:TEXCOORD0;};
            PreviewOut PreviewVS(uint id:SV_VertexID){
                float2 uv=float2((id==2)?2:0,(id==1)?2:0);
                PreviewOut o;
                o.position=float4(uv*float2(2,-2)+float2(-1,1),0,1);
                o.uv=uv;
                return o;
            }
            float4 PreviewPS(PreviewOut i):SV_TARGET{
                return Texture.Sample(Sampler,i.uv);
            }
        )";
        ComPtr<ID3DBlob> pvs,pps;
        if(FAILED(D3DCompile(previewShader,strlen(previewShader),nullptr,nullptr,nullptr,
            "PreviewVS","vs_4_0",0,0,&pvs,&errors)))return false;
        if(FAILED(D3DCompile(previewShader,strlen(previewShader),nullptr,nullptr,nullptr,
            "PreviewPS","ps_4_0",0,0,&pps,&errors)))return false;
        if(FAILED(device_->CreateVertexShader(pvs->GetBufferPointer(),pvs->GetBufferSize(),
            nullptr,&previewVS_)))return false;
        if(FAILED(device_->CreatePixelShader(pps->GetBufferPointer(),pps->GetBufferSize(),
            nullptr,&previewPS_)))return false;
        D3D11_SAMPLER_DESC sampler{};
        sampler.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU=sampler.AddressV=sampler.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler.MaxLOD=D3D11_FLOAT32_MAX;
        if(FAILED(device_->CreateSamplerState(&sampler,&sampler_)))return false;
        const unsigned char opaqueWhite[4]={255,255,255,255};
        if(!uploadRGBA(1,1,opaqueWhite,whiteTexture_))return false;
        const D3D11_INPUT_ELEMENT_DESC input[] = {
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,28,D3D11_INPUT_PER_VERTEX_DATA,0}
        };
        if(FAILED(device_->CreateInputLayout(input,3,vs->GetBufferPointer(),
            vs->GetBufferSize(),&layout_)))return false;
        D3D11_BUFFER_DESC cb{};cb.ByteWidth=sizeof(XMFLOAT4X4);
        cb.Usage=D3D11_USAGE_DYNAMIC;cb.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
        cb.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
        if(FAILED(device_->CreateBuffer(&cb,nullptr,&constants_)))return false;
        // Explicitly a host-side debug grid. This is NOT Ape Escape 3 geometry.
        std::vector<Vertex> grid;
        for(int v=-20;v<=20;++v){
            float c=v==0?.48f:.21f;
            grid.push_back({float(v),0,-20,c,c,c,1});
            grid.push_back({float(v),0,20,c,c,c,1});
            grid.push_back({-20,0,float(v),c,c,c,1});
            grid.push_back({20,0,float(v),c,c,c,1});
        }
        gridCount_=static_cast<UINT>(grid.size());
        D3D11_BUFFER_DESC vb{};
        vb.ByteWidth=static_cast<UINT>(sizeof(Vertex)*grid.size());
        vb.Usage=D3D11_USAGE_IMMUTABLE;vb.BindFlags=D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA src{};src.pSysMem=grid.data();
        if(FAILED(device_->CreateBuffer(&vb,&src,&vertices_)))return false;
        D3D11_RASTERIZER_DESC both{};
        both.FillMode=D3D11_FILL_SOLID;
        both.CullMode=D3D11_CULL_NONE;
        both.DepthClipEnable=TRUE;
        if(FAILED(device_->CreateRasterizerState(&both,&bothSides_)))return false;
        return resize(std::max(1L,rc.right),std::max(1L,rc.bottom));
    }
    bool resize(int width,int height){
        if(!swap_||width<=0||height<=0)return false;
        context_->OMSetRenderTargets(0,nullptr,nullptr);
        target_.Reset();
        depthView_.Reset();
        depthBuffer_.Reset();
        if(FAILED(swap_->ResizeBuffers(0,width,height,DXGI_FORMAT_UNKNOWN,0)))return false;
        ComPtr<ID3D11Texture2D> back;
        if(FAILED(swap_->GetBuffer(0,IID_PPV_ARGS(&back))))return false;
        if(FAILED(device_->CreateRenderTargetView(back.Get(),nullptr,&target_)))return false;
        D3D11_TEXTURE2D_DESC depth{};
        depth.Width=static_cast<UINT>(width);
        depth.Height=static_cast<UINT>(height);
        depth.MipLevels=1;depth.ArraySize=1;
        depth.SampleDesc.Count=1;
        depth.Format=DXGI_FORMAT_D24_UNORM_S8_UINT;
        depth.BindFlags=D3D11_BIND_DEPTH_STENCIL;
        if(FAILED(device_->CreateTexture2D(&depth,nullptr,&depthBuffer_)))return false;
        if(FAILED(device_->CreateDepthStencilView(depthBuffer_.Get(),nullptr,&depthView_)))return false;
        width_=width;height_=height;return true;
    }
    void clearScene(){
        sceneVertices_.Reset();sceneCount_=0;
        sceneBatches_.clear();sceneTextures_.clear();sceneTextureFailures_=0;
        animatedBuffer_=false;
        clearCollisionOverlay();
    }
    void clearCollisionOverlay(){
        collisionWire_.Reset();collisionLineCount_=0;showCollision_=true;
    }
    bool toggleCollisionOverlay(){
        showCollision_=!showCollision_;
        return showCollision_;
    }
    bool setCollisionOverlay(const ae3::StaticScene& collision){
        constexpr std::size_t maxSampleTris=80000;
        const auto count=collision.triangles.size()/3;
        if(!device_||count==0)return false;
        const std::size_t stride=std::max<std::size_t>(1,count/maxSampleTris);
        std::vector<Vertex> lines;
        lines.reserve(std::min(count,maxSampleTris+1)*6);
        const auto wire=[](const ae3::SceneVertex& v)->Vertex{
            return {v.x,v.y,v.z,0.18f,0.95f,0.35f,1.f,0,0};
        };
        for(std::size_t tri=0;tri<count;tri+=stride){
            const auto& a=collision.triangles[tri*3];
            const auto& b=collision.triangles[tri*3+1];
            const auto& c=collision.triangles[tri*3+2];
            lines.push_back(wire(a));lines.push_back(wire(b));
            lines.push_back(wire(b));lines.push_back(wire(c));
            lines.push_back(wire(c));lines.push_back(wire(a));
        }
        if(lines.empty()||
           lines.size()>std::numeric_limits<UINT>::max()/sizeof(Vertex))return false;
        D3D11_BUFFER_DESC vb{};
        vb.ByteWidth=static_cast<UINT>(lines.size()*sizeof(Vertex));
        vb.Usage=D3D11_USAGE_IMMUTABLE;
        vb.BindFlags=D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA data{};data.pSysMem=lines.data();
        ComPtr<ID3D11Buffer> replacement;
        if(FAILED(device_->CreateBuffer(&vb,&data,&replacement)))return false;
        collisionWire_=std::move(replacement);
        collisionLineCount_=static_cast<UINT>(lines.size());
        showCollision_=true;
        return true;
    }
    unsigned textureFailures() const noexcept{return sceneTextureFailures_;}
    bool uploadRGBA(UINT width,UINT height,const unsigned char* pixels,
                    ComPtr<ID3D11ShaderResourceView>& destination) {
        if(!device_ || !width || !height || width>4096 || height>4096)return false;
        D3D11_TEXTURE2D_DESC tex{};
        tex.Width=width;tex.Height=height;tex.MipLevels=1;tex.ArraySize=1;
        tex.Format=DXGI_FORMAT_R8G8B8A8_UNORM;tex.SampleDesc.Count=1;
        tex.Usage=D3D11_USAGE_IMMUTABLE;tex.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem=pixels;data.SysMemPitch=width*4;
        ComPtr<ID3D11Texture2D> texture;
        if(FAILED(device_->CreateTexture2D(&tex,&data,&texture)))return false;
        return SUCCEEDED(device_->CreateShaderResourceView(
            texture.Get(),nullptr,&destination));
    }
    bool loadImageWIC(const std::vector<std::uint8_t>& encoded,
                      ComPtr<ID3D11ShaderResourceView>& destination) {
        if(encoded.empty() || encoded.size()>32*1024*1024)return false;
        ComPtr<IWICImagingFactory> factory;
        if(FAILED(CoCreateInstance(CLSID_WICImagingFactory,nullptr,
            CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&factory))))return false;
        ComPtr<IWICStream> stream;
        if(FAILED(factory->CreateStream(&stream)))return false;
        if(FAILED(stream->InitializeFromMemory(
            const_cast<BYTE*>(encoded.data()),static_cast<DWORD>(encoded.size()))))
            return false;
        ComPtr<IWICBitmapDecoder> decoder;
        if(FAILED(factory->CreateDecoderFromStream(
            stream.Get(),nullptr,WICDecodeMetadataCacheOnLoad,&decoder)))return false;
        ComPtr<IWICBitmapFrameDecode> frame;
        if(FAILED(decoder->GetFrame(0,&frame)))return false;
        UINT width=0,height=0;
        if(FAILED(frame->GetSize(&width,&height))||!width||!height||
           width>4096||height>4096||std::uint64_t(width)*height>16*1024*1024)
            return false;
        ComPtr<IWICFormatConverter> converter;
        if(FAILED(factory->CreateFormatConverter(&converter)))return false;
        if(FAILED(converter->Initialize(frame.Get(),GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone,nullptr,0,WICBitmapPaletteTypeCustom)))return false;
        std::vector<std::uint8_t> pixels(std::size_t(width)*height*4);
        if(FAILED(converter->CopyPixels(nullptr,width*4,
            static_cast<UINT>(pixels.size()),pixels.data())))return false;
        return uploadRGBA(width,height,pixels.data(),destination);
    }
    bool updateAnimation(const ae3::StaticScene& frame){
        if(!sceneVertices_ || !animatedBuffer_ ||
           frame.triangles.size()!=sceneCount_)return false;
        D3D11_MAPPED_SUBRESOURCE map{};
        if(FAILED(context_->Map(sceneVertices_.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&map)))
            return false;
        std::memcpy(map.pData,frame.triangles.data(),
                    frame.triangles.size()*sizeof(Vertex));
        context_->Unmap(sceneVertices_.Get(),0);
        return true;
    }
    bool displayScene(const ae3::StaticScene& scene,bool animated=false){
        if(!device_ || scene.triangles.empty() ||
           scene.triangles.size()>3'000'000 ||
           scene.triangles.size()>std::numeric_limits<UINT>::max()/sizeof(Vertex))
            return false;
        clearTexture();
        clearScene();
        D3D11_BUFFER_DESC vb{};
        vb.ByteWidth=static_cast<UINT>(scene.triangles.size()*sizeof(Vertex));
        vb.Usage=animated?D3D11_USAGE_DYNAMIC:D3D11_USAGE_IMMUTABLE;
        vb.BindFlags=D3D11_BIND_VERTEX_BUFFER;
        vb.CPUAccessFlags=animated?D3D11_CPU_ACCESS_WRITE:0;
        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem=scene.triangles.data();
        if(FAILED(device_->CreateBuffer(&vb,&data,&sceneVertices_)))return false;
        sceneCount_=static_cast<UINT>(scene.triangles.size());
        animatedBuffer_=animated;
        sceneBatches_=scene.batches;
        sceneTextures_.resize(scene.textures.size());
        for(std::size_t t=0;t<scene.textures.size();++t) {
            if(!loadImageWIC(scene.textures[t].encoded,sceneTextures_[t]))
                ++sceneTextureFailures_;
        }
        return true;
    }
    void clearTexture(){previewSRV_.Reset();}
    bool displayTexture(const ae3::Texture& image){
        if(!device_ || !image.width || !image.height ||
            image.rgba.size()!=std::size_t(image.width)*image.height*4)return false;
        clearTexture();
        D3D11_TEXTURE2D_DESC tex{};
        tex.Width=image.width;tex.Height=image.height;
        tex.MipLevels=1;tex.ArraySize=1;
        tex.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        tex.SampleDesc.Count=1;tex.Usage=D3D11_USAGE_IMMUTABLE;
        tex.BindFlags=D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA data{};
        data.pSysMem=image.rgba.data();data.SysMemPitch=image.width*4;
        ComPtr<ID3D11Texture2D> texture;
        if(FAILED(device_->CreateTexture2D(&tex,&data,&texture)))return false;
        return SUCCEEDED(device_->CreateShaderResourceView(texture.Get(),nullptr,&previewSRV_));
    }
    void render(){
        if(!target_)return;
        static constexpr float sky[4]={.055f,.09f,.14f,1};
        context_->ClearRenderTargetView(target_.Get(),sky);
        D3D11_VIEWPORT vp{};vp.Width=float(width_);vp.Height=float(height_);
        vp.MinDepth=0;vp.MaxDepth=1;
        context_->RSSetViewports(1,&vp);
        context_->OMSetRenderTargets(1,target_.GetAddressOf(),nullptr);
        if(previewSRV_) {
            // Draw a native fullscreen texture via SV_VertexID (no vertex buffer).
            context_->IASetInputLayout(nullptr);
            context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context_->VSSetShader(previewVS_.Get(),nullptr,0);
            context_->PSSetShader(previewPS_.Get(),nullptr,0);
            ID3D11ShaderResourceView* image=previewSRV_.Get();
            ID3D11SamplerState* sampler=sampler_.Get();
            context_->PSSetShaderResources(0,1,&image);
            context_->PSSetSamplers(0,1,&sampler);
            context_->Draw(3,0);
            ID3D11ShaderResourceView* empty=nullptr;
            context_->PSSetShaderResources(0,1,&empty);
            swap_->Present(1,0);
            return;
        }
        if(sceneCount_ && depthView_){
            context_->ClearDepthStencilView(depthView_.Get(),D3D11_CLEAR_DEPTH,1.0f,0);
            context_->OMSetRenderTargets(1,target_.GetAddressOf(),depthView_.Get());
            context_->RSSetState(bothSides_.Get());
        }else{
            context_->RSSetState(nullptr);
        }
        XMVECTOR pos=XMVectorSet(gX,gHeight,gZ,1);
        XMVECTOR target=XMVectorSet(gX+std::sin(gYaw),gHeight-.3f,
                                    gZ+std::cos(gYaw),1);
        auto view=XMMatrixLookAtLH(pos,target,XMVectorSet(0,1,0,0));
        auto proj=XMMatrixPerspectiveFovLH(XMConvertToRadians(65),
                              float(width_)/float(height_),.05f,100);
        XMFLOAT4X4 mvp{};
        XMStoreFloat4x4(&mvp,view*proj);
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if(SUCCEEDED(context_->Map(constants_.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&mapped))){
            memcpy(mapped.pData,&mvp,sizeof(mvp));context_->Unmap(constants_.Get(),0);
        }
        UINT stride=sizeof(Vertex),offset=0;
        ID3D11Buffer* cb=constants_.Get();
        context_->VSSetConstantBuffers(0,1,&cb);
        context_->IASetInputLayout(layout_.Get());
        ID3D11Buffer* vb=sceneCount_?sceneVertices_.Get():vertices_.Get();
        context_->IASetVertexBuffers(0,1,&vb,&stride,&offset);
        context_->IASetPrimitiveTopology(sceneCount_?
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
            D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        context_->VSSetShader(vs_.Get(),nullptr,0);
        context_->PSSetShader(ps_.Get(),nullptr,0);
        ID3D11SamplerState* hostSampler=sampler_.Get();
        context_->PSSetSamplers(0,1,&hostSampler);
        if(sceneCount_){
            for(const auto& batch:sceneBatches_){
                ID3D11ShaderResourceView* texture=whiteTexture_.Get();
                if(batch.textureIndex>=0 &&
                    static_cast<std::size_t>(batch.textureIndex)<sceneTextures_.size() &&
                    sceneTextures_[std::size_t(batch.textureIndex)])
                    texture=sceneTextures_[std::size_t(batch.textureIndex)].Get();
                context_->PSSetShaderResources(0,1,&texture);
                context_->Draw(batch.vertexCount,batch.firstVertex);
            }
        }else{
            ID3D11ShaderResourceView* texture=whiteTexture_.Get();
            context_->PSSetShaderResources(0,1,&texture);
            context_->Draw(gridCount_,0);
        }
        if(collisionWire_ && collisionLineCount_ && showCollision_) {
            // Overlay collision in a separate native pass. Depth is disabled
            // on purpose so original I3C data can be compared through art.
            context_->OMSetRenderTargets(1,target_.GetAddressOf(),nullptr);
            UINT lineStride=sizeof(Vertex),offset=0;
            ID3D11Buffer* vb=collisionWire_.Get();
            context_->IASetVertexBuffers(0,1,&vb,&lineStride,&offset);
            context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
            ID3D11ShaderResourceView* white=whiteTexture_.Get();
            context_->PSSetShaderResources(0,1,&white);
            context_->Draw(collisionLineCount_,0);
        }
        swap_->Present(1,0);
    }
private:
    HWND hwnd_{};
    int width_=1,height_=1;
    UINT gridCount_=0,sceneCount_=0;
    bool animatedBuffer_=false;
    ComPtr<ID3D11Buffer> sceneVertices_;
    ComPtr<ID3D11Buffer> collisionWire_;
    UINT collisionLineCount_=0;
    bool showCollision_=true;
    std::vector<ae3::SceneBatch> sceneBatches_;
    std::vector<ComPtr<ID3D11ShaderResourceView>> sceneTextures_;
    ComPtr<ID3D11ShaderResourceView> whiteTexture_;
    unsigned sceneTextureFailures_=0;
    ComPtr<ID3D11Texture2D> depthBuffer_;
    ComPtr<ID3D11DepthStencilView> depthView_;
    ComPtr<ID3D11RasterizerState> bothSides_;
    ComPtr<IDXGISwapChain> swap_;
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<ID3D11RenderTargetView> target_;
    ComPtr<ID3D11Buffer> vertices_,constants_;
    ComPtr<ID3D11VertexShader> vs_;
    ComPtr<ID3D11PixelShader> ps_;
    ComPtr<ID3D11VertexShader> previewVS_;
    ComPtr<ID3D11PixelShader> previewPS_;
    ComPtr<ID3D11ShaderResourceView> previewSRV_;
    ComPtr<ID3D11SamplerState> sampler_;
    ComPtr<ID3D11InputLayout> layout_;
} gRenderer;
std::wstring wide(const std::string& s) {
    if(s.empty())return L"";
    int n=MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,nullptr,0);
    if(!n)return L"<invalid asset name>";
    std::wstring out(n,0);
    MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,out.data(),n);
    out.pop_back();return out;
}
void status(const std::wstring& message){gProgress=message;if(gInfo)SetWindowTextW(gInfo,message.c_str());}
void loadArchive(const fs::path& path){
    try{
        auto mounted=std::make_unique<ae3::Archive>(path);
        SendMessageW(gList,LB_RESETCONTENT,0,0);
        for(const auto& entry:mounted->entries()){
            auto name=wide(entry.path);
            SendMessageW(gList,LB_ADDSTRING,0,reinterpret_cast<LPARAM>(name.c_str()));
        }
        gSource=path.wstring();
        gControllerTimelines.clear();
        gNativeStage.reset();
        gNativeWorld.reset();
        gRenderer.clearTexture();
        gRenderer.clearScene();
        gAnimatedModel.reset();
        gWalkCollisionScene={};
        gOriginalI3cActive=false;
        gNativeWalkMode=false;
        gArchive=std::move(mounted);
        std::wstringstream info;info<<L"Native mount: "<<gArchive->entries().size()
            <<L" real AE3 assets | "<<wide(gArchive->sourceKind())
            <<L". No PS2 hardware runtime. WASD/QE + arrows move debug camera.";
        status(info.str());
        if(!gArchive->entries().empty())SendMessageW(gList,LB_SETCURSEL,0,0);
    }catch(const std::exception& e){gArchive.reset();status(L"Game data mount failed: "+wide(e.what()));}
}
void openFile(){
    wchar_t name[32768]={};
    OPENFILENAMEW dialog{};dialog.lStructSize=sizeof(dialog);
    dialog.hwndOwner=gMain;dialog.lpstrFile=name;dialog.nMaxFile=32768;
    dialog.lpstrFilter=L"Ape Escape 3 files (*.iso;*.bin)\0*.iso;*.bin\0All files (*.*)\0*.*\0";
    dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_EXPLORER;
    dialog.lpstrTitle=L"Choose YOUR Ape Escape 3 USA ISO or extracted DATA.BIN";
    if(GetOpenFileNameW(&dialog))loadArchive(name);
}
void loadNativeGlb(const fs::path& path){
    try{
        auto motion=std::make_unique<ae3::NativeSkinnedGlb>(path);
        const auto& scene=motion->restScene();
        const bool animated=motion->clipCount()>0 && motion->skinnedVertexCount()>0;
        if(!gRenderer.displayScene(scene,animated))
            throw std::runtime_error("Direct3D11 vertex buffer upload failed");
        gAnimatedModel=animated?std::move(motion):nullptr;
        gAnimationSeconds=0;
        gActiveClip=0;
        gAnimationPaused=false;
        gWalkCollisionScene=scene;
        gOriginalI3cActive=false;
        gNativeWalkMode=false;
        gX=scene.bounds.center[0];
        gHeight=scene.bounds.center[1]+scene.bounds.radius*.15f;
        gZ=scene.bounds.center[2]-std::max(2.f,scene.bounds.radius*2.8f);
        gYaw=0.f;
        std::wstringstream msg;
        msg<<L"Native GLB: "<<scene.sourceNodes<<L" scene nodes, "
           <<scene.sourcePrimitives<<L" mesh primitives, "
           <<scene.triangles.size()/3<<L" triangles; "
           <<scene.skinnedPrimitives<<L" skinned rest-pose primitives. "
           <<L"WASD/QE + arrows to move debug camera. "
           <<L"Texture files loaded: "<<scene.textures.size()<<L", "
           <<L"missing: "<<scene.missingTextures<<L", "
           <<L"decode failures: "<<gRenderer.textureFailures()<<L". "
           <<L"F5 toggles native walk/jump testing on mesh geometry. "
           <<(animated?L" ORIGINAL I3M-GLB skeletal playback active. F8 clip, F9 pause. ":
                       L" No valid skeleton animation clip in this GLB. ")
           <<L"Original entity AI/capture not yet implemented.";
        status(msg.str());
        gRenderer.render();
    }catch(const std::exception& error){
        status(L"Native GLB model load failed: "+wide(error.what()));
    }
}
void openNativeGlb(){
    wchar_t name[32768]={};
    OPENFILENAMEW dialog{};
    dialog.lStructSize=sizeof(dialog);
    dialog.hwndOwner=gMain;dialog.lpstrFile=name;dialog.nMaxFile=32768;
    dialog.lpstrFilter=L"Native I3D/GLB models (*.glb)\0*.glb\0All files (*.*)\0*.*\0";
    dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_EXPLORER;
    dialog.lpstrTitle=L"Open native GLB exported from original AE3 I3D model";
    if(GetOpenFileNameW(&dialog))loadNativeGlb(fs::path(name));
}
void loadNativeLevel(const fs::path& folder) {
    try {
        // The owner may supply the matching original placeholder.plc in
        // the level export folder. If none is available, clearly report
        // unpositioned models rather than pretending an authentic world.
        fs::path plc=folder/"placeholder.plc";
        if(!fs::is_regular_file(plc))plc.clear();
        const auto level=ae3::loadNativeLevelFolder(folder,plc);
        if(!gRenderer.displayScene(level.scene))
            throw std::runtime_error("Direct3D11 native level vertex upload failed");
        gAnimatedModel.reset();
        gWalkCollisionScene=level.scene;
        gOriginalI3cActive=false;
        gNativeWalkMode=false;
        const auto& box=level.scene.bounds;
        gX=box.center[0];
        gHeight=box.center[1]+box.radius*.2f;
        gZ=box.center[2]-std::max(2.f,box.radius*2.5f);
        gYaw=0;
        std::wstringstream info;
        info<<L"Native AE3 level asset scene: "
            <<level.glbSourceFiles<<L" original GLBs, "
            <<level.placedInstances<<L" instances, "
            <<level.scene.triangles.size()/3<<L" world triangles, "
            <<level.unplaced<<L" skipped unplaced objects. "
            <<(plc.empty()?L"NO PLC PROVIDED: original world placement UNVERIFIED. ":
                            L"Original simple PLC placement active. ")
            <<L"F5 activates basic native WASD/Space walk/jump test. "
            <<L"Original I3C collision and player behavior NOT YET RECOVERED.";
        status(info.str());
        gRenderer.render();
    }catch(const std::exception& e){
        status(L"Native original-level geometry load failed: "+wide(e.what()));
    }
}
void loadOriginalCollision(const fs::path& file){
    try{
        // Decode native triangle DATA. This is not original PS2 hardware emulation.
        const auto original=fs::is_directory(file)?
            ae3::loadOriginalI3cDirectory(file):ae3::parseOriginalI3c(file);
        if(!gRenderer.setCollisionOverlay(original.geometry))
            throw std::runtime_error("unable to upload original I3C diagnostic wireframe");
        gWalkCollisionScene=original.geometry;
        gOriginalI3cActive=true;
        gNativeWalkMode=false;
        std::wstringstream msg;
        msg<<L"ORIGINAL I3C COLLISION ACTIVE: "
           <<original.geometry.triangles.size()/3<<L" real triangles, "
           <<original.originalVertices<<L" original vertices, "
           <<original.bvhNodes<<L" BVH nodes. F5 walks; F7 toggles green collision wireframe. "
           <<L"Select I3C from the SAME original stage as the loaded GLB.";
        status(msg.str());
    }catch(const std::exception& error){
        status(L"Native I3C collision rejected: "+wide(error.what()));
    }
}
void openOriginalCollision(){
    wchar_t file[32768]={};
    OPENFILENAMEW dlg{};
    dlg.lStructSize=sizeof(dlg);dlg.hwndOwner=gMain;dlg.lpstrFile=file;
    dlg.nMaxFile=32768;
    dlg.lpstrFilter=L"Original AE3 collision (*.i3c;*.i3d)\0*.i3c;*.i3d\0All files (*.*)\0*.*\0";
    dlg.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_EXPLORER;
    dlg.lpstrTitle=L"Select ORIGINAL I3D_I3C collision for the same stage as the 3D model";
    if(GetOpenFileNameW(&dlg))loadOriginalCollision(fs::path(file));
}
void toggleNativeWalkMode(){
    if(gNativeWalkMode){
        gNativeWalkMode=false;
        status(L"Native walk test OFF. Free-fly camera restored. "
               L"Arrow keys rotate, WASD/QE fly.");
        return;
    }
    if(!gNativeWalker.reset(gWalkCollisionScene)){
        status(L"Cannot enter native walk test: imported 3D scene has no walkable triangle.");
        return;
    }
    gNativeWalkMode=true;
    const auto& pawn=gNativeWalker.state();
    gX=pawn.x;gHeight=pawn.y+1.65f;gZ=pawn.z;
    status(gOriginalI3cActive?
           L"ORIGINAL I3C COLLISION WALK TEST ON: WASD movement, Space jump, "
           L"Shift sprint. Movement is an experimental native controller.":
           L"NATIVE MESH WALK TEST ON: WASD movement, Shift sprint, "
           L"Space jump, arrows turn, F5 free flight. "
           L"This is an early host-native collision/controller test, NOT original AE3 gameplay.");
}
void openNativeLevel(){
    BROWSEINFOW dlg{};
    dlg.hwndOwner=gMain;
    dlg.lpszTitle=L"Choose PRIVATE exported GLB folder for native AE3 level scene";
    dlg.ulFlags=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE;
    PIDLIST_ABSOLUTE chosen=SHBrowseForFolderW(&dlg);
    if(!chosen)return;
    wchar_t folder[MAX_PATH]={};
    const BOOL ok=SHGetPathFromIDListW(chosen,folder);
    CoTaskMemFree(chosen);
    if(ok)loadNativeLevel(fs::path(folder));
}
void openRecoveredStage(){
    wchar_t path[32768]={};
    OPENFILENAMEW dialog{};
    dialog.lStructSize=sizeof(dialog);
    dialog.hwndOwner=gMain;dialog.lpstrFile=path;dialog.nMaxFile=32768;
    dialog.lpstrFilter=L"Recovered Lua 5.0 source (*.lua)\0*.lua\0All files (*.*)\0*.*\0";
    dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_EXPLORER;
    dialog.lpstrTitle=L"Open recovered AE3 area.lua (original scripts remain Lua)";
    if(!GetOpenFileNameW(&dialog))return;
    try{
        std::ifstream stream(fs::path(path),std::ios::binary);
        if(!stream)throw std::runtime_error("cannot open recovered Lua source");
        std::string source((std::istreambuf_iterator<char>(stream)),
                            std::istreambuf_iterator<char>());
        if(source.size()>16*1024*1024)
            throw std::runtime_error("Lua source exceeds stage size budget");
        auto world=std::make_unique<ae3::NativeStageWorld>();
        auto stage=std::make_unique<ae3::LuaStage>(*world);
        stage->executeSource(source,"area.lua");
        stage->callSetupArea("spawn_default");
        std::vector<ae3::NativeControllerTimeline> controllers;
        for(const auto& entity:world->entities()) {
            if(entity.controllers.kind==ae3::ScriptValue::Kind::object)
                controllers.emplace_back(entity.controllers);
        }
        gControllerTimelines=std::move(controllers);
        gScriptEventCount=0;
        gLastScriptTick=std::chrono::steady_clock::now();
        gNativeStage.reset();
        gNativeWorld=std::move(world);
        gNativeStage=std::move(stage);
        gArchive.reset();
        gRenderer.clearTexture();
        // Keep an already loaded original-model GLB visible when setting
        // up Lua stage entities: world art and Lua scripts share one window.
        // Entity-to-model placement and behavior still need reconstruction.
        SendMessageW(gList,LB_RESETCONTENT,0,0);
        for(const auto& ent:gNativeWorld->entities()){
            std::wstring line=wide(ent.entityClass)+L": "+wide(ent.name);
            if(!ent.tagName.empty())line+=L" ["+wide(ent.tagName)+L"]";
            SendMessageW(gList,LB_ADDSTRING,0,reinterpret_cast<LPARAM>(line.c_str()));
        }
        std::wstringstream info;
        info<<L"Executed original-style Lua 5.0 script on native CPU: "
            <<gNativeWorld->entities().size()<<L" entities, "
            <<gNativeWorld->parameters().size()<<L" Lua params, "
            <<gControllerTimelines.size()<<L" running native controller timelines. "
            <<L"ENGINE DESCRIPTORS ONLY: gameplay, collision and real level rendering still incomplete.";
        status(info.str());
    }catch(const std::exception& e){
        status(L"Native Lua 5.0 stage failed: "+wide(e.what()));
    }
}
void signalFirstNativeTrigger(){
    for(auto& timeline:gControllerTimelines){
        if(timeline.isWaitingForTrigger()){
            const std::string trigger=timeline.waitingTriggerName();
            const bool accepted=timeline.signal(trigger,timeline.expectedTriggerActive());
            status(L"Native controller signal "+wide(trigger)+
                   (accepted?L" accepted; timeline resumed.":L" was not accepted."));
            return;
        }
    }
    status(L"No native script controller currently waits on a trigger.");
}
void exportSelected(){
    if(!gArchive)return;
    auto index=SendMessageW(gList,LB_GETCURSEL,0,0);
    if(index==LB_ERR || static_cast<std::size_t>(index)>=gArchive->entries().size())return;
    const auto& entry=gArchive->entries()[static_cast<std::size_t>(index)];
    std::wstring suggested=wide(entry.path.substr(entry.path.find_last_of('/')+1));
    wchar_t name[MAX_PATH]={};
    wcsncpy_s(name,MAX_PATH,suggested.c_str(),_TRUNCATE);
    OPENFILENAMEW dlg{};dlg.lStructSize=sizeof(dlg);
    dlg.hwndOwner=gMain;dlg.lpstrFile=name;dlg.nMaxFile=MAX_PATH;
    dlg.lpstrFilter=L"All files (*.*)\0*.*\0";dlg.Flags=OFN_OVERWRITEPROMPT;
    if(!GetSaveFileNameW(&dlg))return;
    try{
        auto bytes=gArchive->read(entry);
        std::ofstream out(fs::path(name),std::ios::binary|std::ios::trunc);
        if(!out)throw std::runtime_error("could not create output file");
        out.write(reinterpret_cast<const char*>(bytes.data()),std::streamsize(bytes.size()));
        if(!out)throw std::runtime_error("host filesystem write failed");
        status(L"Saved native asset: "+wide(entry.path));
    }catch(const std::exception& e){status(L"Export failed: "+wide(e.what()));}
}
void layout(HWND wnd){
    RECT rect{};GetClientRect(wnd,&rect);
    int w=std::max(1L,rect.right),h=std::max(1L,rect.bottom);
    const int panel=std::clamp(w/3,250,440);
    MoveWindow(gList,10,84,panel-20,std::max(1,h-160),TRUE);
    MoveWindow(gView,panel,84,std::max(1,w-panel-10),std::max(1,h-160),TRUE);
    MoveWindow(gInfo,10,std::max(1,h-68),std::max(1,w-20),56,TRUE);
    if(gRenderer.resize(w-panel-10,h-160))gRenderer.render();
}
LRESULT CALLBACK viewProc(HWND hwnd,UINT msg,WPARAM w,LPARAM l){
    if(msg==WM_ERASEBKGND)return 1;
    if(msg==WM_SIZE){gRenderer.resize(std::max(1,int(LOWORD(l))),std::max(1,int(HIWORD(l))));return 0;}
    if(msg==WM_PAINT){
        PAINTSTRUCT ps{};BeginPaint(hwnd,&ps);EndPaint(hwnd,&ps);
        gRenderer.render();return 0;
    }
    return DefWindowProcW(hwnd,msg,w,l);
}
LRESULT CALLBACK wndProc(HWND wnd,UINT msg,WPARAM w,LPARAM l){
    switch(msg){
    case WM_CREATE:{
        gMain=wnd;
        auto inst=reinterpret_cast<LPCREATESTRUCTW>(l)->hInstance;
        CreateWindowW(L"BUTTON",L"Open owned AE3 ISO / DATA.BIN",WS_CHILD|WS_VISIBLE,
            10,8,224,32,wnd,reinterpret_cast<HMENU>(INT_PTR(kOpen)),inst,nullptr);
        CreateWindowW(L"BUTTON",L"Export selected original asset",WS_CHILD|WS_VISIBLE,
            245,8,210,32,wnd,reinterpret_cast<HMENU>(INT_PTR(kExport)),inst,nullptr);
        CreateWindowW(L"BUTTON",L"Run original Lua 5.0 stage",WS_CHILD|WS_VISIBLE,
            465,8,225,32,wnd,reinterpret_cast<HMENU>(INT_PTR(kLuaStage)),inst,nullptr);
        CreateWindowW(L"BUTTON",L"Signal Lua trigger",WS_CHILD|WS_VISIBLE,
            700,8,155,32,wnd,reinterpret_cast<HMENU>(INT_PTR(kSignalTrigger)),inst,nullptr);
        CreateWindowW(L"BUTTON",L"Open native 3D model (GLB)",WS_CHILD|WS_VISIBLE,
            866,8,240,32,wnd,reinterpret_cast<HMENU>(INT_PTR(kOpenGlb)),inst,nullptr);
        CreateWindowW(L"BUTTON",L"Open GLB level folder",WS_CHILD|WS_VISIBLE,
            1117,8,170,32,wnd,reinterpret_cast<HMENU>(INT_PTR(kOpenLevel)),inst,nullptr);
        CreateWindowW(L"BUTTON",L"Use original I3C collision",WS_CHILD|WS_VISIBLE,
            10,48,260,30,wnd,reinterpret_cast<HMENU>(INT_PTR(kOriginalCollision)),inst,nullptr);
        gList=CreateWindowExW(WS_EX_CLIENTEDGE,L"LISTBOX",nullptr,
            WS_CHILD|WS_VISIBLE|WS_VSCROLL|LBS_NOTIFY|LBS_NOINTEGRALHEIGHT,
            0,0,1,1,wnd,reinterpret_cast<HMENU>(INT_PTR(kList)),inst,nullptr);
        gView=CreateWindowW(L"AE3NativeViewport",nullptr,WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS,
            0,0,1,1,wnd,nullptr,inst,nullptr);
        gInfo=CreateWindowW(L"STATIC",L"Native Windows engine workbench. Debug grid is not the game.",
            WS_CHILD|WS_VISIBLE|SS_LEFT,0,0,1,1,wnd,reinterpret_cast<HMENU>(INT_PTR(kStatus)),inst,nullptr);
        if(!gRenderer.init(gView))status(L"Native D3D11 hardware device initialization failed.");
        SetTimer(wnd,1,16,nullptr);
        return 0;
    }
    case WM_SIZE: if(gView)layout(wnd);return 0;
    case WM_COMMAND:
        if(LOWORD(w)==kOpen && HIWORD(w)==BN_CLICKED)openFile();
        if(LOWORD(w)==kExport && HIWORD(w)==BN_CLICKED)exportSelected();
        if(LOWORD(w)==kLuaStage && HIWORD(w)==BN_CLICKED)openRecoveredStage();
        if(LOWORD(w)==kSignalTrigger && HIWORD(w)==BN_CLICKED)signalFirstNativeTrigger();
        if(LOWORD(w)==kOpenGlb && HIWORD(w)==BN_CLICKED)openNativeGlb();
        if(LOWORD(w)==kOpenLevel && HIWORD(w)==BN_CLICKED)openNativeLevel();
        if(LOWORD(w)==kOriginalCollision && HIWORD(w)==BN_CLICKED)openOriginalCollision();
        if(LOWORD(w)==kList && HIWORD(w)==LBN_SELCHANGE && gArchive){
            auto idx=SendMessageW(gList,LB_GETCURSEL,0,0);
            if(idx!=LB_ERR && static_cast<std::size_t>(idx)<gArchive->entries().size()){
                const auto& e=gArchive->entries()[static_cast<std::size_t>(idx)];
                std::wstringstream info;info<<wide(e.path)<<L" | "<<e.size
                    <<L" original bytes | "<<wide(ae3::assetType(e.path))
                    <<L" | export is native file I/O; no game logic is running.";
                gRenderer.clearTexture();
                if(e.size<=16*1024*1024) {
                    try {
                        auto bytes=gArchive->read(e);
                        if(ae3::isTim2(bytes)){
                            auto image=ae3::decodeTim2(bytes);
                            if(gRenderer.displayTexture(image)){
                                info<<L" | LIVE NATIVE TIM2 PREVIEW "<<image.width
                                    <<L"x"<<image.height;
                            }else info<<L" | D3D11 texture upload failed";
                        }
                    } catch(const std::exception& error) {
                        info<<L" | asset preview unavailable: "<<wide(error.what());
                    }
                }
                status(info.str());
            }
        }
        return 0;
    case WM_KEYDOWN: if(w<256)gKeys[w]=true;return 0;
    case WM_KEYUP: if(w<256)gKeys[w]=false;return 0;
    case WM_TIMER:
        if(w==1){
            // Native host-time game loop. Controller and Lua use the
            // same monotonic time delta: NO PlayStation hardware clock.
            const auto now=std::chrono::steady_clock::now();
            const double dt=std::clamp(
                std::chrono::duration<double>(now-gLastScriptTick).count(),0.0,0.1);
            gLastScriptTick=now;
            auto held=[&](int key)->bool{
                return GetForegroundWindow()==wnd &&
                       (GetAsyncKeyState(key)&0x8000)!=0;
            };
            const bool f5=held(VK_F5);
            if(f5&&!gF5HeldLastFrame)toggleNativeWalkMode();
            gF5HeldLastFrame=f5;
            const bool f7=held(VK_F7);
            if(f7&&!gF7HeldLastFrame){
                const bool visible=gRenderer.toggleCollisionOverlay();
                status(visible?L"Original I3C collision diagnostic overlay visible.":
                               L"Original I3C collision wireframe hidden; collision remains active.");
            }
            gF7HeldLastFrame=f7;
            const bool f8=held(VK_F8);
            if(f8&&!gF8HeldLastFrame&&gAnimatedModel){
                gActiveClip=(gActiveClip+1)%gAnimatedModel->clipCount();
                gAnimationSeconds=0;
                status(L"Native I3M/glTF skeleton animation clip: "+
                       wide(gAnimatedModel->clipNames()[gActiveClip]));
            }
            gF8HeldLastFrame=f8;
            const bool f9=held(VK_F9);
            if(f9&&!gF9HeldLastFrame&&gAnimatedModel){
                gAnimationPaused=!gAnimationPaused;
                status(gAnimationPaused?L"Native skinned animation paused (F9 resumes).":
                                        L"Native skeletal animation playing.");
            }
            gF9HeldLastFrame=f9;
            if(gAnimatedModel && !gAnimationPaused){
                try{
                    gAnimationSeconds+=dt;
                    const auto& pose=gAnimatedModel->sample(gActiveClip,gAnimationSeconds);
                    if(!gRenderer.updateAnimation(pose))
                        throw std::runtime_error("Direct3D11 animated vertex-buffer map failed");
                }catch(const std::exception& e){
                    status(L"Native original skeleton playback stopped: "+wide(e.what()));
                    gAnimatedModel.reset();
                }
            }
            if(held(VK_LEFT))gYaw-=.025f;
            if(held(VK_RIGHT))gYaw+=.025f;
            if(gNativeWalkMode){
                ae3::WalkInput input;
                input.forward=(held('W')?1.f:0.f)-(held('S')?1.f:0.f);
                input.strafe=(held('D')?1.f:0.f)-(held('A')?1.f:0.f);
                input.facingRadians=gYaw;
                input.sprint=held(VK_SHIFT);
                input.jump=held(VK_SPACE);
                try{
                    gNativeWalker.tick(gWalkCollisionScene,
                                      static_cast<float>(dt),input);
                    const auto& pawn=gNativeWalker.state();
                    gX=pawn.x;gHeight=pawn.y+1.65f;gZ=pawn.z;
                }catch(const std::exception& e){
                    status(L"Native walking test stopped: "+wide(e.what()));
                    gNativeWalkMode=false;
                }
            }else{
                // Free-flight camera for examining original GLB exports.
                float speed=.055f;
                if(held(VK_SHIFT))speed*=3;
                const float dx=std::sin(gYaw),dz=std::cos(gYaw);
                if(held('W')){gX+=dx*speed;gZ+=dz*speed;}
                if(held('S')){gX-=dx*speed;gZ-=dz*speed;}
                if(held('A')){gX-=dz*speed;gZ+=dx*speed;}
                if(held('D')){gX+=dz*speed;gZ-=dx*speed;}
                if(held('Q'))gHeight-=speed;
                if(held('E'))gHeight+=speed;
            }
            if(gNativeWorld) {
                try{
                    for(auto& controller:gControllerTimelines) {
                        const auto events=controller.advance(dt);
                        for(const auto& event:events) {
                            ++gScriptEventCount;
                            // Debug only: real gameplay event handlers remain
                            // to be recovered (audio, physics, AI, messaging).
                            std::wstringstream info;
                            info<<L"Lua event #"<<gScriptEventCount<<L" at "
                                <<event.gameSeconds<<L"s: "<<wide(event.opcode);
                            const std::wstring item=info.str();
                            if(SendMessageW(gList,LB_GETCOUNT,0,0)>1500)
                                SendMessageW(gList,LB_DELETESTRING,0,0);
                            SendMessageW(gList,LB_ADDSTRING,0,
                                         reinterpret_cast<LPARAM>(item.c_str()));
                        }
                    }
                }catch(const std::exception& e){
                    status(L"Lua controller runtime stopped: "+wide(e.what()));
                    gControllerTimelines.clear();
                }
            }
            gRenderer.render();
        }
        return 0;
    case WM_DESTROY:KillTimer(wnd,1);PostQuitMessage(0);return 0;
    }
    return DefWindowProcW(wnd,msg,w,l);
}
} // namespace
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE,PWSTR command,int show){
    WNDCLASSW view{};view.lpfnWndProc=viewProc;view.hInstance=inst;view.lpszClassName=L"AE3NativeViewport";
    view.hCursor=LoadCursorW(nullptr,IDC_ARROW);RegisterClassW(&view);
    WNDCLASSW win{};win.lpfnWndProc=wndProc;win.hInstance=inst;win.lpszClassName=L"AE3NativePort";
    win.hCursor=LoadCursorW(nullptr,IDC_ARROW);win.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
    if(!RegisterClassW(&win))return 1;
    const HRESULT com=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    HWND window=CreateWindowW(L"AE3NativePort",
        L"Getchu 3 — NATIVE PC PORT WORKBENCH (not yet playable)",
        WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,1280,750,
        nullptr,nullptr,inst,nullptr);
    if(!window){
        if(SUCCEEDED(com))CoUninitialize();
        return 2;
    }
    ShowWindow(window,show);UpdateWindow(window);
    if(command && *command){
        std::wstring arg=command;
        if(arg.front()==L'"' && arg.back()==L'"')arg=arg.substr(1,arg.size()-2);
        const fs::path file=fs::path(arg);
        std::wstring extension=file.extension().wstring();
        std::transform(extension.begin(),extension.end(),extension.begin(),
                       [](wchar_t c){return wchar_t(std::towlower(c));});
        if(fs::is_directory(file))loadNativeLevel(file);
        else if(extension==L".glb")loadNativeGlb(file);
        else if(extension==L".i3c"||extension==L".i3d")loadOriginalCollision(file);
        else loadArchive(file);
    }
    MSG message{};
    while(GetMessageW(&message,nullptr,0,0)>0){
        TranslateMessage(&message);DispatchMessageW(&message);
    }
    if(SUCCEEDED(com))CoUninitialize();
    return int(message.wParam);
}
