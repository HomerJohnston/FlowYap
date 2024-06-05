// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "FlowYap/FlowYapProjectSettings.h"

#include "FlowYap/FlowYapEngineUtils.h"

#include "FlowYap/FlowYapTextCalculator.h"

#define LOCTEXT_NAMESPACE "FlowYap"

UFlowYapProjectSettings::UFlowYapProjectSettings()
{
	PortraitKeyIconPath.Path = "";

	DefaultTimeModeSetting = EFlowYapTimeMode::AudioTime;
	
	AudioTimeFallbackTimedMode = EFlowYapTimeMode::TextTime;

	bDefaultInterruptibleSetting = true;
	
	DialogueWidthAdjustment = 0;

	TextWordsPerMinute = 120;
	
	MinimumAutoTextTimeLength = 2.5;

	MinimumAutoAudioTimeLength = 1.0;

	TextCalculator = UFlowYapTextCalculator::StaticClass();

	AudioTimeCacher = nullptr; // UFlowYapAudioTimeCacher::StaticClass(); // You must create your own class and calculate time in it!
}

#if WITH_EDITOR
FString UFlowYapProjectSettings::GetPortraitIconPath(FName Key) const
{
	if (PortraitKeyIconPath.Path == "")
	{
		return FFlowYapEngineUtils::GetFlowYapPluginDir() / FString::Format(TEXT("Resources/PortraitKeys/{1}.png"), { PortraitKeyIconPath.Path, Key.ToString() });
	}
	
	return FPaths::ProjectDir() / FString::Format(TEXT("{0}/{1}.png"), { PortraitKeyIconPath.Path, Key.ToString() });
}

const TArray<FName>& UFlowYapProjectSettings::GetPortraitKeys() const
{
	return PortraitKeys;
}

void UFlowYapProjectSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = PropertyChangedEvent.Property->GetFName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, PortraitKeys))
	{
		OnPortraitKeysChanged.Broadcast();
	}
}

EFlowYapTimeMode UFlowYapProjectSettings::GetMissingAudioFallbackTimeMode() const
{
	return AudioTimeFallbackTimedMode;
}

UClass* UFlowYapProjectSettings::GetDialogueAssetClass() const
{
	return DialogueAssetClass;
}

int32 UFlowYapProjectSettings::GetDialogueWidthAdjustment() const
{
	return DialogueWidthAdjustment;
}

bool UFlowYapProjectSettings::GetHideTitleTextOnNPCDialogueNodes() const
{
	return bHideTitleTextOnNPCDialogueNodes;
}

int32 UFlowYapProjectSettings::GetTextWordsPerMinute() const
{
	return TextWordsPerMinute;
}

double UFlowYapProjectSettings::GetMinimumAutoTextTimeLength() const
{
	return MinimumAutoTextTimeLength;
}

double UFlowYapProjectSettings::GetMinimumAutoAudioTimeLength() const
{
	return MinimumAutoAudioTimeLength;
}
#endif

#undef LOCTEXT_NAMESPACE