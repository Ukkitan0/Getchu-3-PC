#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "IPythonScriptPlugin.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "ToolMenus.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/DragAndDrop.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "Getchu3Importer"

namespace {
const FName TabId(TEXT("Getchu3Importer"));

FString PythonLiteral(FString Value)
{
    // Paths are data, never evaluated as Python source.
    Value.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
    Value.ReplaceInline(TEXT("'"), TEXT("\\'"));
    Value.ReplaceInline(TEXT("\r"), TEXT("\\r"));
    Value.ReplaceInline(TEXT("\n"), TEXT("\\n"));
    return TEXT("'") + Value + TEXT("'");
}

class SGetchu3ImporterPanel final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SGetchu3ImporterPanel) {} SLATE_END_ARGS()

    void Construct(const FArguments&)
    {
        const FLinearColor Panel(0.13f, 0.13f, 0.13f);
        ChildSlot[
            SNew(SBorder).BorderBackgroundColor(FLinearColor::Black).Padding(24)
            [
                SNew(SScrollBox)
                + SScrollBox::Slot()[
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 22)[
                        SNew(STextBlock).Text(LOCTEXT("Title", "Ape Escape 3 — Unreal Importer"))
                            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 21))
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)[
                        SNew(STextBlock).Text(LOCTEXT("Source", "ORIGINAL GAME SOURCE"))
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 18)[
                        SNew(SBorder).BorderBackgroundColor(Panel).Padding(16)[
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[
                                SNew(STextBlock).Text_Lambda([this]() {
                                    return FText::FromString(SelectedIso.IsEmpty()
                                        ? TEXT("Select Ape Escape 3 (USA).iso")
                                        : SelectedIso);
                                })
                            ]
                            + SHorizontalBox::Slot().AutoWidth()[
                                SNew(SButton).Text(LOCTEXT("ChooseIso", "Browse ISO..."))
                                    .OnClicked(this, &SGetchu3ImporterPanel::Browse)
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 14)[
                        SNew(STextBlock).Text(LOCTEXT("Extract", "↓  Automatic extraction and conversion"))
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 14)[
                        SNew(SUniformGridPanel).SlotPadding(FMargin(5))
                        + SUniformGridPanel::Slot(0, 0)[MakeTile(TEXT("Models & textures"))]
                        + SUniformGridPanel::Slot(1, 0)[MakeTile(TEXT("Levels & collision"))]
                        + SUniformGridPanel::Slot(0, 1)[MakeTile(TEXT("Lua & game logic"))]
                        + SUniformGridPanel::Slot(1, 1)[MakeTile(TEXT("Animation & audio"))]
                    ]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 14)[
                        SNew(STextBlock).Text(LOCTEXT("Setup", "↓  Unreal asset conversion and original gameplay integration"))
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 14)[
                        SNew(SBorder).BorderBackgroundColor(Panel).Padding(FMargin(20, 28))[
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)[
                                SNew(STextBlock).Text(LOCTEXT("StatusHeading", "Original game integration"))
                                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 17))
                            ]
                            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 10)[
                                SNew(STextBlock).AutoWrapText(true).Text_Lambda([this]() {
                                    return FText::FromString(Status);
                                })
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)[
                        SNew(SButton).HAlign(HAlign_Center)
                            .IsEnabled_Lambda([this]() { return !SelectedIso.IsEmpty() && !bImporting; })
                            .OnClicked(this, &SGetchu3ImporterPanel::Import)
                            [SNew(STextBlock).Text(LOCTEXT("ImportGame", "Import Original ISO"))]
                    ]
                    + SVerticalBox::Slot().AutoHeight()[
                        SNew(SButton).HAlign(HAlign_Center).IsEnabled(false)
                            [SNew(STextBlock).Text(LOCTEXT("Play", "▷  Play Ape Escape 3"))]
                    ]
                ]
            ]
        ];
    }

    virtual FReply OnDrop(const FGeometry&, const FDragDropEvent& Event) override
    {
        TSharedPtr<FExternalDragOperation> Operation = Event.GetOperationAs<FExternalDragOperation>();
        if (!Operation.IsValid() || bImporting)
            return FReply::Unhandled();
        for (const FString& Candidate : Operation->GetFiles())
            if (FPaths::GetExtension(Candidate).Equals(TEXT("iso"), ESearchCase::IgnoreCase))
            {
                SelectedIso = Candidate;
                Status = TEXT("Disc selected. Import will verify the USA release before extracting data.");
                return FReply::Handled();
            }
        return FReply::Unhandled();
    }

    virtual FReply OnDragOver(const FGeometry&, const FDragDropEvent& Event) override
    {
        return Event.GetOperationAs<FExternalDragOperation>().IsValid()
            ? FReply::Handled() : FReply::Unhandled();
    }

