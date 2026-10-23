#include "Getchu3OriginalManagerProvider.h"

// Original game manager transactions are mandatory, not visual/UI stubs.
// They must stage changes until commit and undo every pending operation on abort.
bool UGetchu3OriginalManagerProvider::BeginOriginalStageSetup_Implementation(
    const FString&, const FString&) { return false; }
bool UGetchu3OriginalManagerProvider::CommitOriginalStageSetup_Implementation()
{ return false; }
void UGetchu3OriginalManagerProvider::AbortOriginalStageSetup_Implementation() {}

// The original game manager MUST supply source-recovered implementations.
// Deliberately no automatic Unreal actor factory, synthetic stage parameters,
// invented progression values, or plausible-but-incorrect gadget behavior.
bool UGetchu3OriginalManagerProvider::CollectVerifiedOriginalVoidFunctions_Implementation(
    TArray<FGetchu3OriginalVoidFunction>& OutFunctions)
{ OutFunctions.Reset(); return false; }

bool UGetchu3OriginalManagerProvider::InvokeRecoveredOriginalVoidFunction_Implementation(
    int32, const FString&, const FString&) { return false; }

bool UGetchu3OriginalManagerProvider::CreateOriginalEntity_Implementation(
    const FString&, const FString&, const FString&, const FString&, const FString&)
{ return false; }

bool UGetchu3OriginalManagerProvider::SubmitOriginalParameters_Implementation(
    const FString&) { return false; }

bool UGetchu3OriginalManagerProvider::WriteOriginalFlag_Implementation(
    const FString&, const FString&, bool) { return false; }

bool UGetchu3OriginalManagerProvider::ReadOriginalFlag_Implementation(
    const FString&, const FString&, bool& OutValue)
{ OutValue = false; return false; }

bool UGetchu3OriginalManagerProvider::ReadOriginalStageInteger_Implementation(
    const FString&, int32& OutValue)
{ OutValue = 0; return false; }

bool UGetchu3OriginalManagerProvider::ReadOriginalSurvivalMode_Implementation(
    bool& OutValue)
{ OutValue = false; return false; }

bool UGetchu3OriginalManagerProvider::ReadOriginalPlayerProgress_Implementation(
    TArray<uint8>& PackedSourceBytes)
{ PackedSourceBytes.Reset(); return false; }

bool UGetchu3OriginalManagerProvider::WriteOriginalPlayerProgress_Implementation(
    const TArray<uint8>&) { return false; }

bool UGetchu3OriginalManagerProvider::SelectOriginalPlayerWeapon_Implementation(
    int32) { return false; }
