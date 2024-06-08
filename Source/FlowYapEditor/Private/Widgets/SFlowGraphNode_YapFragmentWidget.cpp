// Copyright Ghost Pepper Games, Inc. All Rights Reserved.

#include "Widgets/SFlowGraphNode_YapFragmentWidget.h"

#include "FlowYapColors.h"
#include "PropertyCustomizationHelpers.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "FlowYapEditorSubsystem.h"
#include "FlowYapInputTracker.h"
#include "FlowYapTransactions.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/Enums/FlowYapErrorLevel.h"
#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SSeparator.h"

#define LOCTEXT_NAMESPACE "FlowYap"

void SFlowGraphNode_YapFragmentWidget::Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, FFlowYapFragment* InFragment)
{
	Owner = InOwner;

	Fragment = InFragment;

	MoveFragmentButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("PropertyEditor.AssetComboStyle");

	MoveFragmentButtonStyle.Normal.TintColor = FlowYapColor::Noir_Trans;
	MoveFragmentButtonStyle.Hovered.TintColor = FlowYapColor::DarkGray;
	MoveFragmentButtonStyle.Pressed.TintColor = FlowYapColor::DarkGrayPressed;
	
	MoveFragmentButtonStyle.NormalForeground = FlowYapColor::LightGray;
	MoveFragmentButtonStyle.HoveredForeground = FlowYapColor::White;
	MoveFragmentButtonStyle.PressedForeground = FlowYapColor::LightGrayPressed;

	DeleteFragmentButtonStyle = MoveFragmentButtonStyle;
	DeleteFragmentButtonStyle.NormalForeground = FlowYapColor::LightRed;
	DeleteFragmentButtonStyle.HoveredForeground = FlowYapColor::LightRedHovered;
	DeleteFragmentButtonStyle.PressedForeground = FlowYapColor::LightRedPressed;
	
	ChildSlot
	[
		CreateDialogueContentArea()
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::PortraitImage_Visibility() const
{
	if (!Owner->GetIsSelected())
	{
		return EVisibility::Visible;
	}
	
	if (Owner->GetFocusedFragment().Get() == this)
	{
		return EVisibility::Collapsed;
	}

	return EVisibility::Visible;
}

EVisibility SFlowGraphNode_YapFragmentWidget::TitleText_Visibility() const
{
	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		return EVisibility::Visible;
	}
	
	return GetDefault<UFlowYapProjectSettings>()->GetHideTitleTextOnNPCDialogueNodes() ? EVisibility::Collapsed : EVisibility::Visible;
}

FReply SFlowGraphNode_YapFragmentWidget::OnClickDialogueTextBox()
{
	UE_LOG(LogTemp, Warning, TEXT("Hello World"));
	return FReply::Unhandled();
}

FText SFlowGraphNode_YapFragmentWidget::DialogueText_ToolTipText() const
{
	if (Owner->GetFocusedFragment().Get() == this)
	{
		return LOCTEXT("DialogueText_Tooltip", "To be displayed during speaking");
	}
	else
	{
		return Fragment->Bit.GetTitleText().IsEmptyOrWhitespace() ? LOCTEXT("DialogueText_Tooltip", "No title text") : FText::Format(LOCTEXT("DialogueText_Tooltip", "Title Text: {0}"), Fragment->Bit.GetTitleText());
	}
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDialogueContentArea()
{
	UClass* DialogueAssetClass = GetDefault<UFlowYapProjectSettings>()->GetDialogueAssetClass();

	if (!DialogueAssetClass)
	{
		DialogueAssetClass = UObject::StaticClass();
	}
	
	return SNew(SBox)
	.WidthOverride(this, &SFlowGraphNode_YapFragmentWidget::Fragment_WidthOverride)
	[
		SNew(SVerticalBox)
		// ===================
		// TOP (PORTRAIT | DIALOGUE)
		// ===================
		+ SVerticalBox::Slot()
		.Padding(0, 0, 0, 0)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			// ===================
			// DIALOGUE (LEFT SIDE)
			// ===================
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(1.0f)
			.Padding(0, 0, 0, 0)
			[
				SNew(SBox)
				.MinDesiredHeight(74) // This is the normal height of the full portrait widget
				.MaxDesiredHeight(this, &SFlowGraphNode_YapFragmentWidget::DialogueText_MaxDesiredHeight)
				.Padding(0, 0, 0, 0)
				[
					SAssignNew(DialogueBox, SMultiLineEditableTextBox)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::DialogueText_Text)
					.ModiferKeyForNewLine(EModifierKey::Shift)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::DialogueText_OnTextCommitted)
					.OverflowPolicy(ETextOverflowPolicy::Clip)
					.HintText(LOCTEXT("DialogueText_Hint", "Enter dialogue text"))
					//.ToolTipText(LOCTEXT("DialogueText_Tooltip", "To be displayed during speaking"))
					.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::DialogueText_ToolTipText)
					.Margin(FMargin(0,0,0,0))
					.Padding(3)
					.BackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::DialogueText_BackgroundColor)
					.ForegroundColor(this, &SFlowGraphNode_YapFragmentWidget::DialogueText_ForegroundColor)
				]
			]
			// ===================
			// PORTRAIT IMAGE (RIGHT SIDE)
			// ===================
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.AutoWidth()
			.Padding(0, 0, 0, 0)
			[
				CreatePortraitWidget()
			]
		]
		// ===================
		// BOTTOM (TITLE TEXT ABOVE, AUDIO | SETTINGS BELOW)
		// ===================
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(0, 0, 0, 0)
		[
			SNew(SBox)
			//.HeightOverride(52)
			[
				SNew(SVerticalBox)
				.Visibility(this, &SFlowGraphNode_YapFragmentWidget::FragmentLowerControls_Visibility)
				// ===================
				// TITLE TEXT
				// ===================
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 4, 0, 0)
				[
					SAssignNew(TitleTextBox, SEditableTextBox)
					.Visibility(this, &SFlowGraphNode_YapFragmentWidget::TitleText_Visibility)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::TitleText_Text)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::TitleText_OnTextCommitted)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.Padding(2)
					.HintText(LOCTEXT("TitleText_Hint", "Enter optional title text"))
					.ToolTipText(LOCTEXT("TitleText_Tooltip", "Title text may be used to build player's dialogue selection list."))
				]
				// ===================
				// BOTTOM CONTROLS
				// ===================
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2, 0, 0)
				[
					// ===================
					// AUDIO ASSET SELECTOR
					// ===================
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(0, 2, 0, 0)
					[
						SNew(SOverlay)
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SAssignNew(AudioAssetProperty, SObjectPropertyEntryBox)
							.DisplayBrowse(true)
							.DisplayUseSelected(false)
							.DisplayThumbnail(false)
							.AllowedClass(DialogueAssetClass)
							.EnableContentPicker(true)
							.ObjectPath(this, &SFlowGraphNode_YapFragmentWidget::DialogueAudioAsset_ObjectPath)
							.OnObjectChanged(this, &SFlowGraphNode_YapFragmentWidget::DialogueAudioAsset_OnObjectChanged)
							.ToolTipText(LOCTEXT("DialogueAudioAsset_Tooltip", "Select an audio asset."))
						]
						+ SOverlay::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SNew(SImage)
							.Image(FAppStyle::GetBrush("MarqueeSelection"))
							.Visibility(this, &SFlowGraphNode_YapFragmentWidget::DialogueAudioAssetErrorState_Visibility)
							.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::DialogueAudioErrorState_ColorAndOpacity)
						]
					]
					+ SHorizontalBox::Slot()
					.Padding(0, 0, 1, 0)
					.AutoWidth()
					[
						SNew(SSeparator)
						.SeparatorImage(FAppStyle::Get().GetBrush("Menu.Separator"))
						.Orientation(Orient_Vertical)
						.Thickness(1.0f)
						.ColorAndOpacity(FLinearColor::Gray)
					]
					// ===================
					// ADDITIONAL OPTIONS
					// ===================
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Right)
					.Padding(6,2,0,0)
					[
						CreateTimeSettingsWidget()
					]
				]
			]
		]
	];
}

