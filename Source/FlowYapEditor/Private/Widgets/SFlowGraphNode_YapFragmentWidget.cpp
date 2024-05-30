// 2022 The Enby Witch

#include "Widgets/SFlowGraphNode_YapFragmentWidget.h"

#include "AkAudioEvent.h"
#include "FlowYapColors.h"
#include "GraphNodes/FlowGraphNode_YapDialogue.h"
#include "PropertyCustomizationHelpers.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/Nodes/FlowNode_YapDialogue.h"
#include "Slate/DeferredCleanupSlateBrush.h"
#include "FlowYapEditorSubsystem.h"
#include "FlowYapTransactions.h"
#include "FlowYap/FlowYapLog.h"
#include "Widgets/SFlowGraphNode_YapDialogueWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "PGFlowEditor"

/*
TSharedRef<SFlowGraphNode_YapFragmentWidget> SFlowGraphNode_YapFragmentWidget::New()
{
	return MakeShareable(new SFlowGraphNode_YapFragmentWidget());
}
*/

void SFlowGraphNode_YapFragmentWidget::Construct(const FArguments& InArgs, SFlowGraphNode_YapDialogueWidget* InOwner, FFlowYapFragment* InFragment)
{
	Owner = InOwner;
	FragmentID = InFragment->GetEditorID();

	ChildSlot
	[
		CreateDialogueContentArea()
	];
}

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreateDialogueContentArea()
{
	return SNew(SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		// ===================
		// PORTRAIT IMAGE (LEFT SIDE)
		// ===================
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.AutoWidth()
		.Padding(2.f, 5.f, 5.f, 2.f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				GetPortraitWidget()
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(0, 0, 2, 2))
			[
				GetPortraitKeySelectorWidget()
			]
		]
		// ===================
		// DIALOGUE SETTINGS (RIGHT SIDE)
		// ===================
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		.FillWidth(1.0f)
		.Padding(5.f)
		[
			SNew(SVerticalBox)
			// ===================
			// TITLE TEXT
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Fill)
				[
					SNew(SEditableTextBox)
					.Text(this, &SFlowGraphNode_YapFragmentWidget::GetTitleText)
					.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::HandleTitleTextCommitted)
					.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					.MinDesiredWidth(270)
					.HintText(INVTEXT("Enter optional title text"))
					.ToolTipText(LOCTEXT("Test", "Test Test TODO"))
					//.ToolTip(INVTEXT("Title text is only used for selectable dialogue.\nWhen supplied, the game's UI will display selectable repsonses using their title text representation.\nWhen not supplied, the game's UI will display the full dialogue below for selectable responses."))
				]
			]
			// ===================
			// SPACER
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(4.f)
			]
			// ===================
			// AUDIO ASSET SELECTOR
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SObjectPropertyEntryBox)
				.DisplayBrowse(true)
				.DisplayThumbnail(true)
				.AllowedClass(UAkAudioEvent::StaticClass())
				.EnableContentPicker(true)
				.ObjectPath(this, &SFlowGraphNode_YapFragmentWidget::GetSelectedDialogueAudioAssetPath)
				.OnObjectChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleDialogueAudioAssetChanged)
				.ToolTipText(LOCTEXT("Test", "Test Test TODO"))
			]
			// ===================
			// SPACER
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SSpacer)
				.Size(4.f)
			]
			// ===================
			// ADDITIONAL OPTIONS
			// ===================
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				GetAdditionalOptionsWidget()
			]
		]
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Top)
		.MaxDesiredHeight(this, &SFlowGraphNode_YapFragmentWidget::GetMaxDialogueWidgetHeight)
		.MaxDesiredWidth(this, &SFlowGraphNode_YapFragmentWidget::GetMaxDialogueWidgetWidth)
		.Padding(4.f)
		[
			SAssignNew(DialogueBox, SMultiLineEditableTextBox)
			.Text(this, &SFlowGraphNode_YapFragmentWidget::GetDialogueText)
			.OnTextCommitted(this, &SFlowGraphNode_YapFragmentWidget::HandleDialogueTextCommitted)
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.HintText(INVTEXT("Enter full dialogue"))
		]
	];
}

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::GetPortraitWidget()
{
	TSharedPtr<SBox> PortraitPreview;
	
	SAssignNew(PortraitPreview, SBox)
	.WidthOverride(100)
	.HeightOverride(100)
	[
		SNew(SBorder)
		.ForegroundColor(FLinearColor::White)
		//.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleColor)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(2)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(this, &SFlowGraphNode_YapFragmentWidget::GetPortraitBrush)
			]
			+ SOverlay::Slot()
			[
				SNew(STextBlock)
				.RenderTransformPivot(FVector2D(0.5, 0.5))
				.RenderTransform(FSlateRenderTransform(FQuat2D(FMath::DegreesToRadians(-30.0f))))
				.Visibility(this, &SFlowGraphNode_YapFragmentWidget::GetVisibilityForMissingPortraitText)
				.Text(INVTEXT("MISSING"))
			]
		]
	];

	return PortraitPreview.ToSharedRef();
}

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::GetPortraitKeySelectorWidget()
{
	TSharedPtr<SBox> Box;
	
	FMenuBuilder MenuBuilder(true, NULL);
	
	FName SelectedPortraitKey = SFlowGraphNode_YapFragmentWidget::GetPortraitKey();

	for (const FName& PortraitKey : GetDefault<UFlowYapProjectSettings>()->GetPortraitKeys())
	{
		if (PortraitKey == NAME_None)
		{
			UE_LOG(FlowYap, Warning, TEXT("Warning: Portrait keys contains a 'NONE' entry. Clean this up!"));
			continue;
		}
		
		bool bSelected = PortraitKey == SelectedPortraitKey;
		MenuBuilder.AddWidget(CreatePortraitKeyButton(PortraitKey, bSelected), FText::GetEmpty());
	}

	TSharedPtr<SImage> PortraitIconImage;
	
	FString IconPath = GetDefault<UFlowYapProjectSettings>()->GetPortraitIconPath(GetPortraitKey());

	// TODO this is kind of uggers, can I maybe store FSlateIcons in the subsystem instead?
	UTexture2D* PortraitKeyIcon = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetPortraitKeyIcon(GetPortraitKey());
	
	FSlateBrush Brush;
	Brush.ImageSize = FVector2D(16, 16);
	Brush.SetResourceObject(PortraitKeyIcon);
		
	TSharedRef<FDeferredCleanupSlateBrush> PortraitKeyBrush = FDeferredCleanupSlateBrush::CreateBrush(Brush);
	
	SAssignNew(Box, SBox)
	[
		SNew(SComboButton)
		.HasDownArrow(false)
		.ContentPadding(FMargin(0.f, 0.f))
		.MenuPlacement(MenuPlacement_CenteredBelowAnchor)
		.ButtonColorAndOpacity(FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.75f)))
		.HAlign(HAlign_Center)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ButtonContent()
		[
			SNew(SBox)
			.Padding(4, 4)
			[
				SAssignNew(PortraitIconImage, SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				//.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([PortraitKeyBrush](){return PortraitKeyBrush->GetSlateBrush();})))
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

TSharedRef<SWidget> SFlowGraphNode_YapFragmentWidget::CreatePortraitKeyButton(FName InIconName, bool bSelected, const FText& InLabel, FName InTextStyle)
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
		.Padding(0.f)
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
		//HBox.ToSharedRef()
		SAssignNew(PortraitIconImage, SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(TAttribute<const FSlateBrush*>::Create(TAttribute<const FSlateBrush*>::FGetter::CreateLambda([PortraitKeyBrush](){return PortraitKeyBrush->GetSlateBrush();})))
				//.Image(this, &SFlowGraphNode_YapFragmentWidget::GetPortraitBrush)
	];
}

#pragma region Getters
FText SFlowGraphNode_YapFragmentWidget::GetTitleText() const
{
	return GetFragment().GetTitleText();
}

FText SFlowGraphNode_YapFragmentWidget::GetDialogueText() const
{
	return GetFragment().GetDialogueText();
}

FString SFlowGraphNode_YapFragmentWidget::GetSelectedDialogueAudioAssetPath() const
{
	const UAkAudioEvent* Asset = GetFragment().GetDialogueAudio();

	if (!Asset)
	{
		return "";
	}

	FSoftObjectPath Path(Asset);

	return Path.GetAssetPathString();
}

FName SFlowGraphNode_YapFragmentWidget::GetPortraitKey() const
{
	return GetFragment().GetPortraitKey();
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::GetPortraitBrush() const
{
	FSlateBrush* Brush = GetFlowNodeYapDialogue()->GetSpeakerPortraitBrush(GetPortraitKey());

	if (Brush && Brush->GetResourceObject())
	{
		return Brush;
	}

	return nullptr;
}

const FSlateBrush* SFlowGraphNode_YapFragmentWidget::GetPortraitKeyBrush() const
{
	return GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetPortraitKeyBrush(GetPortraitKey());
}

EVisibility SFlowGraphNode_YapFragmentWidget::GetVisibilityForMissingPortraitText() const
{
	FSlateBrush* Brush = GetFlowNodeYapDialogue()->GetSpeakerPortraitBrush(GetPortraitKey());

	if (Brush)
	{
		return (Brush->GetResourceObject()) ? EVisibility::Hidden : EVisibility::Visible;
	}
	
	return EVisibility::Visible;
}

UFlowNode_YapDialogue* SFlowGraphNode_YapFragmentWidget::GetFlowNodeYapDialogue() const
{
	return Owner->GetFlowYapDialogueNode();
}

FFlowYapFragment& SFlowGraphNode_YapFragmentWidget::GetFragment() const
{
	return Owner->GetFlowYapDialogueNode()->GetFragment(FragmentID);
}
#pragma endregion

#pragma region Input Handling
void SFlowGraphNode_YapFragmentWidget::HandleTitleTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Title Text Changed"), GetFlowNodeYapDialogue());
	
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetFragment().SetTitleText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

void SFlowGraphNode_YapFragmentWidget::HandleDialogueTextCommitted(const FText& CommittedText, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Dialogue Text Changed"), GetFlowNodeYapDialogue());

	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		GetFragment().SetDialogueText(CommittedText);
	}

	FFlowYapTransactions::EndModify();
}

void SFlowGraphNode_YapFragmentWidget::HandleDialogueAudioAssetChanged(const FAssetData& InAssetData)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Audio Asset Changed"), GetFlowNodeYapDialogue());

	GetFragment().SetDialogueAudio(Cast<UAkAudioEvent>(InAssetData.GetAsset()));

	FFlowYapTransactions::EndModify();
}

FReply SFlowGraphNode_YapFragmentWidget::HandlePortraitKeyChanged(FName NewValue)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Portrait Key Changed"), GetFlowNodeYapDialogue());

	GetFragment().SetPortraitKey(NewValue);

	FFlowYapTransactions::EndModify();

	return FReply::Handled();
}

