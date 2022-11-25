// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnimNodes/AnimNode_ModifyCurve.h"
#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "ZLWAnimNode.h"
#include "MLP.h"

FZLWAnimNode::FZLWAnimNode()
{
	ApplyMode = EZLWCurveApplyMode::Blend;
	Alpha = 1.f;
}

void FZLWAnimNode::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)
	Super::Initialize_AnyThread(Context);
	SourcePose.Initialize(Context);

	MLP::initialize();
	auto mlp = MLP();
	mlp.run({ 1,1,1 });
}

void FZLWAnimNode::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)
	Super::CacheBones_AnyThread(Context);
	InitializeBoneReferences(Context.AnimInstanceProxy->GetRequiredBones());
	SourcePose.CacheBones(Context);
}

void FZLWAnimNode::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)
	FPoseContext SourceData(Output);
	SourcePose.Evaluate(SourceData);

	ReadBones(SourceData);
	UpdateP(SourceData);


	Output = SourceData;

	////	Morph target and Material parameter curves
	//USkeleton* Skeleton = Output.AnimInstanceProxy->GetSkeleton();

	//for (int32 ModIdx = 0; ModIdx < num_bs; ModIdx++)
	//{
	//	FName CurveName("000");
	//	SmartName::UID_Type NameUID = Skeleton->GetUIDByName(USkeleton::AnimCurveMappingName, CurveName);
	//	if (NameUID != SmartName::MaxUID)
	//	{
	//		float NewValue = 1.0 * rand() / RAND_MAX;
	//		float CurrentValue = Output.Curve.Get(NameUID);
	//		if (ApplyMode == EZLWCurveApplyMode::WeightedMovingAverage)
	//		{
	//			float& LastValue = LastCurveValues[ModIdx];
	//			ProcessCurveWMAOperation(Output, NameUID, CurrentValue, NewValue, LastValue);
	//		}
	//		else
	//		{
	//			ProcessCurveOperation(ApplyMode, Output, NameUID, CurrentValue, NewValue);
	//		}
	//	}
	//}
}

void FZLWAnimNode::ProcessCurveOperation(const EZLWCurveApplyMode& InApplyMode, FPoseContext& Output, const SmartName::UID_Type& NameUID, float CurrentValue, float NewValue)
{
	float UseNewValue = CurrentValue;

	if (InApplyMode == EZLWCurveApplyMode::Blend)
	{
		UseNewValue = NewValue;
	}

	float UseAlpha = FMath::Clamp(Alpha, 0.f, 1.f);
	Output.Curve.Set(NameUID, FMath::Lerp(CurrentValue, UseNewValue, UseAlpha));
}


void FZLWAnimNode::ProcessCurveWMAOperation(FPoseContext& Output, const SmartName::UID_Type& NameUID, float CurrentValue, float NewValue, float& InOutLastValue)
{
	const float WAvg = FMath::WeightedMovingAverage(CurrentValue, InOutLastValue, Alpha);
	// Update the last curve value for next run
	InOutLastValue = WAvg;

	Output.Curve.Set(NameUID, WAvg);
}

void FZLWAnimNode::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)

	// Run update on input pose nodes
	SourcePose.Update(Context);

	// Evaluate any BP logic plugged into this node
	GetEvaluateGraphExposedInputs().Execute(Context);
}

void FZLWAnimNode::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(InitializeBoneReferences)

	BoneToModify_list.Reset(8);
	BoneTM_ref_list.Reset(8);

	BoneTM_72_list.Reset(72);
	BoneTM_72_list.AddZeroed(72);
	BoneTM_72_list_last.Reset(72);
	BoneTM_72_list_last.AddZeroed(72);


	BoneToModify_list.Add(BoneToModify_0);
	BoneToModify_list.Add(BoneToModify_1);
	BoneToModify_list.Add(BoneToModify_2);
	BoneToModify_list.Add(BoneToModify_3);
	BoneToModify_list.Add(BoneToModify_4);
	BoneToModify_list.Add(BoneToModify_5);
	BoneToModify_list.Add(BoneToModify_6);
	BoneToModify_list.Add(BoneToModify_7);

	for (auto& BoneToModify : BoneToModify_list) {
		BoneToModify.Initialize(RequiredBones);
		BoneTM_ref_list.Add(RequiredBones.GetRefPoseTransform(BoneToModify.GetCompactPoseIndex(RequiredBones)));
	}
}


