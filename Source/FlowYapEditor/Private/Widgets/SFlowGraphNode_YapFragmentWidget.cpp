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
#include "SGameplayTagWidget.h"
#include "FlowYap/FlowYapCharacter.h"
#include "FlowYap/FlowYapLog.h"
#include "FlowYap/Enums/FlowYapErrorLevel.h"
#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "SGameplayTagCombo.h"
#include "GameplayTagsManager.h"
#include "Widgets/SGameplayTagComboFiltered.h"
#include "IPropertyUtilities.h"
#include "FlowYap/FlowYapUtil.h"

#define LOCTEXT_NAMESPACE "FlowYap"
 
FButtonStyle SFlowGraphNode_YapFragmentWidget::MoveFragmentButtonStyle;
bool SFlowGraphNode_YapFragmentWidget::bStylesInitialized = false;

void SFlowGraphNode_YapFragmentWidget::Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, uint8 InFragmentIndex)
{
	Owner = InOwner;
	FragmentIndex = InFragmentIndex;
	
	if (!bStylesInitialized)
	{
		MoveFragmentButtonStyle = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("PropertyEditor.AssetComboStyle");

		// Button colors
		MoveFragmentButtonStyle.Normal.TintColor = FlowYapEditor::Color::Noir_Trans;
		MoveFragmentButtonStyle.Hovered.TintColor = FlowYapEditor::Color::DarkGray_Trans;
		MoveFragmentButtonStyle.Pressed.TintColor = FlowYapEditor::Color::DarkGrayPressed_Trans;

		// Text colors
		MoveFragmentButtonStyle.NormalForeground = FlowYapEditor::Color::LightGray;
		MoveFragmentButtonStyle.HoveredForeground = FlowYapEditor::Color::White;
		MoveFragmentButtonStyle.PressedForeground = FlowYapEditor::Color::LightGrayPressed;

		bStylesInitialized = true;
	}

	// Find the FragmentTag property
	GetFlowYapDialogueNode()->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UFlowNode_YapDialogue, Fragments));
	
	ChildSlot
	[
		CreateFragmentWidget()
	];
}

// ================================================================================================
// FRAGMENT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentWidget()
{
	return SNew(SBox)
	.WidthOverride(this, &SFlowGraphNode_YapFragmentWidget::Fragment_WidthOverride)
	[
		SNew(SVerticalBox)
		// ===================
		// TOP SECTION
		// ===================
		+ SVerticalBox::Slot()
		.Padding(0, 0, 0, 0)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			// -------------------
			// DIALOGUE
			// -------------------
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(1.0f)
			.Padding(0, 0, 0, 0)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					CreateDialogueWidget()
				]
				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				.Padding(3, 0, 3, 2)
				[
					CreateFragmentTagPreviewWidget()
				]
			]
			// -------------------
			// PORTRAIT
			// -------------------
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
		// BOTTOM SECTION
		// ===================
		+ SVerticalBox::Slot()
		.AutoHeight()
		//.HAlign(HAlign_Fill)
		.Padding(0, 0, 0, 0)
		[
			SNew(SVerticalBox)
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::FragmentBottomSection_Visibility)
			//.Visibility(EVisibility::Visible)
			// -------------------
			// MIDDLE ROW
			// -------------------
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.Padding(0, 4, -1, 0)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.Padding(0, 0, 2, 0)
				[
					CreateTitleTextWidget()
				]
				
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.Padding(2, 0, 0, 0)
				.AutoWidth()
				[
					CreateFragmentTagWidget()
				]
			]
			// -------------------
			// BOTTOM ROW
			// -------------------
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 2, -1, 0)
			[
				CreateBottomRowWidget()
			]
		]
	];
}

FOptionalSize SFlowGraphNode_YapFragmentWidget::Fragment_WidthOverride() const
{
	return 360 + UFlowYapProjectSettings::Get()->GetDialogueWidthAdjustment();
}

