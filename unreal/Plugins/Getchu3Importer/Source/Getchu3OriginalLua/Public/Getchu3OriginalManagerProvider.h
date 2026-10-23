#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Getchu3OriginalManagerProvider.generated.h"

/**
 * Unreal-facing contract for the TRUE original Ape Escape 3 native managers.
 *
 * Hex strings carry exact original Lua bytes; typed JSON values preserve
 * original Lua type/number/table structure, not invented gameplay parameters.
 * Return true ONLY after the actual original manager performs the operation.
 * The default implementations reject all operations, deliberately.
 *
 * The original Lua 5.0 runtime calls these synchronously on the game thread.
 * Do not implement a generic AActor substitute as if it were the original.
 */
/** A recovered VOID-return game callback identified by genuine executable VA.
 * Never list unknown or value-return functions here. The game-manager
 * implementation must separately recover its original semantics.
 */
USTRUCT(BlueprintType)
struct GETCHU3ORIGINALLUA_API FGetchu3OriginalVoidFunction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Original Source")
    FString OriginalName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Original Source")
    int32 OriginalVA = 0;
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class GETCHU3ORIGINALLUA_API UGetchu3OriginalManagerProvider : public UObject
{
    GENERATED_BODY()

public:
    // A stage must not leak partially spawned actors or game flag changes
    // when a later original Lua native binding is unavailable.
    // Only genuinely atomic source-backed managers may return true.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool BeginOriginalStageSetup(const FString& OriginalLuaPath,
                                 const FString& OriginalSpawnTag);
    virtual bool BeginOriginalStageSetup_Implementation(
        const FString&, const FString&);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool CommitOriginalStageSetup();
    virtual bool CommitOriginalStageSetup_Implementation();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    void AbortOriginalStageSetup();
    virtual void AbortOriginalStageSetup_Implementation();

    /** Return only independently source-verified, genuinely implemented
     * void-return Lua callbacks. A CreateBind JAL location is NOT a callback
     * VA. All results are checked by LuaStage against existing globals.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool CollectVerifiedOriginalVoidFunctions(
        UPARAM(ref) TArray<FGetchu3OriginalVoidFunction>& OutFunctions);
    virtual bool CollectVerifiedOriginalVoidFunctions_Implementation(
        TArray<FGetchu3OriginalVoidFunction>& OutFunctions);

    /** Invoke actual recovered original C++ behavior; false aborts script. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool InvokeRecoveredOriginalVoidFunction(
        int32 OriginalVA, const FString& OriginalName,
        const FString& OriginalTypedArgumentsJson);
    virtual bool InvokeRecoveredOriginalVoidFunction_Implementation(
        int32, const FString&, const FString&);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool CreateOriginalEntity(const FString& ClassHex, const FString& NameHex,
                              const FString& TagHex, const FString& FieldsTypedJson,
                              const FString& ControllersTypedJson);
    virtual bool CreateOriginalEntity_Implementation(
        const FString&, const FString&, const FString&, const FString&, const FString&);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool SubmitOriginalParameters(const FString& ParametersTypedJson);
    virtual bool SubmitOriginalParameters_Implementation(const FString&);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool WriteOriginalFlag(const FString& ScopeHex, const FString& KeyHex, bool Value);
    virtual bool WriteOriginalFlag_Implementation(const FString&, const FString&, bool);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool ReadOriginalFlag(const FString& ScopeHex, const FString& KeyHex, bool& OutValue);
    virtual bool ReadOriginalFlag_Implementation(const FString&, const FString&, bool& OutValue);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool ReadOriginalStageInteger(const FString& ArgumentsTypedJson, int32& OutValue);
    virtual bool ReadOriginalStageInteger_Implementation(const FString&, int32& OutValue);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool ReadOriginalSurvivalMode(bool& OutValue);
    virtual bool ReadOriginalSurvivalMode_Implementation(bool& OutValue);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool ReadOriginalPlayerProgress(UPARAM(ref) TArray<uint8>& PackedSourceBytes);
    virtual bool ReadOriginalPlayerProgress_Implementation(TArray<uint8>& PackedSourceBytes);

    // Must atomically replace the exact 1033-byte source-backed player state.
    // A Blueprint or C++ handler that only stores a mock value is not valid.
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool WriteOriginalPlayerProgress(const TArray<uint8>& PackedSourceBytes);
    virtual bool WriteOriginalPlayerProgress_Implementation(
        const TArray<uint8>& PackedSourceBytes);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool SelectOriginalPlayerWeapon(int32 OriginalSelection);
    virtual bool SelectOriginalPlayerWeapon_Implementation(int32);
};
