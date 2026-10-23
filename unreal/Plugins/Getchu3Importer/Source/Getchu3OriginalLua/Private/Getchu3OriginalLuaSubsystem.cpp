#include "Getchu3OriginalLuaSubsystem.h"
#include "ae3/ae3_unreal_lua_bridge.h"

#include "Containers/StringConv.h"
#include "HAL/PlatformProcess.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
// FPlatformMisc::GetSHA256Signature is a fatal stub on Windows in some
// Unreal versions. Use the Windows CNG SHA-256 provider directly instead.
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "ae3/windows_source_sha256.hpp"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
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

int CallbackReadOriginalProgress(void* Context, void* Destination, size_t ExactBytes) {
    auto* Manager = OriginalManager(Context);
    if (!Manager || !Destination || ExactBytes != 1033) return 0;
    TArray<uint8> OriginalPackedBytes;
    if (!Manager->ReadOriginalPlayerProgress(OriginalPackedBytes) ||
        OriginalPackedBytes.Num() != ExactBytes)
        return 0;
    FMemory::Memcpy(Destination, OriginalPackedBytes.GetData(), ExactBytes);
    return 1;
}

int CallbackSelectWeapon(void* Context, int Selection) {
    auto* Manager = OriginalManager(Context);
    return Manager && Manager->SelectOriginalPlayerWeapon(Selection) ? 1 : 0;
}
}