EVisibility SFlowGraphNode_YapFragmentWidget::FragmentBottomSection_Visibility() const
{
	// Always show if there are errors!
	if (AudioAssetErrorLevel() != EFlowYapErrorLevel::OK)
	{
		return EVisibility::Visible;
	}
	
	if (FragmentFocused())
	{
		return EVisibility::Visible;
	}
	
	if (!Owner->GetIsSelected())
	{
		return EVisibility::Collapsed;
	}

	if (Owner->GetShiftHooked())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Collapsed;
}

// ================================================================================================
// DIALOGUE WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateDialogueWidget()
{
	TSharedRef<SScrollBar> HScrollBar = SNew(SScrollBar)
	.Orientation(Orient_Horizontal)
	.Thickness(this, &SFlowGraphNode_YapFragmentWidget::DialogueScrollBar_Thickness)
	.Padding(-1);
	
	TSharedRef<SScrollBar> VScrollBar = SNew(SScrollBar)
	.Orientation(Orient_Vertical)
	.Thickness(this, &SFlowGraphNode_YapFragmentWidget::DialogueScrollBar_Thickness)
	.Padding(-1);
	
	return SNew(SBox)
	.MinDesiredHeight(58) // This is the normal height of the full portrait widget
	.MaxDesiredHeight(this, &SFlowGraphNode_YapFragmentWidget::Dialogue_MaxDesiredHeight)
	.Padding(0, 0, 0, 0)
	[
		SAssignNew(DialogueBox, SMultiLineEditableTextBox)
		.Text(this, &SFlowGraphNode_YapFragmentWidget::Dialogue_Text)
		.ModiferKeyForNewLine(EModifierKey::Shift)
		.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::Dialogue_OnTextCommitted)
		.OverflowPolicy(ETextOverflowPolicy::Clip)
		.HintText(LOCTEXT("DialogueText_Hint", "Enter dialogue text"))
		.ToolTipText(this, &SFlowGraphNode_YapFragmentWidget::Dialogue_ToolTipText)
		.Margin(FMargin(0,0,0,0))
		.Padding(3)
		.BackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::Dialogue_BackgroundColor)
		.ForegroundColor(this, &SFlowGraphNode_YapFragmentWidget::Dialogue_ForegroundColor)
		.HScrollBar(HScrollBar)
		.VScrollBar(VScrollBar)
	];
}

FVector2D SFlowGraphNode_YapFragmentWidget::DialogueScrollBar_Thickness() const
{
	if (FragmentFocused())
	{
		return FVector2D(8, 8);
	}
	
	return FVector2D(0, 0);
}

FOptionalSize SFlowGraphNode_YapFragmentWidget::Dialogue_MaxDesiredHeight() const
{
	if (FragmentFocused())
	{
		int16 DeadSpace = 15;
		int16 LineHeight = 15;
		int16 FocusedLines = 9;
	
		return DeadSpace + FocusedLines * LineHeight;
	}

	return 58; // TODO fluctuate with portrait widget height
}

FText SFlowGraphNode_YapFragmentWidget::Dialogue_Text() const
{
	return GetFragment()->Bit.GetDialogueText();
}

void SFlowGraphNode_YapFragmentWidget::Dialogue_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeDialogueTextChanged", "Dialogue Text Changed"), GetFlowYapDialogueNode());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetFragment()->Bit.SetDialogueText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

FText SFlowGraphNode_YapFragmentWidget::Dialogue_ToolTipText() const
{
	uint8 FocusedFragmentIndex;
	if (Owner->GetFocusedFragmentIndex(FocusedFragmentIndex) && FocusedFragmentIndex == FragmentIndex)
	{
		return LOCTEXT("DialogueText_Tooltip", "To be displayed during speaking");
	}

	return GetFragment()->Bit.GetTitleText().IsEmptyOrWhitespace() ? LOCTEXT("DialogueText_Tooltip", "No title text") : FText::Format(LOCTEXT("DialogueText_Tooltip", "Title Text: {0}"), GetFragment()->Bit.GetTitleText());
}