void FZLWAnimNode::ReadBones(FPoseContext& SourceData) {


	FComponentSpacePoseContext CSPose(SourceData.AnimInstanceProxy);

	CSPose.Pose.InitPose(SourceData.Pose);
	//CSPose.Curve = MoveTemp(SourceData.Curve);
	//CSPose.CustomAttributes = MoveTemp(SourceData.CustomAttributes);

	const FBoneContainer& BoneContainer = SourceData.Pose.GetBoneContainer();

	//UE_LOG(LogTemp, Warning, TEXT("======================="));

	for (int i = 0; i < BoneToModify_list.Num(); i++) {
		auto BoneToModify = BoneToModify_list[i];
		auto BoneTM_ref = BoneTM_ref_list[i];

		FCompactPoseBoneIndex BoneIndex = BoneToModify.GetCompactPoseIndex(BoneContainer);
		const FTransform& BoneTM = CSPose.Pose.GetComponentSpaceTransform(BoneIndex); // global
		FTransform NewBoneTM = BoneTM;
		FTransform BoneTM_to_parent = BoneTM;

		//UE_LOG(LogTemp, Warning, TEXT("current BoneTM is %s"), *BoneTM.GetRotation().Euler().ToString());
		//UE_LOG(LogTemp, Warning, TEXT("current BoneTM_ref is %s"), *BoneTM_ref.GetRotation().Euler().ToString());

		const FCompactPoseBoneIndex ParentIndex = CSPose.Pose.GetPose().GetParentBoneIndex(BoneIndex);
		if (ParentIndex != INDEX_NONE)
		{
			const FTransform& ParentTM = CSPose.Pose.GetComponentSpaceTransform(ParentIndex);
			BoneTM_to_parent.SetToRelativeTransform(ParentTM);
		}

		//UE_LOG(LogTemp, Warning, TEXT("current BoneTM_to_parent is %s"), *BoneTM_to_parent.GetRotation().Euler().ToString());

		FTransform BoneTM_to_ref = BoneTM_to_parent;
		BoneTM_to_ref.SetToRelativeTransform(BoneTM_ref);

		//UE_LOG(LogTemp, Warning, TEXT("current BoneTM_to_ref[%d] is %s"), i, *BoneTM_to_ref.GetRotation().Euler().ToString());

		FMatrix44d matrix = BoneTM_to_ref.GetRotation().ToMatrix();
		BoneTM_72_list[i * 9 + 0] = matrix.M[0][0] - 1;
		BoneTM_72_list[i * 9 + 1] = matrix.M[1][0] * -1;
		BoneTM_72_list[i * 9 + 2] = matrix.M[2][0];
		BoneTM_72_list[i * 9 + 3] = matrix.M[0][1] * -1;
		BoneTM_72_list[i * 9 + 4] = matrix.M[1][1] - 1;
		BoneTM_72_list[i * 9 + 5] = matrix.M[2][1] * -1;
		BoneTM_72_list[i * 9 + 6] = matrix.M[0][2];
		BoneTM_72_list[i * 9 + 7] = matrix.M[1][2] * -1;
		BoneTM_72_list[i * 9 + 8] = matrix.M[2][2] - 1;

		//if (i == 1) {
		//	UE_LOG(LogTemp, Warning, TEXT("current BoneTM_72_list is %.3f %.3f %.3f"), BoneTM_72_list[i * 9 + 0], BoneTM_72_list[i * 9 + 1], BoneTM_72_list[i * 9 + 2]);
		//	UE_LOG(LogTemp, Warning, TEXT("current BoneTM_72_list is %.3f %.3f %.3f"), BoneTM_72_list[i * 9 + 3], BoneTM_72_list[i * 9 + 4], BoneTM_72_list[i * 9 + 5]);
		//	UE_LOG(LogTemp, Warning, TEXT("current BoneTM_72_list is %.3f %.3f %.3f"), BoneTM_72_list[i * 9 + 6], BoneTM_72_list[i * 9 + 7], BoneTM_72_list[i * 9 + 8]);
		//}
	}


	//NewBoneTM.SetToRelativeTransform(BoneTM);

	//FQuat BoneQuat({ 1,0,0 }, 0);
	//NewBoneTM.SetRotation(BoneQuat);

	//NewBoneTM *= BoneTM;

	//float BlendWeight = 1;
	//TArray<FBoneTransform> OutBoneTransforms;
	//OutBoneTransforms.Add(FBoneTransform(BoneIndex, NewBoneTM));
	//CSPose.Pose.LocalBlendCSBoneTransforms(OutBoneTransforms, BlendWeight);

	//FCSPose<FCompactPose>::ConvertComponentPosesToLocalPoses(MoveTemp(CSPose.Pose), Output.Pose);
}

void FZLWAnimNode::UpdateP(FPoseContext& SourceData) {

	USkeleton* Skeleton = SourceData.AnimInstanceProxy->GetSkeleton();

	for (int i = 0; i < BoneTM_72_list.Num(); i++)
	{
		char buff[16];
		snprintf(buff, 16, "P_%06d", i);
		FName CurveName(buff);
		SmartName::UID_Type NameUID = Skeleton->GetUIDByName(USkeleton::AnimCurveMappingName, CurveName);
		if (NameUID != SmartName::MaxUID)
		{
			float NewValue = BoneTM_72_list[i];
			float CurrentValue = SourceData.Curve.Get(NameUID);
			if (ApplyMode == EZLWCurveApplyMode::WeightedMovingAverage)
			{
				float& LastValue = BoneTM_72_list_last[i];
				ProcessCurveWMAOperation(SourceData, NameUID, CurrentValue, NewValue, LastValue);
			}
			else
			{
				ProcessCurveOperation(ApplyMode, SourceData, NameUID, CurrentValue, NewValue);
			}
		}
	}
}