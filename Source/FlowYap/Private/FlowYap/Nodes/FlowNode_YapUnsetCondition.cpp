#include "FlowYap/Nodes/FlowNode_YapUnsetCondition.h"

#include "GameplayTagsManager.h"
#include "FlowYap/FlowYapProjectSettings.h"
#include "FlowYap/FlowYapSubsystem.h"
#include "FlowYap/FlowYapUtil.h"

UFlowNode_YapUnsetCondition::UFlowNode_YapUnsetCondition()
{
	NodeStyle = EFlowNodeStyle::Default;
	
#if WITH_EDITOR
	UFlowYapProjectSettings::RegisterTagFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, Condition), EFlowYap_TagFilter::Conditions);
#endif
}

FString UFlowNode_YapUnsetCondition::GetNodeCategory() const
{
	return FlowYapUtil::NodeCategory;
}

FText UFlowNode_YapUnsetCondition::GetNodeTitle() const
{
	return INVTEXT("Yap Unset Condition");
}

FString UFlowNode_YapUnsetCondition::GetNodeDescription() const
{
	return UFlowYapProjectSettings::GetTrimmedGameplayTagString(EFlowYap_TagFilter::Conditions, Condition);
}
