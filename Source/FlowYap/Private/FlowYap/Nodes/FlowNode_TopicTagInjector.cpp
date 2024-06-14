#include "FlowYap/Nodes/FlowNode_TopicTagInjector.h"

#include "FlowYap/Nodes/Category.h"

UFlowNode_TopicTagInjector::UFlowNode_TopicTagInjector()
{	
	InputPins.Empty();
}

FString UFlowNode_TopicTagInjector::GetNodeCategory() const
{
	return FlowYapRuntime::NodeCategory;
}

FText UFlowNode_TopicTagInjector::GetNodeTitle() const
{
	return Super::GetNodeTitle();
}

FText UFlowNode_TopicTagInjector::GetNodeToolTip() const
{
	return Super::GetNodeToolTip();
}

void UFlowNode_TopicTagInjector::InitializeInstance()
{
	Super::InitializeInstance();

	OnActivate();
}

void UFlowNode_TopicTagInjector::OnActivate()
{
	TriggerFirstOutput(true);
}