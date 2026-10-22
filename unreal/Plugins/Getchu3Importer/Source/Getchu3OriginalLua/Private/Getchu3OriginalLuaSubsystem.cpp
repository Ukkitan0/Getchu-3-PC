#include "Getchu3OriginalLuaSubsystem.h"
#include "ae3/ae3_unreal_lua_bridge.h"

#include "Containers/StringConv.h"
#include "HAL/PlatformProcess.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogGetchu3OriginalLua, Log, All);

namespace {
template<typename FunctionPointer>
FunctionPointer GetOriginalLuaFunction(void* Library, const TCHAR* Name) {
    return reinterpret_cast<FunctionPointer>(FPlatformProcess::GetDllExport(Library, Name));
}

FString Utf8String(const char* Bytes) {
    return Bytes ? FString(UTF8_TO_TCHAR(Bytes)) : FString();
}

UGetchu3OriginalManagerProvider* OriginalManager(void* Context) {
    auto* Instance = static_cast<UGetchu3OriginalLuaSubsystem*>(Context);
    return Instance && IsInGameThread() && IsValid(Instance->GetOriginalManager())
        ? Instance->GetOriginalManager() : nullptr;
}

int CallbackCreateEntity(void* Context, const char* ClassHex, const char* NameHex,
                         const char* TagHex, const char* FieldsTypedJson,
                         const char* ControllersTypedJson) {
    auto* Manager = OriginalManager(Context);
    if (!Manager) return 0;
    return Manager->CreateOriginalEntity(
        Utf8String(ClassHex), Utf8String(NameHex), Utf8String(TagHex),
        Utf8String(FieldsTypedJson), Utf8String(ControllersTypedJson)) ? 1 : 0;
}

int CallbackStageParameters(void* Context, const char* ParametersTypedJson) {
    auto* Manager = OriginalManager(Context);
    return Manager && Manager->SubmitOriginalParameters(Utf8String(ParametersTypedJson))
        ? 1 : 0;
}

int CallbackWriteFlag(void* Context, const char* Scope, const char* Key, int Value) {
    auto* Manager = OriginalManager(Context);
    return Manager && Manager->WriteOriginalFlag(
        Utf8String(Scope), Utf8String(Key), Value != 0) ? 1 : 0;
}

int CallbackReadFlag(void* Context, const char* Scope, const char* Key, int* Value) {
    auto* Manager = OriginalManager(Context);
    if (!Manager || !Value) return 0;
    bool Result = false;
    if (!Manager->ReadOriginalFlag(Utf8String(Scope), Utf8String(Key), Result))
        return 0;
    *Value = Result ? 1 : 0;
    return 1;
}

int CallbackReadStageInt(void* Context, const char* TypedArguments, int* Value) {
    auto* Manager = OriginalManager(Context);
    if (!Manager || !Value) return 0;
    int32 Result = 0;
    if (!Manager->ReadOriginalStageInteger(Utf8String(TypedArguments), Result))
        return 0;
    *Value = static_cast<int>(Result);
    return 1;
}

int CallbackReadSurvival(void* Context, int* Value) {
    auto* Manager = OriginalManager(Context);
    if (!Manager || !Value) return 0;
    bool Result = false;
    if (!Manager->ReadOriginalSurvivalMode(Result))
        return 0;
    *Value = Result ? 1 : 0;
    return 1;
}

int CallbackSelectWeapon(void* Context, int Selection) {
    auto* Manager = OriginalManager(Context);
    return Manager && Manager->SelectOriginalPlayerWeapon(Selection) ? 1 : 0;
}
}

