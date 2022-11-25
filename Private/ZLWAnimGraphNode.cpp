
// Fill out your copyright notice in the Description page of Project Settings.

#include "ZLWAnimGraphNode.h"

#define LOCTEXT_NAMESPACE "A3Nodes"

UZLWAnimGraphNode::UZLWAnimGraphNode(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

FLinearColor UZLWAnimGraphNode::GetNodeTitleColor() const
{
	return FLinearColor::Green;
}

FText UZLWAnimGraphNode::GetTooltipText() const
{
	return LOCTEXT("ZLWBlendNode", "ZLWBlendNode");
}

FText UZLWAnimGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("ZLWBlendNode", "ZLWBlendNode");
}

FString UZLWAnimGraphNode::GetNodeCategory() const
{
	return TEXT("ZLWBlendNode");
}

#undef LOCTEXT_NAMESPACE