FSlateColor SFlowGraphNode_YapFragmentWidget::Dialogue_BackgroundColor() const
{
	return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? FlowYapEditor::Color::White : FlowYapEditor::Color::Noir;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::Dialogue_ForegroundColor() const
{
	return GetFlowYapDialogueNode()->GetIsPlayerPrompt() ? FlowYapEditor::Color::White : FlowYapEditor::Color::LightGray;
}

// ================================================================================================
// FRAGMENT TAG OVERLAY WIDGET (OVER DIALOGUE, PREVIEW PURPOSE ONLY)
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentTagPreviewWidget()
{	
	return SNew(SBorder)
	.BorderImage(FAppStyle::Get().GetBrush("DetailsView.CategoryMiddle"))
	.BorderBackgroundColor(this, &SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_BorderBackgroundColor)
	.Padding(6, 5, 6, 5)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_Visibility)
	.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_ColorAndOpacity)
	[
		SNew(STextBlock)
		.Text(this, &SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_Text)
		.IsEnabled(false)
		.Font(FAppStyle::GetFontStyle("SmallFont"))
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_Visibility() const
{
	if (GetFragment()->FragmentTag == FGameplayTag::EmptyTag)
	{
		return EVisibility::Collapsed;
	}

	if (!GetFragment()->FragmentTag.IsValid())
	{
		return EVisibility::HitTestInvisible;
	}
	
	/*
	if (FragmentFocused())
	{
		return EVisibility::Collapsed;
	}*/
	
	return EVisibility::HitTestInvisible;
}

FText SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_Text() const
{
	// Pass tag from the properties

	FString Filter = GetFlowYapDialogueNode()->GetPromptTag().ToString();
	
	return FText::FromString(FlowYapUtil::GetFilteredSubTag(Filter, GetFragment()->FragmentTag));
}

FSlateColor SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_BorderBackgroundColor() const
{
	if (DialogueBox->HasKeyboardFocus())
	{
		return FlowYapEditor::Color::DeepGray_Glass;
	}
	
	return FlowYapEditor::Color::DeepGray_SemiTrans;
}

FLinearColor SFlowGraphNode_YapFragmentWidget::FragmentTagPreview_ColorAndOpacity() const
{
	if (DialogueBox->HasKeyboardFocus())
	{
		return FlowYapEditor::Color::Gray_Trans;
	}
	
	return FlowYapEditor::Color::White;
}

// ================================================================================================
// PORTRAIT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreatePortraitWidget()
{
	return SNew(SBox)
	.Padding(0, 0, 0, 0)
	[
		SNew(SOverlay)
		.Visibility(this, &SFlowGraphNode_YapFragmentWidget::PortraitImage_Visibility)
		+ SOverlay::Slot()
		.Padding(4, 0, 0, 0)
		[
			SNew(SBox)
			.WidthOverride(58)//(74)
			.HeightOverride(58)//(74)
			[
				SNew(SBorder)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Padding(0.0f)
				.BorderImage(FAppStyle::Get().GetBrush("PropertyEditor.AssetThumbnailBorder"))
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					[
						SNew(SImage)
						.DesiredSizeOverride(FVector2D(48, 48))
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
			CreateMoodKeySelectorWidget()
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Right)
		.Padding(0, 0, 2, 0)
		[
			CreateFragmentControlsWidget()
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Left)
		[
			SAssignNew(AudioAssetProperty, SObjectPropertyEntryBox)
			.DisplayBrowse(false)
			.DisplayThumbnail(false)
			.DisplayUseSelected(false)
			.AllowedClass(UFlowYapCharacter::StaticClass())
			//.EnableContentPicker(true)
			//.ObjectPath(this, &SFlowGraphNode_YapFragmentWidget::AudioAsset_ObjectPath)
			//.OnObjectChanged(this, &SFlowGraphNode_YapFragmentWidget::AudioAsset_OnObjectChanged)
			.ToolTipText(LOCTEXT("DialogueAudioAsset_Tooltip", "Select an audio asset."))
		]
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::PortraitImage_Visibility() const
{	
	if (FragmentFocused())
	{
		return EVisibility::Collapsed;
	}
	
	if (!Owner->GetIsSelected())
	{
		return EVisibility::Visible;
	}

	return EVisibility::Visible;
}



const FSlateBrush* SFlowGraphNode_YapFragmentWidget::PortraitImage_Image() const
{
	return GetFragment()->GetBit().GetSpeakerPortraitBrush();
}

EVisibility SFlowGraphNode_YapFragmentWidget::MissingPortraitWarning_Visibility() const
{
	const FSlateBrush* Brush = GetFragment()->GetBit().GetSpeakerPortraitBrush();

	if (Brush)
	{
		return (Brush->GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
	}
	
	return EVisibility::Visible;
}

// ================================================================================================
// MOOD KEY SELECTOR WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateMoodKeySelectorWidget()
{
	TSharedPtr<SBox> Box;
	FMenuBuilder MenuBuilder(true, nullptr);
	FName SelectedMoodKey = GetCurrentMoodKey();

	for (const FName& MoodKey : UFlowYapProjectSettings::Get()->GetMoodKeys())
	{
		if (MoodKey == NAME_None)
		{
			UE_LOG(FlowYap, Warning, TEXT("Warning: Portrait keys contains a 'NONE' entry. Clean this up!"));
			continue;
		}
		
		bool bSelected = MoodKey == SelectedMoodKey;
		MenuBuilder.AddWidget(CreateMoodKeyMenuEntryWidget(MoodKey, bSelected), FText::GetEmpty());
	}

	TSharedPtr<SImage> PortraitIconImage;
	
	FString IconPath = UFlowYapProjectSettings::Get()->GetPortraitIconPath(GetCurrentMoodKey());

	// TODO ensure that system works and displays labels if user does not supply icons but only FNames. Use Generic mood icon?
	SAssignNew(Box, SBox)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::MoodKeySelector_Visibility)
	[
		SNew(SComboButton)
		.HasDownArrow(false)
		.ContentPadding(FMargin(0.f, 0.f))
		.MenuPlacement(MenuPlacement_CenteredBelowAnchor)
		.ButtonColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.75f)))
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.OnMenuOpenChanged(this, &SFlowGraphNode_YapFragmentWidget::MoodKeySelector_OnMenuOpenChanged)
		.ToolTipText(LOCTEXT("Fragment", "Choose a mood key"))
		.ButtonContent()
		[
			SNew(SBox)
			.Padding(4, 4)
			[
				SAssignNew(PortraitIconImage, SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(this, &SFlowGraphNode_YapFragmentWidget::MoodKeyBrush_GetBrush)
			]
		]
		.MenuContent()
		[
			MenuBuilder.MakeWidget()
		]
	];
	
	return Box.ToSharedRef();
}

EVisibility SFlowGraphNode_YapFragmentWidget::MoodKeySelector_Visibility() const
{
	return IsHovered() || MoodKeySelectorMenuOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

void SFlowGraphNode_YapFragmentWidget::MoodKeySelector_OnMenuOpenChanged(bool bMenuOpen)
{
	MoodKeySelectorMenuOpen = bMenuOpen;
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::MoodKeyBrush_GetBrush() const
{
	return GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetMoodKeyBrush(GetCurrentMoodKey());
}

FName SFlowGraphNode_YapFragmentWidget::GetCurrentMoodKey() const
{
	return GetFragment()->Bit.GetMoodKey();
}

// ================================================================================================
// MOOD KEY MENU ENTRY WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateMoodKeyMenuEntryWidget(FName InIconName, bool bSelected, const FText& InLabel, FName InTextStyle)
{
	const UFlowYapProjectSettings* ProjectSettings = UFlowYapProjectSettings::Get();
		
	TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);

	TSharedPtr<SImage> PortraitIconImage;
		
	FString IconPath = ProjectSettings->GetPortraitIconPath(InIconName);

	// TODO this is dumb, cache FSlateIcons or FSlateBrushes in the subsystem instead?
	UTexture2D* MoodKeyIcon = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetMoodKeyIcon(InIconName);
	
	FSlateBrush Brush;
	Brush.ImageSize = FVector2D(16, 16);
	Brush.SetResourceObject(MoodKeyIcon);
	
	TSharedRef<FDeferredCleanupSlateBrush> MoodKeyBrush = FDeferredCleanupSlateBrush::CreateBrush(Brush);
	
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
			.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([MoodKeyBrush](){return MoodKeyBrush->GetSlateBrush();})))
		];
	}

	// TODO ensure that system works and displays labels if user does not supply icons but only FNames
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
	.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::MoodKeyMenuEntry_OnClicked, InIconName)
	[
		SAssignNew(PortraitIconImage, SImage)
		.ColorAndOpacity(FSlateColor::UseForeground())
		.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([MoodKeyBrush](){return MoodKeyBrush->GetSlateBrush();})))
	];
}

