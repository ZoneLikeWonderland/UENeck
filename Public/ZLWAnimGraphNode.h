// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ZLWAnimNode.h"
#include "CoreMinimal.h"
#include "AnimGraph/Public/AnimGraphNode_Base.h"
#include "ZLWAnimGraphNode.generated.h"
/**
 *
 */
UCLASS()
class UZLWAnimGraphNode : public UAnimGraphNode_Base
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "Settings")
		FZLWAnimNode Node;

	//~ Begin UEdGraphNode Interface.
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ End UEdGraphNode Interface.

	//~ Begin UAnimGraphNode_Base Interface
	virtual FString GetNodeCategory() const override;
	//~ End UAnimGraphNode_Base Interface

	UZLWAnimGraphNode(const FObjectInitializer& ObjectInitializer);
};
