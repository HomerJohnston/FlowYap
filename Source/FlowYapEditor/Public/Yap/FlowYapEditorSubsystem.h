#pragma once

#include "Textures/SlateIcon.h"

#include "FlowYapEditorSubsystem.generated.h"

class UFlowYapCharacter;
class FFlowYapInputTracker;

struct FCheckBoxStyles
{
	// Generic check boxes
	FCheckBoxStyle ToggleButtonCheckBox_Red;
	FCheckBoxStyle ToggleButtonCheckBox_Green;
	FCheckBoxStyle ToggleButtonCheckBox_Blue;
	FCheckBoxStyle ToggleButtonCheckBox_Orange;
	FCheckBoxStyle ToggleButtonCheckBox_White;
	FCheckBoxStyle ToggleButtonCheckBox_Transparent;

	// Custom check boxes
	FCheckBoxStyle ToggleButtonCheckBox_PlayerPrompt;
	FCheckBoxStyle ToggleButtonCheckBox_DialogueInterrupt;
};

UCLASS()
class UFlowYapEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	TMap<FName, UTexture2D*> MoodKeyIconTextures;
	TMap<FName, TSharedPtr<FSlateBrush>> MoodKeyIconBrushes; // TODO is this safe? I fucking suck at memory management

protected:
	static FCheckBoxStyles CheckBoxStyles;

	// STATE
	TSharedPtr<FFlowYapInputTracker> InputTracker;
	
public:
	void UpdateMoodKeyIconsMap();
	
	UTexture2D* GetMoodKeyIcon(FName MoodKey);

	const FSlateBrush* GetMoodKeyBrush(FName Name);

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;

	void Deinitialize() override;
	
	void LoadIcon(FString LocalResourcePath, UTexture2D*& Texture, FSlateBrush& Brush, int32 XYSize = 16);
	
	static const FCheckBoxStyles& GetCheckBoxStyles();

	FFlowYapInputTracker* GetInputTracker();
};