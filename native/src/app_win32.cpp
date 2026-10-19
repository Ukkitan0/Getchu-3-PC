#include "ae3/archive.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace DirectX;
namespace fs = std::filesystem;

namespace {
constexpr int kList=100, kOpen=101, kExport=102, kStatus=103;
HWND gMain=nullptr,gList=nullptr,gView=nullptr,gInfo=nullptr;
std::unique_ptr<ae3::Archive> gArchive;
std::wstring gSource, gError, gProgress;
float gYaw=.4f,gX=0,gZ=-5,gHeight=2.5f;
bool gKeys[256]={};
struct Vertex { float x,y,z; float r,g,b,a; };
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
            struct Vin {float3 pos:POSITION; float4 color:COLOR;};
            struct Vout {float4 pos:SV_POSITION; float4 color:COLOR;};
            Vout VSMain(Vin i){Vout o;o.pos=mul(float4(i.pos,1),mvp);o.color=i.color;return o;}
            float4 PSMain(Vout i):SV_TARGET{return i.color;}
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
        const D3D11_INPUT_ELEMENT_DESC input[] = {
            {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
            {"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
        };
        if(FAILED(device_->CreateInputLayout(input,2,vs->GetBufferPointer(),
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
        return resize(std::max(1L,rc.right),std::max(1L,rc.bottom));
    }
    bool resize(int width,int height){
        if(!swap_||width<=0||height<=0)return false;
        context_->OMSetRenderTargets(0,nullptr,nullptr);
        target_.Reset();
        if(FAILED(swap_->ResizeBuffers(0,width,height,DXGI_FORMAT_UNKNOWN,0)))return false;
        ComPtr<ID3D11Texture2D> back;
        if(FAILED(swap_->GetBuffer(0,IID_PPV_ARGS(&back))))return false;
        if(FAILED(device_->CreateRenderTargetView(back.Get(),nullptr,&target_)))return false;
        width_=width;height_=height;return true;
    }
    void render(){
        if(!target_)return;
        static constexpr float sky[4]={.055f,.09f,.14f,1};
        context_->ClearRenderTargetView(target_.Get(),sky);
        D3D11_VIEWPORT vp{};vp.Width=float(width_);vp.Height=float(height_);
        vp.MinDepth=0;vp.MaxDepth=1;
        context_->RSSetViewports(1,&vp);
        context_->OMSetRenderTargets(1,target_.GetAddressOf(),nullptr);
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
        ID3D11Buffer* vb=vertices_.Get();
        context_->IASetVertexBuffers(0,1,&vb,&stride,&offset);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        context_->VSSetShader(vs_.Get(),nullptr,0);
        context_->PSSetShader(ps_.Get(),nullptr,0);
        context_->Draw(gridCount_,0);
        swap_->Present(1,0);
    }
private:
    HWND hwnd_{};
    int width_=1,height_=1;
    UINT gridCount_=0;
    ComPtr<IDXGISwapChain> swap_;
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<ID3D11RenderTargetView> target_;
    ComPtr<ID3D11Buffer> vertices_,constants_;
    ComPtr<ID3D11VertexShader> vs_;
    ComPtr<ID3D11PixelShader> ps_;
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
    MoveWindow(gList,10,46,panel-20,std::max(1,h-122),TRUE);
    MoveWindow(gView,panel,46,std::max(1,w-panel-10),std::max(1,h-122),TRUE);
    MoveWindow(gInfo,10,std::max(1,h-68),std::max(1,w-20),56,TRUE);
    if(gRenderer.resize(w-panel-10,h-122))gRenderer.render();
}
LRESULT CALLBACK viewProc(HWND hwnd,UINT msg,WPARAM w,LPARAM l){
    if(msg==WM_ERASEBKGND)return 1;
    if(msg==WM_SIZE){gRenderer.resize(std::max(1,LOWORD(l)),std::max(1,HIWORD(l)));return 0;}
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
        if(LOWORD(w)==kList && HIWORD(w)==LBN_SELCHANGE && gArchive){
            auto idx=SendMessageW(gList,LB_GETCURSEL,0,0);
            if(idx!=LB_ERR && static_cast<std::size_t>(idx)<gArchive->entries().size()){
                const auto& e=gArchive->entries()[static_cast<std::size_t>(idx)];
                std::wstringstream info;info<<wide(e.path)<<L" | "<<e.size
                    <<L" original bytes | "<<wide(ae3::assetType(e.path))
                    <<L" | export is native file I/O; no game logic is running.";
                status(info.str());
            }
        }
        return 0;
    case WM_KEYDOWN: if(w<256)gKeys[w]=true;return 0;
    case WM_KEYUP: if(w<256)gKeys[w]=false;return 0;
    case WM_TIMER:
        if(w==1){
            float speed=.055f;
            if(gKeys[VK_SHIFT])speed*=3;
            float dx=std::sin(gYaw),dz=std::cos(gYaw);
            if(gKeys['W']){gX+=dx*speed;gZ+=dz*speed;}
            if(gKeys['S']){gX-=dx*speed;gZ-=dz*speed;}
            if(gKeys['A']){gX-=dz*speed;gZ+=dx*speed;}
            if(gKeys['D']){gX+=dz*speed;gZ-=dx*speed;}
            if(gKeys['Q'])gHeight-=speed;
            if(gKeys['E'])gHeight+=speed;
            if(gKeys[VK_LEFT])gYaw-=.025f;
            if(gKeys[VK_RIGHT])gYaw+=.025f;
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
    HWND window=CreateWindowW(L"AE3NativePort",
        L"Getchu 3 — NATIVE PC PORT WORKBENCH (not yet playable)",
        WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,1280,750,
        nullptr,nullptr,inst,nullptr);
    if(!window)return 2;
    ShowWindow(window,show);UpdateWindow(window);
    if(command && *command){
        std::wstring arg=command;
        if(arg.front()==L'"' && arg.back()==L'"')arg=arg.substr(1,arg.size()-2);
        loadArchive(fs::path(arg));
    }
    MSG message{};
    while(GetMessageW(&message,nullptr,0,0)>0){
        TranslateMessage(&message);DispatchMessageW(&message);
    }
    return int(message.wParam);
}