FOptionalSize SFlowGraphNode_YapFragmentWidget::Fragment_WidthOverride() const
{
	return 360 + GetDefault<UFlowYapProjectSettings>()->GetDialogueWidthAdjustment();
}

FSlateColor SFlowGraphNode_YapFragmentWidget::DialogueText_ForegroundColor() const
{
	return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? FlowYapColor::White : FlowYapColor::LightGray;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::DialogueText_BackgroundColor() const
{
	return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? FlowYapColor::White : FlowYapColor::Noir;
}

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreatePortraitWidget()
{
	FLinearColor DialogueButtonsColor = FlowYapColor::Red;
	
	// TODO clean this up?
	return SNew(SBox)
	.Padding(0, 0, 0, 0)
	[
		SNew(SOverlay)
		.Visibility(this, &SFlowGraphNode_YapFragmentWidget::PortraitImage_Visibility)
		+ SOverlay::Slot()
		.Padding(4, 0, 0, 0)
		[
			SNew(SBox)
			.WidthOverride(74)
			.HeightOverride(74)
			[
				SNew(SBorder)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(2.0f)
				.BorderImage(FAppStyle::Get().GetBrush("PropertyEditor.AssetThumbnailBorder"))
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.Image(this, &SFlowGraphNode_YapFragmentWidget::PortraitImage_Image)
					]		
					+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.RenderTransformPivot(FVector2D(0.5, 0.5))
						.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-30.0f))))
						.Visibility(this, &SFlowGraphNode_YapFragmentWidget::MissingPortraitWarning_Visibility)
						.Text(LOCTEXT("FragmentCharacterMissing", "Missing"))
					]
				]
			]
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Left)
		.Padding(6, 0, 4, 2)
		[
			CreatePortraitKeySelector()
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		.Padding(0, 0, 2, 0)
		[
			// CONTROLS FOR UP/DELETE/DOWN
			SNew(SVerticalBox)
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::FragmentControlsBox_Visibility)
			// UP
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Center)
			.Padding(0, 2)
			[
				SNew(SButton)
				.ButtonStyle(&MoveFragmentButtonStyle)
				.ContentPadding(FMargin(3, 4))
				.ToolTipText(LOCTEXT("DialogueMoveFragmentUp_Tooltip", "Move Fragment Up"))
				.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::MoveFragmentUpButton_OnClicked)
				.Visibility(this, &SFlowGraphNode_YapFragmentWidget::MoveFragmentUpButton_Visibility)
				[
					SNew(SImage)
					.Image(FAppStyle::Get().GetBrush("Symbols.UpArrow"))
					.DesiredSizeOverride(FVector2D(12, 12))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
			// DELETE
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Padding(0, 0)
			[
				SNew(SButton)
				.ButtonStyle(&DeleteFragmentButtonStyle)
				.ContentPadding(FMargin(3, 4))
				.ToolTipText(LOCTEXT("DialogueDeleteFragment_Tooltip", "Delete Fragment"))
				.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::DeleteFragmentButton_OnClicked)
				.Visibility(this, &SFlowGraphNode_YapFragmentWidget::DeleteFragmentButton_Visibility)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Cross"))
					.DesiredSizeOverride(FVector2D(12, 12))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
			// DOWN
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Center)
			.Padding(0, 2)
			[
				SNew(SButton)
				.ButtonStyle(&MoveFragmentButtonStyle)
				.ContentPadding(FMargin(3, 4))
				.ToolTipText(LOCTEXT("DialogueMoveFragmentDown_Tooltip", "Move Fragment Down"))
				.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::MoveFragmentDownButton_OnClicked)
				.Visibility(this, &SFlowGraphNode_YapFragmentWidget::MoveFragmentDownButton_Visibility)
				[
					SNew(SImage)
					.Image(FAppStyle::Get().GetBrush("Symbols.DownArrow"))
					.DesiredSizeOverride(FVector2D(12, 12))
					.ColorAndOpacity(FSlateColor::UseForeground())
				]
			]
		]
	];
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::PortraitImage_Image() const
{
	return GetFlowYapDialogueNode()->GetSpeakerPortraitBrush(GetPortraitKey());
}