bool UGetchu3OriginalLuaSubsystem::StartImportedOriginalStage(
    const FString& OriginalStageId, const FString& OriginalSpawnTag,
    UGetchu3OriginalManagerProvider* SourceManager)
{
    // A successful importer reports authentic source assets, never completed
    // gameplay. Only a real source-backed manager can start a game stage.
    if (!IsInGameThread()) {
        LastError = TEXT("Original Lua stage selection must run on the game thread");
        return false;
    }
    if (bInOriginalManagerCall) {
        bDeferredOriginalStop = true;
        LastError = TEXT("Original stage replacement requested during a native manager callback");
        return false;
    }
    StopOriginalStage();
    LastError.Empty();

    if (OriginalStageId.IsEmpty() || OriginalStageId.Len() > 64) {
        LastError = TEXT("Invalid original USA stage identifier");
        return false;
    }
    for (TCHAR Character : OriginalStageId) {
        if (!((Character >= TEXT('a') && Character <= TEXT('z')) ||
              (Character >= TEXT('A') && Character <= TEXT('Z')) ||
              (Character >= TEXT('0') && Character <= TEXT('9')) ||
              Character == TEXT('_'))) {
            LastError = TEXT("Stage ID contains characters absent from original source stage names");
            return false;
        }
    }

    const FString PrivateRoot = FPaths::ConvertRelativePathToFull(
        FPaths::ProjectSavedDir() / TEXT("Getchu3Private"));
    FString PrivateDir = PrivateRoot;
    FPaths::NormalizeDirectoryName(PrivateDir);
    const FString StatePath = PrivateDir / TEXT("Getchu3ImportStatus.json");
    FString StateText;
    TSharedPtr<FJsonObject> State;
    if (!FFileHelper::LoadFileToString(StateText, *StatePath) ||
        !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(StateText), State) ||
        !State.IsValid()) {
        LastError = TEXT("No complete owner-original ISO import status was found");
        return false;
    }

    FString Status, Workspace;
    if (!State->TryGetStringField(TEXT("status"), Status) ||
        Status != TEXT("original_models_textures_streams_i3c_imported_gameplay_not_implemented") ||
        !State->TryGetStringField(TEXT("workspace"), Workspace)) {
        LastError = TEXT("Original ISO import has not completed; cannot launch an unverified stage");
        return false;
    }
    FString WorkspaceDir = FPaths::ConvertRelativePathToFull(Workspace);
    FPaths::NormalizeDirectoryName(WorkspaceDir);
    if (!WorkspaceDir.StartsWith(PrivateDir + TEXT("/"), ESearchCase::IgnoreCase) ||
        WorkspaceDir.Contains(TEXT("..")) || WorkspaceDir == PrivateDir) {
        LastError = TEXT("Original stage workspace is not inside the owner-private import directory");
        return false;
    }

    const FString CatalogPath = WorkspaceDir / TEXT("getchu3_original_stages.json");
    FString CatalogText;
    TSharedPtr<FJsonObject> Catalog;
    if (!FFileHelper::LoadFileToString(CatalogText, *CatalogPath) ||
        !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(CatalogText), Catalog) ||
        !Catalog.IsValid()) {
        LastError = TEXT("Original stage source catalog missing or invalid");
        return false;
    }
    FString Schema;
    bool bCatalogClaimsGameplay = true; // Fail closed if the field is absent.
    if (!Catalog->TryGetStringField(TEXT("schema"), Schema) ||
        Schema != TEXT("getchu3-original-stage-catalog/v1") ||
        !Catalog->TryGetBoolField(TEXT("gameplay_ready"), bCatalogClaimsGameplay) ||
        bCatalogClaimsGameplay) {
        LastError = TEXT("Unsupported or falsely certified original stage catalog");
        return false;
    }
    const TArray<TSharedPtr<FJsonValue>>* Stages = nullptr;
    if (!Catalog->TryGetArrayField(TEXT("stages"), Stages) || !Stages) {
        LastError = TEXT("Original stage catalog has no verified stage list");
        return false;
    }

    FString SourceRelative, SourceHash;
    int32 Found = 0;
    for (const TSharedPtr<FJsonValue>& Item : *Stages) {
        if (!Item.IsValid() || Item->Type != EJson::Object) continue;
        const TSharedPtr<FJsonObject> Stage = Item->AsObject();
        if (!Stage.IsValid()) continue;
        FString Id;
        if (!Stage->TryGetStringField(TEXT("stage"), Id) ||
            !Id.Equals(OriginalStageId, ESearchCase::IgnoreCase)) continue;
        const TArray<TSharedPtr<FJsonValue>>* Scripts = nullptr;
        if (!Stage->TryGetArrayField(TEXT("area_lua_sources"), Scripts) || !Scripts) {
            LastError = TEXT("Original stage is missing its source area.luc");
            return false;
        }
        for (const TSharedPtr<FJsonValue>& ScriptItem : *Scripts) {
            if (!ScriptItem.IsValid() || ScriptItem->Type != EJson::Object) continue;
            const TSharedPtr<FJsonObject> Script = ScriptItem->AsObject();
            if (!Script.IsValid()) continue;
            FString Relative, Sha;
            if (Script->TryGetStringField(TEXT("original_path"), Relative) &&
                Script->TryGetStringField(TEXT("sha256"), Sha)) {
                ++Found;
                SourceRelative = Relative;
                SourceHash = Sha;
            }
        }
    }
    if (Found != 1 || SourceHash.Len() != 64) {
        LastError = TEXT("Source stage ID is missing or maps to multiple original Lua scripts");
        return false;
    }

    SourceRelative.ReplaceInline(TEXT("\\"), TEXT("/"));
    const FString ExactSource = TEXT("debug/us/stage/") + OriginalStageId.ToLower() +
        TEXT("/area.luc");
    // The Python ISO importer accepts exactly this original 5-component
    // source path. Native execution must use the identical contract rather
    // than accepting arbitrary nested files or locally edited JSON.
    if (!SourceRelative.Equals(ExactSource, ESearchCase::IgnoreCase)) {
        LastError = TEXT("Original Lua source path failed stage-local containment check");
        return false;
    }
    const FString SourceFile = WorkspaceDir / TEXT("original") / TEXT("vfi") / SourceRelative;
    TArray<uint8> LuaBytes;
    const int64 Length = IFileManager::Get().FileSize(*SourceFile);
    if (Length < 14 || Length > 16 * 1024 * 1024 ||
        !FFileHelper::LoadFileToArray(LuaBytes, *SourceFile) ||
        LuaBytes.Num() != Length) {
        LastError = TEXT("Owner-original stage script is missing or incomplete");
        return false;
    }
#if !PLATFORM_WINDOWS
    LastError = TEXT("The original SHA-256 stage verifier is supported on Windows only");
    return false;
