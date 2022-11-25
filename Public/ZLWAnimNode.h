// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "ZLWAnimNode.generated.h"

UENUM()
enum class EZLWCurveApplyMode : uint8
{
	/** Blend input with new curve value, using Alpha setting on the node */
	Blend,
	/** Blend the new curve value with the last curve value using Alpha to determine the weighting (.5 is a moving average, higher values react to new values faster, lower slower) */
	WeightedMovingAverage,
};

/** Easy way to modify curve values on a pose */
USTRUCT(BlueprintInternalUseOnly)
struct FZLWAnimNode : public FAnimNode_Base
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = Links)
		FPoseLink SourcePose;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ModifyCurve, meta = (PinShownByDefault))
		float Alpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ModifyCurve)
		EZLWCurveApplyMode ApplyMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ModifyCurve, meta = (PinShownByDefault))
		int num_bs = 0;


	UPROPERTY(EditAnywhere, Category = SkeletalControl)
		FBoneReference BoneToModify_0 {
		"c7t1"
	};
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
		FBoneReference BoneToModify_1 {
		"c6c7"
	};
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
		FBoneReference BoneToModify_2 {
		"c5c6"
	};
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
		FBoneReference BoneToModify_3 {
		"c4c5"
	};
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
		FBoneReference BoneToModify_4 {
		"c3c4"
	};
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
		FBoneReference BoneToModify_5 {
		"c2c3"
	};
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
		FBoneReference BoneToModify_6 {
		"c1c2"
	};
	UPROPERTY(EditAnywhere, Category = SkeletalControl)
		FBoneReference BoneToModify_7 {
		"oc1"
	};

	TArray<FBoneReference> BoneToModify_list;
	TArray<FTransform> BoneTM_ref_list;
	//TArray<FTransform> BoneTM_delta_list;
	TArray<float> BoneTM_72_list;
	TArray<float> BoneTM_72_list_last;

	FZLWAnimNode();

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;

	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	// End of FAnimNode_Base interface

	void InitializeBoneReferences(const FBoneContainer& RequiredBones);

private:
	void ProcessCurveOperation(const EZLWCurveApplyMode& InApplyMode, FPoseContext& Output, const SmartName::UID_Type& NameUID, float CurrentValue, float NewValue);
	void ProcessCurveWMAOperation(FPoseContext& Output, const SmartName::UID_Type& NameUID, float CurrentValue, float NewValue, float& InOutLastValue);
	void ReadBones(FPoseContext& SourceData);
	void UpdateP(FPoseContext& SourceData);
};