FSlateColor SFlowGraphNode_YapFragmentWidget::GetNodeTitleColor() const
{
	FLinearColor Color;

	if (GetFlowYapDialogueNode()->GetDynamicTitleColor(Color))
	{
		return Color;
	}

	return FLinearColor::Black;
}

EVisibility SFlowGraphNode_YapFragmentWidget::MissingPortraitWarning_Visibility() const
{
	FSlateBrush* Brush = GetFlowYapDialogueNode()->GetSpeakerPortraitBrush(GetPortraitKey());

	if (Brush)
	{
		return (Brush->GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
	}
	
	return EVisibility::Visible;
}

EVisibility SFlowGraphNode_YapFragmentWidget::PortraitKeySelector_Visibility() const
{
	return IsHovered() || bPortraitKeySelectorMenuOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

void SFlowGraphNode_YapFragmentWidget::PortraitKeySelector_OnMenuOpenChanged(bool bMenuOpen)
{
	bPortraitKeySelectorMenuOpen = bMenuOpen;
}

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreatePortraitKeySelector()
{
	TSharedPtr<SBox> Box;
	FMenuBuilder MenuBuilder(true, nullptr);
	FName SelectedPortraitKey = GetPortraitKey();

	for (const FName& PortraitKey : GetDefault<UFlowYapProjectSettings>()->GetPortraitKeys())
	{
		if (PortraitKey == NAME_None)
		{
			UE_LOG(FlowYap, Warning, TEXT("Warning: Portrait keys contains a 'NONE' entry. Clean this up!"));
			continue;
		}
		
		bool bSelected = PortraitKey == SelectedPortraitKey;
		MenuBuilder.AddWidget(CreatePortraitKeyMenuEntry(PortraitKey, bSelected), FText::GetEmpty());
	}

	TSharedPtr<SImage> PortraitIconImage;
	
	FString IconPath = GetDefault<UFlowYapProjectSettings>()->GetPortraitIconPath(GetPortraitKey());

	SAssignNew(Box, SBox)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::PortraitKeySelector_Visibility)
	[
		SNew(SComboButton)
		.HasDownArrow(false)
		.ContentPadding(FMargin(0.f, 0.f))
		.MenuPlacement(MenuPlacement_CenteredBelowAnchor)
		.ButtonColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.75f)))
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.OnMenuOpenChanged(this, &SFlowGraphNode_YapFragmentWidget::PortraitKeySelector_OnMenuOpenChanged)
		.ButtonContent()
		[
			SNew(SBox)
			.Padding(4, 4)
			[
				SAssignNew(PortraitIconImage, SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(this, &SFlowGraphNode_YapFragmentWidget::GetPortraitKeyBrush)
			]
		]
		.MenuContent()
		[
			MenuBuilder.MakeWidget()
		]
	];
	
	return Box.ToSharedRef();
}


TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreatePortraitKeyMenuEntry(FName InIconName, bool bSelected, const FText& InLabel, FName InTextStyle)
{
	const UFlowYapProjectSettings* ProjectSettings = GetDefault<UFlowYapProjectSettings>();
		
	TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);

	TSharedPtr<SImage> PortraitIconImage;
		
	FString IconPath = ProjectSettings->GetPortraitIconPath(InIconName);

	// TODO this is kind of uggers, can I maybe store FSlateIcons in the subsystem instead?
	UTexture2D* PortraitKeyIcon = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetPortraitKeyIcon(InIconName);
	
	FSlateBrush Brush;
	Brush.ImageSize = FVector2D(16, 16);
	Brush.SetResourceObject(PortraitKeyIcon);
		
	TSharedRef<FDeferredCleanupSlateBrush> PortraitKeyBrush = FDeferredCleanupSlateBrush::CreateBrush(Brush);
	
	if (!InIconName.IsNone())
	{
		HBox->AddSlot()
		.AutoWidth()
		.Padding(0, 0, 0, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SAssignNew(PortraitIconImage, SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
			.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([PortraitKeyBrush](){return PortraitKeyBrush->GetSlateBrush();})))
		];
	}
	
	if (!InLabel.IsEmpty())
	{
		HBox->AddSlot()	
		 .VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 0.f, 0.f)
		.AutoWidth()
		[
			SNew(STextBlock)
			.TextStyle( &FAppStyle::Get().GetWidgetStyle< FTextBlockStyle >( InTextStyle ))
			.Justification(ETextJustify::Center)
			.Text(InLabel)
		];
	}
	
	return SNew(SButton)
	.ContentPadding(FMargin(4, 4))
	.ButtonStyle(FAppStyle::Get(), "SimpleButton")
	.ButtonColorAndOpacity(FLinearColor(1,1,1,0.25))
	.ClickMethod(EButtonClickMethod::MouseDown)
	.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::HandlePortraitKeyChanged, InIconName)
	[
		SAssignNew(PortraitIconImage, SImage)
		.ColorAndOpacity(FSlateColor::UseForeground())
		.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([PortraitKeyBrush](){return PortraitKeyBrush->GetSlateBrush();})))
	];
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::GetPortraitKeyBrush() const
{
	return GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetPortraitKeyBrush(GetPortraitKey());
}

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateTimeSettingsWidget()
{
	TSharedPtr<SBox> Box;

	FSlateIcon ProjectSettingsIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");
	const FSlateBrush* ProjectSettingsIconBrush = ProjectSettingsIcon.GetIcon();
	
	TSharedRef<FDeferredCleanupSlateBrush> UseProjectDefaultsBrush = FDeferredCleanupSlateBrush::CreateBrush(*ProjectSettingsIconBrush);
	
	SAssignNew(Box, SBox)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0, 0, 1, 0)
		[
			// =============================
			// USE PROJECT DEFAULTS BUTTON
			// =============================
			SNew(SCheckBox)
			.Style(&UFlowYapEditorSubsystem::GetCheckBoxStyles().ToggleButtonCheckBox_Green)
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.ToolTipText(LOCTEXT("UseProjectDefaultTimeSettings_Tooltip", "Use time settings from project settings"))
			.IsEnabled(true)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_IsChecked)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_OnCheckStateChanged)
			.Content()
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([UseProjectDefaultsBrush](){return UseProjectDefaultsBrush->GetSlateBrush();})))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(1, 0, 1, 0)
		[
			// =============================
			// USE MANUAL TIME ENTRY BUTTON
			// =============================
			SNew(SCheckBox)
			.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.ToolTipText(LOCTEXT("UseEnteredTime_Tooltip", "Use a manually entered time"))
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_IsEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_IsChecked)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_OnCheckStateChanged)
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetTimerBrush())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(1, 0, 1, 0)
		[
			// =============================
			// USE TEXT TIME BUTTON
			// =============================
			SNew(SCheckBox)
			.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.ToolTipText(LOCTEXT("UseTimeFromText_Tooltip", "Use a time calculated from text length"))
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_IsEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_IsChecked)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_OnCheckStateChanged)
			[
				SNew(SBox)
				[
					SNew(SImage)
					.Image(GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetTextTimeBrush())
				]
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(1, 0, 1, 0)
		[
			// =============================
			// USE AUDIO TIME BUTTON
			// =============================
			SNew(SCheckBox)
			.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.ToolTipText(LOCTEXT("UseTimeFromAudio_Tooltip", "Use a time read from the audio asset"))
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsChecked)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_OnCheckStateChanged)
			.HAlign(HAlign_Center)
			[
				SNew(SImage)
				.Image(GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetAudioTimeBrush())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(3, 0, 1, 0)
		//.MaxWidth(73)
		[
			// =============================
			// TIME DISPLAY / MANUAL ENTRY FIELD
			// =============================
			SNew(SBox)
			.WidthOverride(73)
			.VAlign(VAlign_Fill)
			[
				SNew(SNumericEntryBox<double>)
				.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetEnabled_TimeEntryBox)
				.Delta(0.1)
				.MinValue(0.0)
				//.MinDesiredValueWidth(73) /* 8 pixels per side for border + 7 pixels per number... allow for 5 numbers */
				.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
				.Justification(ETextJustify::Center)
				.Value(this, &SFlowGraphNode_YapFragmentWidget::TimeEntryBox_Value)
				.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::TimeEntryBox_OnValueCommitted)
			]
		]
	];

	return Box.ToSharedRef(); 
}