FReply SFlowGraphNode_YapFragmentWidget::MoodKeyMenuEntry_OnClicked(FName NewValue)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeMoodKeyChanged", "Portrait Key Changed"), GetFlowYapDialogueNode());

	GetFragment()->Bit.SetMoodKey(NewValue);

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}



// ================================================================================================
// FRAGMENT CONTROLS WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateFragmentControlsWidget()
{
	return SNew(SBox)
	[
		SNew(SVerticalBox)
		.Visibility(this, &SFlowGraphNode_YapFragmentWidget::FragmentControls_Visibility)
		// UP
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Center)
		.Padding(0, 2, 0, 1)
		[
			SNew(SButton)
			.ButtonStyle(&MoveFragmentButtonStyle)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueMoveFragmentUp_Tooltip", "Move Fragment Up"))
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::MoveFragmentUpButton_Visibility)
			.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::MoveFragmentUpButton_OnClicked)
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Symbols.UpArrow"))
				.DesiredSizeOverride(FVector2D(8, 8))
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
			.ButtonStyle(&MoveFragmentButtonStyle)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueDeleteFragment_Tooltip", "Delete Fragment"))
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::DeleteFragmentButton_Visibility)
			.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::DeleteFragmentButton_OnClicked)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Cross"))
				.DesiredSizeOverride(FVector2D(8, 8))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
		// DOWN
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Center)
		.Padding(0, 1, 0, 2)
		[
			SNew(SButton)
			.ButtonStyle(&MoveFragmentButtonStyle)
			.ContentPadding(FMargin(3, 4))
			.ToolTipText(LOCTEXT("DialogueMoveFragmentDown_Tooltip", "Move Fragment Down"))
			.Visibility(this, &SFlowGraphNode_YapFragmentWidget::MoveFragmentDownButton_Visibility)
			.OnClicked(this, &SFlowGraphNode_YapFragmentWidget::MoveFragmentDownButton_OnClicked)
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Symbols.DownArrow"))
				.DesiredSizeOverride(FVector2D(8, 8))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
	];
}

