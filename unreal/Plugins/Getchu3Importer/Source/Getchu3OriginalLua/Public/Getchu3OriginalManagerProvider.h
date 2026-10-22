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
UCLASS(Abstract, BlueprintType, Blueprintable)
class GETCHU3ORIGINALLUA_API UGetchu3OriginalManagerProvider : public UObject
{
    GENERATED_BODY()

public:
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

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Ape Escape 3|Original Manager")
    bool SelectOriginalPlayerWeapon(int32 OriginalSelection);
    virtual bool SelectOriginalPlayerWeapon_Implementation(int32);
};
