#pragma once
#include "Graph/Widgets/SFlowGraphNode.h"
#include "Widgets/SUserWidget.h"

struct FFlowYapFragment;
class SFlowGraphNode_YapFragmentWidget;
class UFlowNode_YapDialogue;
class UFlowGraphNode_YapDialogue;

class SFlowGraphNode_YapDialogueWidget : public SFlowGraphNode
{
protected:
	// TODO: Is this safe?
	UFlowGraphNode_YapDialogue* FlowGraphNode_YapDialogue = nullptr;

	TSharedPtr<SVerticalBox> FragmentBox;
	
	TArray<TSharedPtr<SFlowGraphNode_YapFragmentWidget>> FragmentWidgets;

	TArray<TSharedPtr<SVerticalBox>> FragmentInputBoxes;
	
	TArray<TSharedPtr<SVerticalBox>> FragmentOutputBoxes;

	TSharedPtr<SBox> BypassOutputBox;

	FTextBlockStyle NormalText;
	
public:
	void Construct(const FArguments& InArgs, UFlowGraphNode* InNode);

	UFlowNode_YapDialogue* GetFlowYapDialogueNode();
	
	const UFlowNode_YapDialogue* GetFlowYapDialogueNode() const;

protected:
	EVisibility GetFragmentMovementVisibility() const;

	FSlateColor GetFragmentMovementControlsColor() const;

	FReply MoveFragment(bool bUp, int64 Index);

	FSlateColor GetFragmentSeparatorColor() const;

	TOptional<int32> GetActivationLimit(FFlowYapFragment* Fragment) const;

	void OnActivationLimitChanged(int32 NewValue, FFlowYapFragment* Fragment);

	FSlateColor GetActivationDotColor(FFlowYapFragment* Fragment, int32 ActivationIndex) const;

	FReply OnClickedActivationDot(FFlowYapFragment* Fragment, int ActivationIndex);
	
	TSharedRef<SWidget> CreateNodeContentArea() override;

	ECheckBoxState GetIsUserPromptDialogue() const;

	void HandleUserPromptDialogueChanged(ECheckBoxState CheckBoxState);
	
	TSharedRef<SWidget> CreateTitleWidget(TSharedPtr<SNodeTitle> NodeTitle) override;
	
	EVisibility GetAddFragmentButtonVisibility() const;
	
	FSlateColor GetAddFragmentButtonColor() const;
	
	void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;

	void AddBypassPin(const TSharedRef<SGraphPin>& PinToAdd);
	
	FReply AddFragment();

public:
	FReply DeleteFragment(int64 FragmentID);
	
	bool GetNormalisedMousePositionInGeometry(UObject *WorldContextObject, FGeometry Geometry, FVector2D &Position) const;
	
	const FSlateBrush* GetShadowBrush(bool bSelected) const override;
};