private:
    FString SelectedIso;
    FString Status = TEXT("Not ready: original gameplay runtime is not implemented. Select an owned USA ISO to import available assets.");
    bool bImporting = false;

    static TSharedRef<SWidget> MakeTile(const FString& Label)
    {
        return SNew(SBorder).BorderBackgroundColor(FLinearColor(0.20f, 0.20f, 0.20f)).Padding(12)
            [SNew(STextBlock).Text(FText::FromString(Label))];
    }

    FReply Browse()
    {
        IDesktopPlatform* Desktop = FDesktopPlatformModule::Get();
        if (!Desktop) { Status = TEXT("Desktop file picker unavailable."); return FReply::Handled(); }
        TArray<FString> Files;
        const void* Parent = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
        if (Desktop->OpenFileDialog(Parent, TEXT("Choose original Ape Escape 3 (USA) ISO"),
                FString(), FString(), TEXT("ISO images|*.iso"), 0, Files) && Files.Num() == 1)
        {
            SelectedIso = Files[0];
            Status = TEXT("Disc selected. Import validates ISO SHA-256 before reading resources.");
        }
        return FReply::Handled();
    }

    FReply Import()
    {
        if (bImporting || !FPaths::FileExists(SelectedIso))
        {
            Status = TEXT("No existing ISO was selected.");
            return FReply::Handled();
        }
        const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("Getchu3Importer"));
        if (!Plugin.IsValid()) { Status = TEXT("Getchu3Importer plugin files unavailable."); return FReply::Handled(); }
        const FString ScriptFolder = Plugin->GetContentDir() / TEXT("Python");
        const FString ScriptFile = ScriptFolder / TEXT("getchu3_importer.py");
        if (!FPaths::FileExists(ScriptFile)) { Status = TEXT("Original import script missing."); return FReply::Handled(); }
        bImporting = true;
        Status = TEXT("Verifying original disc and importing assets; see Unreal Output Log.");
        // PythonScriptPlugin executes on the editor thread so AssetTools can
        // manipulate editor packages; no UObjects are touched by worker threads.
        const FString Command = FString::Printf(TEXT("import sys; sys.path.insert(0, %s); import getchu3_importer; getchu3_importer.run(%s)"),
            *PythonLiteral(ScriptFolder), *PythonLiteral(SelectedIso));
        IPythonScriptPlugin* PythonPlugin = IPythonScriptPlugin::Get();
        if (!PythonPlugin || !PythonPlugin->IsPythonInitialized())
        {
            bImporting = false;
            Status = TEXT("Unreal Editor Python is unavailable. Enable Python Script Plugin and reopen the Editor.");
            return FReply::Handled();
        }
        const bool bOkay = PythonPlugin->ExecPythonCommand(*Command);
        bImporting = false;
        const FString ReportFile = FPaths::ProjectSavedDir() / TEXT("Getchu3Private/Getchu3ImportStatus.json");
        FString Json;
        TSharedPtr<FJsonObject> Report;
        if (FFileHelper::LoadFileToString(Json, *ReportFile) &&
            FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Report) && Report.IsValid())
        {
            const FString Result = Report->GetStringField(TEXT("status"));
            if (Result == TEXT("original_models_textures_streams_i3c_imported_gameplay_not_implemented") && bOkay)
                Status = TEXT("Original model, texture, stream and I3C static meshes imported. Gameplay collision responses, Lua, actors, gadgets and live sound are still incomplete.");
            else
            {
                if (!bOkay && Result != TEXT("blocked"))
                {
                    Status = TEXT("Import failed before a current status report was completed; check Output Log.");
                    return FReply::Handled();
                }
                FString Error;
                Report->TryGetStringField(TEXT("error"), Error);
                Status = TEXT("Import blocked: ") + (Error.IsEmpty() ? Result : Error);
            }
        }
        else
            Status = TEXT("Import failed and no valid status report was written. Check Output Log.");
        return FReply::Handled();
    }
};
}

class FGetchu3ImporterModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabId,
            FOnSpawnTab::CreateRaw(this, &FGetchu3ImporterModule::Spawn))
            .SetDisplayName(LOCTEXT("TabTitle", "Ape Escape 3 — Unreal Importer"))
            .SetMenuType(ETabSpawnerMenuType::Hidden);
        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(
            this, &FGetchu3ImporterModule::RegisterMenu));
    }

    virtual void ShutdownModule() override
    {
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);
    }

private:
    TSharedRef<SDockTab> Spawn(const FSpawnTabArgs&)
    {
        return SNew(SDockTab).TabRole(ETabRole::NomadTab)[SNew(SGetchu3ImporterPanel)];
    }

    void RegisterMenu()
    {
        FToolMenuOwnerScoped Owner(this);
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
        FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("Getchu3Tools"));
        Section.AddMenuEntry(TEXT("Getchu3ImporterOpen"),
            LOCTEXT("OpenLabel", "Ape Escape 3 — Unreal Importer"),
            LOCTEXT("OpenTooltip", "Import owner-local original ISO content (not playable gameplay)."),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([] {
                FGlobalTabmanager::Get()->TryInvokeTab(TabId);
            })));
    }
};

IMPLEMENT_MODULE(FGetchu3ImporterModule, Getchu3Importer)
#undef LOCTEXT_NAMESPACE