EVisibility SFlowGraphNode_YapFragmentWidget::FragmentControls_Visibility() const
{
	return (IsHovered()) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SFlowGraphNode_YapFragmentWidget::MoveFragmentUpButton_Visibility() const
{
	return (GetFragment()->IndexInDialogue == 0) ? EVisibility::Hidden : EVisibility::Visible;		
}

FReply SFlowGraphNode_YapFragmentWidget::MoveFragmentUpButton_OnClicked()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Move Fragment"), GetFlowYapDialogueNode());

	Owner->MoveFragmentUp(GetFragment()->IndexInDialogue);
	
	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

EVisibility SFlowGraphNode_YapFragmentWidget::DeleteFragmentButton_Visibility() const
{
	if (GetFlowYapDialogueNode()->GetNumFragments() <= 1)
	{
		return EVisibility::Collapsed;
	}
	
	return EVisibility::Visible;
}

FReply SFlowGraphNode_YapFragmentWidget::DeleteFragmentButton_OnClicked()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueDeleteFragment", "Delete Fragment"), GetFlowYapDialogueNode() /*GetFlowYapDialogueNode()*/);

	Owner->DeleteFragment(GetFragment()->IndexInDialogue);

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

EVisibility SFlowGraphNode_YapFragmentWidget::MoveFragmentDownButton_Visibility() const
{
	return (GetFragment()->IndexInDialogue == GetFlowYapDialogueNode()->GetNumFragments() - 1) ? EVisibility::Hidden : EVisibility::Visible;
}

FReply SFlowGraphNode_YapFragmentWidget::MoveFragmentDownButton_OnClicked()
{
	FFlowYapTransactions::BeginModify(LOCTEXT("DialogueNode", "Move Fragment"), GetFlowYapDialogueNode());

	Owner->MoveFragmentDown(GetFragment()->IndexInDialogue);
	
	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

// ================================================================================================
// TITLE TEXT WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateTitleTextWidget()
{
	return SAssignNew(TitleTextBox, SEditableTextBox)
	.Visibility(this, &SFlowGraphNode_YapFragmentWidget::TitleText_Visibility)
	.Text(this, &SFlowGraphNode_YapFragmentWidget::TitleText_Text)
	.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::TitleText_OnTextCommitted)
	.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
	.Padding(2)
	.HintText(LOCTEXT("TitleText_Hint", "Enter optional title text"))
	.ToolTipText(LOCTEXT("TitleText_Tooltip", "Title text may be used to build player's dialogue selection list."));
}

EVisibility SFlowGraphNode_YapFragmentWidget::TitleText_Visibility() const
{
	if (GetFlowYapDialogueNode()->GetIsPlayerPrompt())
	{
		return EVisibility::Visible;
	}
	
	return UFlowYapProjectSettings::Get()->GetHideTitleTextOnNPCDialogueNodes() ? EVisibility::Hidden : EVisibility::Visible;
}

FText SFlowGraphNode_YapFragmentWidget::TitleText_Text() const
{
	return GetFragment()->Bit.GetTitleText();
}

void SFlowGraphNode_YapFragmentWidget::TitleText_OnTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeTitleTextChanged", "Title Text Changed"), GetFlowYapDialogueNode());
	
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetFragment()->Bit.SetTitleText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

// ================================================================================================
// FRAGMENT TAG WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateFragmentTagWidget()
{
	const FString FilterString = GetFlowYapDialogueNode()->GetPromptTag().ToString();

	return SNew(SGameplayTagComboFiltered)
	.Tag(this, &SFlowGraphNode_YapFragmentWidget::FragmentTag_Tag)
	.Filter(FilterString)
	.OnTagChanged(this, &SFlowGraphNode_YapFragmentWidget::FragmentTag_OnTagChanged);
}

FGameplayTag SFlowGraphNode_YapFragmentWidget::FragmentTag_Tag() const
{
	return GetFragment()->FragmentTag;
}

void SFlowGraphNode_YapFragmentWidget::FragmentTag_OnTagChanged(FGameplayTag GameplayTag)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Change Fragment Tag"), GetFlowYapDialogueNode());

	GetFragment()->FragmentTag = GameplayTag;

	FFlowYapTransactions::EndModify();
}