#else
    // The same CNG helper is exercised by native Windows CTest before the
    // Unreal editor is opened. A missing provider fails safely.
    uint8 DigestBytes[32] = {};
    if (!ae3::originalSourceSha256(LuaBytes.GetData(),
                                  static_cast<size_t>(LuaBytes.Num()),
                                  DigestBytes)) {
        LastError = TEXT("Windows SHA-256 verification failed");
        return false;
    }
    constexpr TCHAR Hex[] = TEXT("0123456789abcdef");
    FString ActualDigest;
    ActualDigest.Reserve(64);
    for (const uint8 Byte : DigestBytes) {
        ActualDigest.AppendChar(Hex[Byte >> 4]);
        ActualDigest.AppendChar(Hex[Byte & 15]);
    }
    if (!ActualDigest.Equals(SourceHash, ESearchCase::IgnoreCase)) {
        LastError = TEXT("Original stage Lua SHA-256 differs from verified ISO source");
        return false;
    }
#endif

    return StartOriginalStage(SourceFile, OriginalSpawnTag, SourceManager, SourceHash);
}

bool UGetchu3OriginalLuaSubsystem::StartOriginalStage(
    const FString& OriginalLuaFilePath, const FString& OriginalSpawnTag,
    UGetchu3OriginalManagerProvider* SourceManager,
    const FString& ExpectedOriginalSha256) {
    if (!IsInGameThread()) {
        LastError = TEXT("Original Lua runtime may only execute on Unreal's game thread");
        return false;
    }
    if (bInOriginalManagerCall) {
        bDeferredOriginalStop = true;
        LastError = TEXT("Original stage start requested during a native manager callback");
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
    // StageId lookup checks disk once; execute must be bound to these exact
    // bytes, not a second file read that could have changed in between.
    if (ExpectedOriginalSha256.Len() != 64) {
        LastError = TEXT("Original stage script SHA-256 not supplied by verified ISO index");
        return false;
    }
#if PLATFORM_WINDOWS
    uint8 RecheckedDigest[32] = {};
    if (!ae3::originalSourceSha256(Chunk.GetData(),
                                  static_cast<size_t>(Chunk.Num()), RecheckedDigest)) {
        LastError = TEXT("Could not verify loaded original Lua script bytes");
        return false;
    }
    constexpr TCHAR ShaHex[] = TEXT("0123456789abcdef");
    FString ActualLoadedSha256;
    ActualLoadedSha256.Reserve(64);
    for (const uint8 Byte : RecheckedDigest) {
        ActualLoadedSha256.AppendChar(ShaHex[Byte >> 4]);
        ActualLoadedSha256.AppendChar(ShaHex[Byte & 15]);
    }
    if (!ActualLoadedSha256.Equals(ExpectedOriginalSha256, ESearchCase::IgnoreCase)) {
        LastError = TEXT("Original Lua source changed between stage lookup and execution");
        return false;
    }
#endif
#if !PLATFORM_WINDOWS
    LastError = TEXT("Source-backed original native Lua bridge is currently built for Windows only");
    return false;
#else
    const FString ProjectBridge = FPaths::Combine(
        FPaths::ProjectDir(), TEXT("Binaries"), TEXT("Win64"),
        TEXT("Getchu3OriginalLuaBridge.dll"));
    const FString PackagedBridge = FPaths::Combine(
        FPlatformProcess::BaseDir(), TEXT("Getchu3OriginalLuaBridge.dll"));
    const FString BridgePath = FPaths::FileExists(ProjectBridge)
        ? ProjectBridge : PackagedBridge;
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

    // The real managers must provide atomic stage setup. Failed Lua bindings
    // must never leave a partially constructed Player/Monkey world behind.
    Manager = SourceManager; // Retain through callbacks and partial-start rollback.
    bOriginalSetupOpen = true;
    bInOriginalManagerCall = true;
    const bool bBeganSourceTransaction =
        SourceManager->BeginOriginalStageSetup(OriginalLuaFilePath, OriginalSpawnTag);
    bInOriginalManagerCall = false;
    if (!bBeganSourceTransaction || bDeferredOriginalStop) {
        LastError = bDeferredOriginalStop
            ? TEXT("Original stage manager cancelled stage setup")
            : TEXT("Original actor/parameter manager does not support source-backed atomic stage setup");
        StopOriginalStage(); // Abort even if Begin partially allocated resources.
        return false;
    }
    Ae3LuaManagerV4 Bindings{};
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
    Bindings.read_original_player_progress = &CallbackReadOriginalProgress;

    bInOriginalManagerCall = true; // Creation queries original PlayerProgress.
    NativeState = Create(&Bindings);
    bInOriginalManagerCall = false;
    if (!NativeState || bDeferredOriginalStop) {
        LastError = bDeferredOriginalStop
            ? TEXT("Original source manager cancelled Lua VM initialization")
            : TEXT("Original Lua VM rejected manager bindings or failed to initialize");
        StopOriginalStage();
        return false;
    }
    const FTCHARToUTF8 NameUtf8(*OriginalLuaFilePath);
    const FTCHARToUTF8 TagUtf8(*OriginalSpawnTag);
    bInOriginalManagerCall = true;
    const bool bLuaLoaded = Execute(
        static_cast<Ae3LuaRuntime*>(NativeState), Chunk.GetData(),
        static_cast<size_t>(Chunk.Num()), NameUtf8.Get());
    const bool bSourceSetupRan = bLuaLoaded && !bDeferredOriginalStop &&
        SetupArea(static_cast<Ae3LuaRuntime*>(NativeState), TagUtf8.Get());
    bInOriginalManagerCall = false;
    if (!bLuaLoaded || !bSourceSetupRan || bDeferredOriginalStop) {
        LastError = bDeferredOriginalStop
            ? TEXT("Original game manager requested stop during native Lua execution")
            : Utf8String(Error(static_cast<Ae3LuaRuntime*>(NativeState)));
        if (LastError.IsEmpty()) LastError = TEXT("Original Lua 5.0 game initialization failed");
        UE_LOG(LogGetchu3OriginalLua, Error, TEXT("%s"), *LastError);
        StopOriginalStage();
        return false;
    }
    bInOriginalManagerCall = true;
    const bool bSourceCommitted = SourceManager->CommitOriginalStageSetup();
    bInOriginalManagerCall = false;
    if (!bSourceCommitted || bDeferredOriginalStop) {
        LastError = bDeferredOriginalStop
            ? TEXT("Original manager requested cancellation before stage commit completed")
            : TEXT("Original stage managers refused commit; original Lua stage setup was rolled back");
        StopOriginalStage();
        return false;
    }
    bOriginalSetupOpen = false;
    UE_LOG(LogGetchu3OriginalLua, Log,
        TEXT("Original source Lua stage and game manager transaction finished; original gameplay parity remains unverified."));
    return true;
#endif
}

void UGetchu3OriginalLuaSubsystem::StopOriginalStage() {
    if (bInOriginalManagerCall) {
        // An original Lua native function is still on the stack. Deferring
        // cleanup avoids freeing Lua state or unloading code being executed.
        bDeferredOriginalStop = true;
        return;
    }
    bDeferredOriginalStop = false;
    if (bOriginalSetupOpen) {
        // Clear ownership first: Abort itself can synchronously call Stop.
        bOriginalSetupOpen = false;
        if (IsValid(Manager)) {
            bInOriginalManagerCall = true;
            Manager->AbortOriginalStageSetup();
            bInOriginalManagerCall = false;
        }
    }
    if (NativeState && NativeLibrary) {
        const auto Destroy = GetOriginalLuaFunction<decltype(&ae3_lua_destroy)>(
            NativeLibrary, TEXT("ae3_lua_destroy"));
        if (!Destroy) {
            LastError = TEXT("Original native Lua DLL is missing its destruction export");
            bDeferredOriginalStop = true;
            return; // Never unload a DLL with a live VM.
        }
        bInOriginalManagerCall = true;
        const int bDestroyed = Destroy(static_cast<Ae3LuaRuntime*>(NativeState));
        bInOriginalManagerCall = false;
        if (bDestroyed != 1) {
            LastError = TEXT("Original Lua VM is still executing; DLL unload deferred");
            bDeferredOriginalStop = true;
            return; // Caller can safely retry StopOriginalStage after unwind.
        }
    }
    NativeState = nullptr;
    Manager = nullptr;
    if (NativeLibrary)
        FPlatformProcess::FreeDllHandle(NativeLibrary);
    NativeLibrary = nullptr;
    bDeferredOriginalStop = false;
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