FOptionalSize SFlowGraphNode_YapFragmentWidget::GetMaxDialogueWidgetHeight() const
{
	int16 Deadspace = 15;
	int16 LineHeight = 15;

	int16 UnfocusedLines = 6;
	int16 FocusedLines = 8;
	
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
#pragma endregion

FOptionalSize SFlowGraphNode_YapFragmentWidget::GetMaxDialogueWidgetWidth() const
{
	if (!DialogueBox.Get())
	{
		return 421;
	}
	
	if (DialogueBox->HasKeyboardFocus())
	{
		// It feels weird to have the graph morphing around. Don't permit any stretching.
		// TODO can I draw a whole new multiline widget over top of everything somehow eventually?
		return 421;
	}

	return 421;
}

TSharedRef<SBox> SFlowGraphNode_YapFragmentWidget::GetAdditionalOptionsWidget()
{
	TSharedPtr<SBox> Box;
	
	UTexture2D* TimedIcon = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetDialogueTimerIco();
	FSlateBrush TimedBrush;
	TimedBrush.ImageSize = FVector2D(16, 16);
	TimedBrush.SetResourceObject(TimedIcon);
	TSharedRef<FDeferredCleanupSlateBrush> TimedIconBrush = FDeferredCleanupSlateBrush::CreateBrush(TimedBrush);

	UTexture2D* UserInterruptibleIcon = GEditor->GetEditorSubsystem<UFlowYapEditorSubsystem>()->GetDialogueUserInterruptIco();
	FSlateBrush InterruptibleBrush;
	InterruptibleBrush.ImageSize = FVector2D(16, 16);
	InterruptibleBrush.SetResourceObject(UserInterruptibleIcon);
	TSharedRef<FDeferredCleanupSlateBrush> InterruptibleIconBrush = FDeferredCleanupSlateBrush::CreateBrush(InterruptibleBrush);

	FSlateIcon ProjectSettingsIcon(FAppStyle::GetAppStyleSetName(), "ProjectSettings.TabIcon");
	const FSlateBrush* ProjectSettingsIconBrush = ProjectSettingsIcon.GetIcon();
	
	TSharedRef<FDeferredCleanupSlateBrush> ProjectSettingsBrush = FDeferredCleanupSlateBrush::CreateBrush(*ProjectSettingsIconBrush);

	ProjectSettingsButtonStyle = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox");
	//Style.SetCheckedForegroundColor(FSlateColor(FLinearColor::Red));
	ProjectSettingsButtonStyle.CheckedImage.TintColor = FLinearColor(FlowYapColors::Orange);
	ProjectSettingsButtonStyle.CheckedHoveredImage.TintColor = FLinearColor(FlowYapColors::OrangeHovered);
	ProjectSettingsButtonStyle.CheckedPressedImage.TintColor = FLinearColor(FlowYapColors::OrangePressed);
	
	SAssignNew(Box, SBox)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.Style(&ProjectSettingsButtonStyle)
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.ToolTipText(INVTEXT("Test Test TODO"))
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetUseProjectSettings)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleUseProjectSettingsChanged)
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(TAttribute<const FSlateBrush*>::Create(
				TAttribute<const FSlateBrush*>::FGetter::CreateLambda([ProjectSettingsBrush](){return ProjectSettingsBrush->GetSlateBrush();})))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SSpacer)
			.Size(2)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetTimedButtonEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetTimed)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleTimedChanged)
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(TAttribute<const FSlateBrush*>::Create(
				TAttribute<const FSlateBrush*>::FGetter::CreateLambda([TimedIconBrush](){return TimedIconBrush->GetSlateBrush();})))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SSpacer)
			.Size(2)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SNumericEntryBox<double>)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetTimeEntryEnabled)
			.Delta(0.1)
			.MinValue(0.0)
			.MinDesiredValueWidth(26)
			.Value(this, &SFlowGraphNode_YapFragmentWidget::GetTimeManualValue)
			.Justification(ETextJustify::Center)
			.OnValueCommitted(this, &SFlowGraphNode_YapFragmentWidget::HandleTimeChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SSpacer)
			.Size(2)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetUseAutoTimeEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetUseAutoTime)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleUseAutoTimeChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetUseAutoTimeEnabled)
			.Text(INVTEXT("Auto"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0)
		[
			SNew(SSpacer)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetUseAudioLengthEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetUseAudioLength)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleUseAudioLengthChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetUseAudioLengthEnabled)
			.Text(INVTEXT("Match Audio"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0)
		[
			SNew(SSpacer)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SCheckBox)
			.Style( &FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckBox"))
			.Padding(FMargin(4, 3))
			.CheckBoxContentUsesAutoWidth(true)
			.IsEnabled(this, &SFlowGraphNode_YapFragmentWidget::GetUserInterruptibleButtonEnabled)
			.IsChecked(this, &SFlowGraphNode_YapFragmentWidget::GetUserInterruptible)
			.OnCheckStateChanged(this, &SFlowGraphNode_YapFragmentWidget::HandleInterruptibleChanged)
			[
				SNew(SBox)
				[
					SNew(SImage)
					.ColorAndOpacity(FSlateColor::UseForeground())
					.Image(TAttribute<const FSlateBrush*>::Create(
					TAttribute<const FSlateBrush*>::FGetter::CreateLambda([InterruptibleIconBrush](){return InterruptibleIconBrush->GetSlateBrush();})))
				]
			]
		]
	];

	return Box.ToSharedRef(); 
}