// ================================================================================================
// BOTTOM ROW WIDGET (AUDIO ASSET, TIME SETTINGS)
// ------------------------------------------------------------------------------------------------

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::CreateBottomRowWidget()
{
	FSlateIcon ProjectSettingsIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");

	return SNew(SBox)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.Padding(0, 2, 0, 0)
		[
			CreateAudioAssetWidget()
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
		// -------------------
		// TIME SETTINGS
		// -------------------
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		.Padding(6,2,0,0)
		[
			SNew(SBox)
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
						.Image(ProjectSettingsIcon.GetIcon())
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
					.WidthOverride(58)
					.VAlign(VAlign_Fill)
					[
						SNew(SNumericEntryBox<double>)
						.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::TimeEntryBox_IsEnabled)
						.Delta(0.1)
						.MinValue(0.0)
						.ToolTipText(LOCTEXT("FragmentTimeEntry_Tooltip", "Time this dialogue fragment will play for"))
						.Justification(ETextJustify::Center)
						.Value(this, &SFlowGraphNode_YapFragmentWidget::TimeEntryBox_Value)
						.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::TimeEntryBox_OnValueCommitted)
					]
				]
			]
		]
	];
}

// ---------------------
ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_IsChecked() const
{
	return GetFragment()->Bit.GetUseProjectDefaultTimeSettings() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseProjectDefaultTimeSettingsButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeUseProjectDefaultTimeSettings", "Use Project Default Time Settings Changed"), GetFlowYapDialogueNode());

	GetFragment()->Bit.SetUseProjectDefaultSettings(CheckBoxState == ECheckBoxState::Checked);
	
	FFlowYapTransactions::EndModify();
}

