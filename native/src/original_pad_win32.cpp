#include "ae3/original_pad_runtime.hpp"
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <Xinput.h>
#include <algorithm>
#include <cstdint>

namespace ae3 {
namespace {
bool inFocus(void* window) noexcept {
    return window && GetForegroundWindow()==static_cast<HWND>(window);
}
bool down(unsigned key) noexcept {
    return key!=0 && key<=255 && (GetAsyncKeyState(static_cast<int>(key)) & 0x8000)!=0;
}
std::uint8_t stickFromXInput(SHORT v,bool reverse) noexcept {
    // Host-axis quantization only. Does not claim original analog filtering.
    const std::uint32_t range=static_cast<std::uint32_t>(
        static_cast<std::int32_t>(v)+32768);
    const auto value=static_cast<std::uint8_t>(range>>8);
    return reverse?static_cast<std::uint8_t>(255u-value):value;
}
using GetStateFunction=DWORD (WINAPI*)(DWORD,XINPUT_STATE*);
GetStateFunction lookupGetState() noexcept {
    static const auto function=[]() -> GetStateFunction {
        for(const wchar_t* dll:{L"xinput1_4.dll",L"xinput1_3.dll",L"xinput9_1_0.dll"}){
            const HMODULE module=LoadLibraryW(dll);
            if(!module)continue;
            const auto fn=reinterpret_cast<GetStateFunction>(
                GetProcAddress(module,"XInputGetState"));
            if(fn)return fn; // keep DLL loaded for callback lifetime
            FreeLibrary(module);
        }
        return nullptr;
    }();
    return function;
}
} // namespace
OriginalPadObservation pollWin32OriginalKeyboard(
    void* hostWindow,const OriginalKeyboardBindings& binding) noexcept {
    OriginalPadObservation result;
    if(!inFocus(hostWindow))return result;
    result.connected=true;
    for(std::size_t i=0;i<binding.buttonVirtualKeys.size();++i)
        if(down(binding.buttonVirtualKeys[i]))
            result.pressedMask|=static_cast<std::uint16_t>(1u<<i);
    for(std::size_t i=0;i<4;++i){
        const bool negative=down(binding.axes[i].negativeVirtualKey);
        const bool positive=down(binding.axes[i].positiveVirtualKey);
        result.sticks[i]=negative==positive?128:(negative?0:255);
    }
    // Keyboard has no original pressure-sensitive trigger representation.
    result.leftTrigger=(result.pressedMask & originalPadBit(OriginalPadButton::l2))?255:0;
    result.rightTrigger=(result.pressedMask & originalPadBit(OriginalPadButton::r2))?255:0;
    return result;
}
OriginalPadObservation pollWin32OriginalGamepad(
    void* hostWindow,unsigned index) noexcept {
    OriginalPadObservation result;
    if(!inFocus(hostWindow)||index>=XUSER_MAX_COUNT)return result;
    const auto getState=lookupGetState();
    if(!getState)return result;
    XINPUT_STATE state{};
    if(getState(index,&state)!=ERROR_SUCCESS)return result;
    const auto& p=state.Gamepad;
    result.connected=true;
    const struct Physical {WORD xinput;OriginalPadButton original;} mapping[]={
        {XINPUT_GAMEPAD_BACK,OriginalPadButton::select},
        {XINPUT_GAMEPAD_LEFT_THUMB,OriginalPadButton::left_stick},
        {XINPUT_GAMEPAD_RIGHT_THUMB,OriginalPadButton::right_stick},
        {XINPUT_GAMEPAD_START,OriginalPadButton::start},
        {XINPUT_GAMEPAD_DPAD_UP,OriginalPadButton::up},
        {XINPUT_GAMEPAD_DPAD_RIGHT,OriginalPadButton::right},
        {XINPUT_GAMEPAD_DPAD_DOWN,OriginalPadButton::down},
        {XINPUT_GAMEPAD_DPAD_LEFT,OriginalPadButton::left},
        {XINPUT_GAMEPAD_LEFT_SHOULDER,OriginalPadButton::l1},
        {XINPUT_GAMEPAD_RIGHT_SHOULDER,OriginalPadButton::r1},
        {XINPUT_GAMEPAD_Y,OriginalPadButton::triangle},
        {XINPUT_GAMEPAD_B,OriginalPadButton::circle},
        {XINPUT_GAMEPAD_A,OriginalPadButton::cross},
        {XINPUT_GAMEPAD_X,OriginalPadButton::square}
    };
    for(const auto& button:mapping)
        if(p.wButtons & button.xinput)result.pressedMask|=originalPadBit(button.original);
    result.leftTrigger=p.bLeftTrigger;
    result.rightTrigger=p.bRightTrigger;
    if(p.bLeftTrigger)result.pressedMask|=originalPadBit(OriginalPadButton::l2);
    if(p.bRightTrigger)result.pressedMask|=originalPadBit(OriginalPadButton::r2);
    result.sticks={stickFromXInput(p.sThumbLX,false),stickFromXInput(p.sThumbLY,true),
                   stickFromXInput(p.sThumbRX,false),stickFromXInput(p.sThumbRY,true)};
    return result;
}
} // namespace ae3
#else
namespace ae3 {
OriginalPadObservation pollWin32OriginalKeyboard(
    void*,const OriginalKeyboardBindings&) noexcept {return {};}
OriginalPadObservation pollWin32OriginalGamepad(void*,unsigned) noexcept {return {};}
} // namespace ae3
#endif