// ----------------------------------

ECheckBoxState SFlowGraphNode_YapFragmentWidget::GetUseProjectSettings() const
{
	return GetFragment().GetUseProjectSettings() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::GetUserInterruptible() const
{
	if (GetFragment().GetUseProjectSettings())
	{
		return GetDefault<UFlowYapProjectSettings>()->GetSharedSettings().bUserInterruptible ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	
	return GetFragment().GetUserInterruptible() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

bool SFlowGraphNode_YapFragmentWidget::GetTimedButtonEnabled() const
{
	if (GetFragment().GetUseProjectSettings())
	{
		return false;
	}

	return true;
}

bool SFlowGraphNode_YapFragmentWidget::GetTimeEntryEnabled() const
{
	if (GetFragment().GetUseProjectSettings())
	{
		return false;
	}
	
	if (!GetFragment().GetIsTimed())
	{
		return false;
	}

	if (GetFragment().GetUseAutoTime())
	{
		return false;
	}

	if (GetFragment().GetUseAudioTime() && GetFragment().HasDialogueAudio())
	{
		return false;
	}
	
	return true;
}

bool SFlowGraphNode_YapFragmentWidget::GetUseAutoTimeEnabled() const
{
	if (GetFragment().GetUseProjectSettings())
	{
		return false;
	}
	
	return GetFragment().GetIsTimed();
}

bool SFlowGraphNode_YapFragmentWidget::GetUseAudioLengthEnabled() const
{
	if (GetFragment().GetUseProjectSettings())
	{
		return false;
	}
	
	return GetFragment().GetIsTimed() && GetFragment().HasDialogueAudio();
}

bool SFlowGraphNode_YapFragmentWidget::GetUserInterruptibleButtonEnabled() const
{
	if (GetFragment().GetUseProjectSettings())
	{
		return false;
	}

	return GetFragment().GetIsTimed();
}

// ----------------------------------

ECheckBoxState SFlowGraphNode_YapFragmentWidget::GetTimed() const
{
	if (GetFragment().GetUseProjectSettings())
	{
		return GetDefault<UFlowYapProjectSettings>()->GetSharedSettings().bIsTimed ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	
	return GetFragment().GetIsTimed() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

TOptional<double> SFlowGraphNode_YapFragmentWidget::GetTimeManualValue() const
{
	TOptional<double> Value;

	if (GetFragment().GetUseProjectSettings())
	{
		const FFlowYapFragmentSharedSettings& Settings = GetDefault<UFlowYapProjectSettings>()->GetSharedSettings();

		if (Settings.bIsTimed && !Settings.bUseAudioLength && !Settings.bUseAutoTime)
		{
			Value = Settings.TimeManual;
		}
		else
		{
			Value.Reset();
		}
	}
	else if (GetFragment().GetIsTimed() && !GetFragment().GetUseAudioTime() && !GetFragment().GetUseAutoTime())
	{
		Value = GetFragment().GetTimeManualValue();
	}
	else
	{
		Value.Reset();
	}
	
	return Value;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::GetUseAutoTime() const
{
	if (GetFragment().GetUseProjectSettings())
	{
		return GetDefault<UFlowYapProjectSettings>()->GetSharedSettings().bUseAutoTime ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	
	return GetFragment().GetUseAutoTime() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SFlowGraphNode_YapFragmentWidget::GetUseAudioLength() const
{
	if (GetFragment().GetUseProjectSettings())
	{
		return GetDefault<UFlowYapProjectSettings>()->GetSharedSettings().bUseAudioLength ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	
	return (GetFragment().GetUseAudioTime() && GetFragment().HasDialogueAudio()) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SFlowGraphNode_YapFragmentWidget::HandleUseProjectSettingsChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Use Project Settings Changed"), GetFlowNodeYapDialogue());

	GetFragment().SetUseProjectSettings(CheckBoxState == ECheckBoxState::Checked ? true : false);

	FFlowYapTransactions::EndModify();
}

// ----------------------------------

void SFlowGraphNode_YapFragmentWidget::HandleInterruptibleChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Interruptible Changed"), GetFlowNodeYapDialogue());
	
	GetFragment().SetUserInterruptible(CheckBoxState == ECheckBoxState::Checked ? true : false);

	FFlowYapTransactions::EndModify();
}

void SFlowGraphNode_YapFragmentWidget::HandleTimedChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Timed Changed"), GetFlowNodeYapDialogue());

	GetFragment().SetIsTimed(CheckBoxState == ECheckBoxState::Checked ? true : false);

	FFlowYapTransactions::EndModify();
}

void SFlowGraphNode_YapFragmentWidget::HandleTimeChanged(double NewValue, ETextCommit::Type CommitType)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Time Value Changed"), GetFlowNodeYapDialogue());

	FFlowYapFragment& Fragment = GetFragment();
	
	if (CommitType == ETextCommit::OnEnter || CommitType == ETextCommit::OnUserMovedFocus)
	{
		Fragment.SetTimeManualValue(NewValue);
	}

	if (CommitType == ETextCommit::OnCleared)
	{
		Fragment.SetTimeManualValue(0.0);
	}

	FFlowYapTransactions::EndModify();
}

void SFlowGraphNode_YapFragmentWidget::HandleUseAutoTimeChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Use Auto Time Changed"), GetFlowNodeYapDialogue());

	FFlowYapFragment& Fragment = GetFragment();
	
	Fragment.SetUseAutoTime(CheckBoxState == ECheckBoxState::Checked ? true : false);

	if (CheckBoxState == ECheckBoxState::Checked)
	{
		Fragment.SetUseAudioTime(false);
	}

	FFlowYapTransactions::EndModify();
}

void SFlowGraphNode_YapFragmentWidget::HandleUseAudioLengthChanged(ECheckBoxState CheckBoxState)
{
	FFlowYapTransactions::BeginModify(INVTEXT("Use Audio Length Changed"), GetFlowNodeYapDialogue());

	FFlowYapFragment& Fragment = GetFragment();

	Fragment.SetUseAudioTime(CheckBoxState == ECheckBoxState::Checked ? true : false);
	
	if (CheckBoxState == ECheckBoxState::Checked)
	{
		Fragment.SetUseAutoTime(false);
	}

	FFlowYapTransactions::EndModify();
}

#undef LOCTEXT_NAMESPACE