// ---------------------
bool SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_IsEnabled() const
{
	return GetFragment()->Bit.GetUseProjectDefaultTimeSettings() ? false : true;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_IsChecked() const
{
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::ManualTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseManuallyEnteredTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment()->Bit.SetBitTimeMode(EFlowYapTimeMode::ManualTime);
	}

	FFlowYapTransactions::EndModify();
}

// ---------------------
bool SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_IsEnabled() const
{
	return GetFragment()->Bit.GetUseProjectDefaultTimeSettings() ? false : true;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_IsChecked() const
{
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::TextTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseTextTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("Fragment", "Fragment Time Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment()->Bit.SetBitTimeMode(EFlowYapTimeMode::TextTime);
	}
	
	FFlowYapTransactions::EndModify();
}

// ---------------------
bool SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsEnabled() const
{
	return !GetFragment()->Bit.GetUseProjectDefaultTimeSettings();
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_IsChecked() const
{
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::AudioTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::UseAudioTimeButton_OnCheckStateChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeTimedModeChanged", "Timed Mode Changed"), GetFlowYapDialogueNode());

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		GetFragment()->Bit.SetBitTimeMode(EFlowYapTimeMode::AudioTime);
	}
	
	FFlowYapTransactions::EndModify();
}

bool SFlowGraphNode_YapFragmentWidget::TimeEntryBox_IsEnabled() const
{
	return GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::ManualTime;
}

TOptional<double> SFlowGraphNode_YapFragmentWidget::TimeEntryBox_Value() const
{
	double Time = GetFragment()->Bit.GetTime();
	
	return (Time > 0) ? Time : TOptional<double>();
}

void SFlowGraphNode_YapFragmentWidget::TimeEntryBox_OnValueCommitted(double NewValue, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeEnteredTimeChanged", "Entered Time Changed"), GetFlowYapDialogueNode());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetFragment()->Bit.SetManualTime(NewValue);
	}
	else if (CommitType == ETextCommit::OnCleared)
	{
		GetFragment()->Bit.SetManualTime(0.0);
	}

	FFlowYapTransactions::EndModify();
}

// ================================================================================================
// AUDIO ASSET WIDGET
// ------------------------------------------------------------------------------------------------

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateAudioAssetWidget()
{
	UClass* DialogueAssetClass = UFlowYapProjectSettings::Get()->GetDialogueAssetClass();

	if (!DialogueAssetClass)
	{
		DialogueAssetClass = UObject::StaticClass();
	}
	
	return SNew(SOverlay)
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
		.ObjectPath(this, &SFlowGraphNode_YapFragmentWidget::AudioAsset_ObjectPath)
		.OnObjectChanged(this, &SFlowGraphNode_YapFragmentWidget::AudioAsset_OnObjectChanged)
		.ToolTipText(LOCTEXT("DialogueAudioAsset_Tooltip", "Select an audio asset."))
	]
	+ SOverlay::Slot()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SImage)
		.Image(FAppStyle::GetBrush("MarqueeSelection"))
		.Visibility(this, &SFlowGraphNode_YapFragmentWidget::AudioAssetErrorState_Visibility)
		.ColorAndOpacity(this, &SFlowGraphNode_YapFragmentWidget::AudioAssetErrorState_ColorAndOpacity)
	];
}

FString SFlowGraphNode_YapFragmentWidget::AudioAsset_ObjectPath() const
{
	const TSoftObjectPtr<UObject> Asset = GetFragment()->Bit.GetDialogueAudioAsset<UObject>();

	if (!Asset) { return ""; }

	return Asset.ToString();
}