bool SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_IsEnabled() const
{
	return Fragment->Bit.GetUseProjectDefaultTimeSettings() ? false : true;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_IsChecked() const
{
	return Fragment->Bit.GetTimeMode() == EFlowYapTimeMode::ManualTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		Fragment->Bit.SetBitTimeMode(EFlowYapTimeMode::ManualTime);
	}

	FFlowYapTransactions::EndModify();
}

bool SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_IsEnabled() const
{
	return Fragment->Bit.GetUseProjectDefaultTimeSettings() ? false : true;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_IsChecked() const
{
	return Fragment->Bit.GetTimeMode() == EFlowYapTimeMode::TextTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		Fragment->Bit.SetBitTimeMode(EFlowYapTimeMode::TextTime);
	}
	
	FFlowYapTransactions::EndModify();
}

FOptionalSize SFlowGraphNode_YapFragmentWidget::DialogueText_MaxDesiredHeight() const
{
	int16 Deadspace = 15;
	int16 LineHeight = 15;

	int16 UnfocusedLines = 4;
	int16 FocusedLines = 9;
	
	if (!DialogueBox.Get())
	{
		return Deadspace + UnfocusedLines * LineHeight;
	}
	
	if (DialogueBox->HasKeyboardFocus())
	{
		return Deadspace + FocusedLines * LineHeight;
	}

	return Deadspace + UnfocusedLines * LineHeight;
}