bool UGetchu3OriginalLuaSubsystem::StartOriginalStage(
    const FString& OriginalLuaFilePath, const FString& OriginalSpawnTag,
    UGetchu3OriginalManagerProvider* SourceManager) {
    if (!IsInGameThread()) {
        LastError = TEXT("Original Lua runtime may only execute on Unreal's game thread");
        return false;
    }
    StopOriginalStage();
    LastError.Empty();

    if (!IsValid(SourceManager)) {
        LastError = TEXT("Original source-backed game manager is not available");
        return false;
    }
    if (OriginalSpawnTag.IsEmpty() || OriginalLuaFilePath.IsEmpty()) {
        LastError = TEXT("An original stage script and real spawn tag are required");
        return false;
    }
    TArray<uint8> Chunk;
    const int64 FileBytes = IFileManager::Get().FileSize(*OriginalLuaFilePath);
    if (FileBytes < 5 || FileBytes > 16 * 1024 * 1024 ||
        !FFileHelper::LoadFileToArray(Chunk, *OriginalLuaFilePath) ||
        Chunk.Num() != FileBytes) {
        LastError = TEXT("Missing, truncated, or oversized owner-original Lua stage file");
        return false;
    }
    if (Chunk[0] != 0x1b || Chunk[1] != 'L' || Chunk[2] != 'u' ||
        Chunk[3] != 'a' || Chunk[4] != 0x50) {
        LastError = TEXT("Rejected non-original or incompatible Lua bytecode");
        return false;
    }
#if !PLATFORM_WINDOWS
    LastError = TEXT("Source-backed original native Lua bridge is currently built for Windows only");
    return false;
#else
    const FString BridgePath = FPaths::Combine(
        FPaths::ProjectDir(), TEXT("Binaries"), TEXT("Win64"),
        TEXT("Getchu3OriginalLuaBridge.dll"));
    if (!FPaths::FileExists(BridgePath)) {
        LastError = TEXT("Original native Lua DLL missing; build the real native Lua 5.0 bridge");
        return false;
    }
    NativeLibrary = FPlatformProcess::GetDllHandle(*BridgePath);
    if (!NativeLibrary) {
        LastError = TEXT("Could not load original native Lua 5.0 bridge DLL");
        return false;
    }

    const auto Version = GetOriginalLuaFunction<decltype(&ae3_lua_api_version)>(
        NativeLibrary, TEXT("ae3_lua_api_version"));
    const auto Create = GetOriginalLuaFunction<decltype(&ae3_lua_create)>(
        NativeLibrary, TEXT("ae3_lua_create"));
    const auto Execute = GetOriginalLuaFunction<decltype(&ae3_lua_execute_original_chunk)>(
        NativeLibrary, TEXT("ae3_lua_execute_original_chunk"));
    const auto SetupArea = GetOriginalLuaFunction<decltype(&ae3_lua_setup_area)>(
        NativeLibrary, TEXT("ae3_lua_setup_area"));
    const auto Destroy = GetOriginalLuaFunction<decltype(&ae3_lua_destroy)>(
        NativeLibrary, TEXT("ae3_lua_destroy"));
    const auto Error = GetOriginalLuaFunction<decltype(&ae3_lua_last_error)>(
        NativeLibrary, TEXT("ae3_lua_last_error"));
    if (!Version || !Create || !Execute || !SetupArea || !Destroy || !Error ||
        Version() != AE3_LUA_BRIDGE_API_VERSION) {
        LastError = TEXT("Original native Lua bridge exports or API version do not match the Unreal runtime");
        StopOriginalStage();
        return false;
    }

    Manager = SourceManager; // Strong UPROPERTY retained through native callbacks.
    Ae3LuaManagerV1 Bindings{};
    Bindings.api_version = AE3_LUA_BRIDGE_API_VERSION;
    Bindings.struct_size = sizeof(Bindings);
    Bindings.user = this;
    Bindings.create_entity = &CallbackCreateEntity;
    Bindings.submit_parameters = &CallbackStageParameters;
    Bindings.write_flag = &CallbackWriteFlag;
    Bindings.read_flag = &CallbackReadFlag;
    Bindings.read_stage_int = &CallbackReadStageInt;
    Bindings.read_survival = &CallbackReadSurvival;
    Bindings.select_weapon = &CallbackSelectWeapon;

    NativeState = Create(&Bindings);
    if (!NativeState) {
        LastError = TEXT("Original Lua VM rejected manager bindings or failed to initialize");
        StopOriginalStage();
        return false;
    }
    const FTCHARToUTF8 NameUtf8(*OriginalLuaFilePath);
    const FTCHARToUTF8 TagUtf8(*OriginalSpawnTag);
    if (!Execute(static_cast<Ae3LuaRuntime*>(NativeState), Chunk.GetData(),
                 static_cast<size_t>(Chunk.Num()), NameUtf8.Get()) ||
        !SetupArea(static_cast<Ae3LuaRuntime*>(NativeState), TagUtf8.Get())) {
        LastError = Utf8String(Error(static_cast<Ae3LuaRuntime*>(NativeState)));
        if (LastError.IsEmpty()) LastError = TEXT("Original Lua 5.0 game initialization failed");
        UE_LOG(LogGetchu3OriginalLua, Error, TEXT("%s"), *LastError);
        StopOriginalStage();
        return false;
    }
    UE_LOG(LogGetchu3OriginalLua, Log,
        TEXT("Original source Lua stage ran against explicit game manager bindings; gameplay fidelity not certified."));
    return true;
#endif
}

void UGetchu3OriginalLuaSubsystem::StopOriginalStage() {
    if (NativeState && NativeLibrary) {
        const auto Destroy = GetOriginalLuaFunction<decltype(&ae3_lua_destroy)>(
            NativeLibrary, TEXT("ae3_lua_destroy"));
        if (Destroy)
            Destroy(static_cast<Ae3LuaRuntime*>(NativeState));
    }
    NativeState = nullptr;
    Manager = nullptr;
    if (NativeLibrary)
        FPlatformProcess::FreeDllHandle(NativeLibrary);
    NativeLibrary = nullptr;
}

void UGetchu3OriginalLuaSubsystem::ReportRuntimeError(const FString& Fallback) {
    LastError = Fallback;
    if (NativeState && NativeLibrary) {
        const auto Error = GetOriginalLuaFunction<decltype(&ae3_lua_last_error)>(
            NativeLibrary, TEXT("ae3_lua_last_error"));
        if (Error) {
            const FString Source = Utf8String(
                Error(static_cast<Ae3LuaRuntime*>(NativeState)));
            if (!Source.IsEmpty()) LastError = Source;
        }
    }
}

void UGetchu3OriginalLuaSubsystem::Deinitialize() {
    StopOriginalStage();
    Super::Deinitialize();
}