void SFlowGraphNode_YapFragmentWidget::AudioAsset_OnObjectChanged(const FAssetData& InAssetData)
{
	FFlowYapTransactions::BeginModify(LOCTEXT("NodeAudioAssetChanged", "Audio Asset Changed"), GetFlowYapDialogueNode());

	GetFragment()->Bit.SetDialogueAudioAsset(InAssetData.GetAsset());

	FFlowYapTransactions::EndModify();
}

EVisibility SFlowGraphNode_YapFragmentWidget::AudioAssetErrorState_Visibility() const
{
	if (AudioAssetErrorLevel() != EFlowYapErrorLevel::OK)
	{
		return EVisibility::HitTestInvisible;
	}
	
	return EVisibility::Hidden;
}

FSlateColor SFlowGraphNode_YapFragmentWidget::AudioAssetErrorState_ColorAndOpacity() const
{
	switch (AudioAssetErrorLevel())
	{
	case EFlowYapErrorLevel::OK:
		{
			return FlowYapEditor::Color::Green;
		}
	case EFlowYapErrorLevel::Warning:
		{
			return FlowYapEditor::Color::Orange;
		}
	case EFlowYapErrorLevel::Error:
		{
			return FlowYapEditor::Color::Red;
		}
	}

	return FlowYapEditor::Color::Black;
}

EFlowYapErrorLevel SFlowGraphNode_YapFragmentWidget::AudioAssetErrorLevel() const
{
	UClass* AssetClass = UFlowYapProjectSettings::Get()->GetDialogueAssetClass();

	static EFlowYapErrorLevel CachedErrorLevel = EFlowYapErrorLevel::OK;
	static double LastUpdateTime = 0;
	
	if (GWorld)
	{
		if (GWorld->GetRealTimeSeconds() - LastUpdateTime < 1.0)
		{
			return CachedErrorLevel;
		}		
	}
	
	const TSoftObjectPtr<UObject> Asset = GetFragment()->Bit.GetDialogueAudioAsset<UObject>();

	if (Asset)
	{
		if (!Asset->IsA(AssetClass))
		{
			CachedErrorLevel = EFlowYapErrorLevel::Error;
			return CachedErrorLevel;
		}
	}

	if (GetFragment()->Bit.GetTimeMode() == EFlowYapTimeMode::AudioTime && !GetFragment()->Bit.HasDialogueAudioAsset())
	{
		CachedErrorLevel = UFlowYapProjectSettings::Get()->GetMissingAudioErrorLevel();
		return CachedErrorLevel;
	}

	CachedErrorLevel = EFlowYapErrorLevel::OK;
	return CachedErrorLevel;
}

// ================================================================================================
// HELPER API
// ================================================================================================

UFlowNode_YapDialogue* SFlowGraphNode_YapFragmentWidget::GetFlowYapDialogueNode() const
{
	return Owner->GetFlowYapDialogueNode();
}

FFlowYapFragment* SFlowGraphNode_YapFragmentWidget::GetFragment() const
{
	return GetFlowYapDialogueNode()->GetFragmentByIndexMutable(FragmentIndex);
}

bool SFlowGraphNode_YapFragmentWidget::FragmentFocused() const
{
	uint8 FocusedFragmentIndex;
	return (Owner->GetFocusedFragmentIndex(FocusedFragmentIndex) && FocusedFragmentIndex == FragmentIndex);
}

// ================================================================================================
// OVERRIDES
// ================================================================================================

FSlateColor SFlowGraphNode_YapFragmentWidget::GetNodeTitleColor() const
{
	FLinearColor Color;

	if (GetFlowYapDialogueNode()->GetDynamicTitleColor(Color))
	{
		return Color;
	}

	return FLinearColor::Black;
}

void SFlowGraphNode_YapFragmentWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (DialogueBox->HasKeyboardFocus() || TitleTextBox->HasKeyboardFocus())
	{
		Owner->SetFocusedFragmentIndex(GetFragment()->IndexInDialogue);
	}
	else if (Owner->GetFocusedFragmentIndex() == GetFragment()->IndexInDialogue)
	{
		Owner->ClearTypingFocus();
	}
}

#undef LOCTEXT_NAMESPACE