// -----------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapFragmentWidget::TitleText_Text() const
{
	return Fragment->Bit.GetTitleText();
}

void SFlowGraphNode_YapFragmentWidget::TitleText_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeTitleTextChanged", "Title Text Changed"), GetFlowYapDialogueNode());
	
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		Fragment->Bit.SetTitleText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

// -----------------------------------------------------------------------------------------------
FText SFlowGraphNode_YapFragmentWidget::DialogueText_Text() const
{
	return Fragment->Bit.GetDialogueText();
}

void SFlowGraphNode_YapFragmentWidget::DialogueText_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeDialogueTextChanged", "Dialogue Text Changed"), GetFlowYapDialogueNode());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		Fragment->Bit.SetDialogueText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

 // -----------------------------------------------------------------------------------------------
FString SFlowGraphNode_YapFragmentWidget::DialogueAudioAsset_ObjectPath() const
{
	const TSoftObjectPtr<UObject> Asset = Fragment->Bit.GetDialogueAudioAsset<UObject>();

	if (!Asset) { return ""; }

	return Asset.ToString();
}

void SFlowGraphNode_YapFragmentWidget::DialogueAudioAsset_OnObjectChanged(const FAssetData& InAssetData)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeAudioAssetChanged", "Audio Asset Changed"), GetFlowYapDialogueNode());

	Fragment->Bit.SetDialogueAudioAsset(InAssetData.GetAsset());

	FFlowYapTransactions::EndModify();
}

// -----------------------------------------------------------------------------------------------
FName SFlowGraphNode_YapFragmentWidget::GetPortraitKey() const
{
	return Fragment->Bit.GetPortraitKey();
}

FReply SFlowGraphNode_YapFragmentWidget::HandlePortraitKeyChanged(FName NewValue)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodePortraitKeyChanged", "Portrait Key Changed"), GetFlowYapDialogueNode());

	Fragment->Bit.SetPortraitKey(NewValue);

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

// -----------------------------------------------------------------------------------------------


void SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeTimedModeChanged", "Timed Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		Fragment->Bit.SetBitTimeMode(EFlowYapTimeMode::AudioTime);
	}
	
	FFlowYapTransactions::EndModify();
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_IsChecked() const
{
	return Fragment->Bit.GetUseProjectDefaultTimeSettings() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeUseProjectDefaultTimeSettings", "Use Project Default Time Settings Changed"), GetFlowYapDialogueNode());

	Fragment->Bit.SetUseProjectDefaultSettings(CheckBoxState == ECheckBoxState::Checked ? true : false);
	
	FFlowYapTransactions::EndModify();
}

// -----------------------------------------------------------------------------------------------
bool SFlowGraphNode_YapFragmentWidget::GetEnabled_TimeEntryBox() const
{
	return Fragment->Bit.GetTimeMode() == EFlowYapTimeMode::ManualTime;
}

bool SFlowGraphNode_YapFragmentWidget::InterruptibleButton_IsEnabled() const
{
	return Fragment->Bit.GetTimeMode() != EFlowYapTimeMode::None;
}

bool SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsEnabled() const
{
	return !Fragment->Bit.GetUseProjectDefaultTimeSettings();
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsChecked() const
{
	return Fragment->Bit.GetTimeMode() == EFlowYapTimeMode::AudioTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

// -----------------------------------------------------------------------------------------------
bool SFlowGraphNode_YapFragmentWidget::IsManualTimeEntryEnabled() const
{
	EFlowYapTimeMode TimeMode = Fragment->Bit.GetTimeMode();
	
	return TimeMode == EFlowYapTimeMode::ManualTime;
}

TOptional<double> SFlowGraphNode_YapFragmentWidget::TimeEntryBox_Value() const
{
	double Time = Fragment->Bit.GetTime();
	
	return (Time > 0) ? Time : TOptional<double>();
}

void SFlowGraphNode_YapFragmentWidget::TimeEntryBox_OnValueCommitted(double NewValue, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeEnteredTimeChanged", "Entered Time Changed"), GetFlowYapDialogueNode());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		Fragment->Bit.SetManualTime(NewValue);
	}
	else if (CommitType == ETextCommit::OnCleared)
	{
		Fragment->Bit.SetManualTime(0.0);
	}

	FFlowYapTransactions::EndModify();
}

bool SFlowGraphNode_YapFragmentWidget::GetEnabled_UseTextTimeButton() const
{
	return !Fragment->Bit.GetUseProjectDefaultTimeSettings();
}

EVisibility SFlowGraphNode_YapFragmentWidget::DialogueAudioAssetWarningState_Visibility() const
{
	return GetAudioErrorLevel() != EFlowYapErrorLevel::OK ? EVisibility::HitTestInvisible : EVisibility::Hidden;
}

EFlowYapErrorLevel SFlowGraphNode_YapFragmentWidget::GetAudioErrorLevel() const
{
	UClass* AssetClass = GetDefault<UFlowYapProjectSettings>()->GetDialogueAssetClass();

	static EFlowYapErrorLevel CachedErrorLevel = EFlowYapErrorLevel::OK;
	static double LastUpdateTime = 0;
	
	if (GWorld)
	{
		if (GWorld->GetRealTimeSeconds() - LastUpdateTime < 1.0)
		{
			return CachedErrorLevel;
		}		
	}
	
	const TSoftObjectPtr<UObject> Asset = Fragment->Bit.GetDialogueAudioAsset<UObject>();

	if (Asset)
	{
		if (!Asset->IsA(AssetClass))
		{
			CachedErrorLevel = EFlowYapErrorLevel::Error;
			return CachedErrorLevel;
		}
	}

	if (Fragment->Bit.GetTimeMode() == EFlowYapTimeMode::AudioTime && !Fragment->Bit.HasDialogueAudioAsset())
	{
		CachedErrorLevel = GetDefault<UFlowYapProjectSettings>()->GetMissingAudioErrorLevel();
		return CachedErrorLevel;
	}

	CachedErrorLevel = EFlowYapErrorLevel::OK;
	return CachedErrorLevel;
}

EVisibility SFlowGraphNode_YapFragmentWidget::DialogueAudioAssetErrorState_Visibility() const
{
	if (GetAudioErrorLevel() != EFlowYapErrorLevel::OK)
	{
		return EVisibility::HitTestInvisible;
	}
	
	return EVisibility::Hidden;
}

FReply SFlowGraphNode_YapFragmentWidget::MoveFragmentUpButton_OnClicked()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Move Fragment"), GetFlowYapDialogueNode());

	Owner->MoveFragmentUp(Fragment->IndexInDialogue);
	
	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

EVisibility SFlowGraphNode_YapFragmentWidget::MoveFragmentUpButton_Visibility() const
{
	return (Fragment->IndexInDialogue == 0) ? EVisibility::Hidden : EVisibility::Visible;		
}

FReply SFlowGraphNode_YapFragmentWidget::DeleteFragmentButton_OnClicked()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueDeleteFragment", "Delete Fragment"), GetFlowYapDialogueNode() /*GetFlowYapDialogueNode()*/);

	Owner->DeleteFragment(Fragment->IndexInDialogue);

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

FReply SFlowGraphNode_YapFragmentWidget::MoveFragmentDownButton_OnClicked()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Move Fragment"), GetFlowYapDialogueNode());

	Owner->MoveFragmentDown(Fragment->IndexInDialogue);
	
	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

EVisibility SFlowGraphNode_YapFragmentWidget::MoveFragmentDownButton_Visibility() const
{
	return (Fragment->IndexInDialogue == GetFlowYapDialogueNode()->GetNumFragments() - 1) ? EVisibility::Hidden : EVisibility::Visible;
}

EVisibility SFlowGraphNode_YapFragmentWidget::DeleteFragmentButton_Visibility() const
{
	if (GetFlowYapDialogueNode()->GetNumFragments() <= 1)
	{
		return EVisibility::Collapsed;
	}
	
	return EVisibility::Visible;
}

EVisibility SFlowGraphNode_YapFragmentWidget::FragmentControlsBox_Visibility() const
{
	return (IsHovered()) ? EVisibility::Visible : EVisibility::Collapsed;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::DialogueAudioErrorState_ColorAndOpacity() const
{
	switch (GetAudioErrorLevel())
	{
	case EFlowYapErrorLevel::OK:
		{
			return FlowYapColor::Green;
		}
	case EFlowYapErrorLevel::Warning:
		{
			return FlowYapColor::Orange;
		}
	case EFlowYapErrorLevel::Error:
		{
			return FlowYapColor::Red;
		}
	}

	return FlowYapColor::Black;
}

EVisibility SFlowGraphNode_YapFragmentWidget::FragmentLowerControls_Visibility() const
{
	// Always show if there are errors!
	// TODO: I am running GetAudioErrorLevel() multiple times, should I cache it?
	if (GetAudioErrorLevel() != EFlowYapErrorLevel::OK)
	{
		return EVisibility::Visible;
	}
	
	// Always show if the user is holding down CTRL and hovering over the dialogue!
	if (Owner->IsHovered() && bShiftPressed && Owner->GetKeyboardFocusedFragmentWidget().Get() == this)
	{
		//return EVisibility::Visible;
	}
	
	if (!Owner->GetIsSelected())
	{
		return EVisibility::Collapsed;
	}
	
	if (Owner->GetFocusedFragment().Get() == this)
	{
		return EVisibility::Visible;
	}

	if (Owner->GetControlHooked())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

// -----------------------------------------------------------------------------------------------
UFlowNode_YapDialogue* SFlowGraphNode_YapFragmentWidget::GetFlowYapDialogueNode() const
{
	return Owner->GetFlowYapDialogueNode();
}

void SFlowGraphNode_YapFragmentWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	bShiftPressed = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetInputTracker()->GetShiftPressed();
	bControlPressed = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetInputTracker()->GetControlPressed();

	if (DialogueBox->HasKeyboardFocus() || TitleTextBox->HasKeyboardFocus())
	{
		Owner->SetFocusedFragment(Fragment->IndexInDialogue);
		Owner->SetTypingFragment(Fragment->IndexInDialogue);
	}
	else
	{
		Owner->ClearTypingFragment(Fragment->IndexInDialogue);
	}
}
#undef LOCTEXT_NAMESPACE
