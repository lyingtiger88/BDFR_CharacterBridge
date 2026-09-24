#include "Retarget/BDFRRetargetAssetGenerator.h"

#include "Adapters/Daz/BDFRDazSkeletonAdapter.h"
#include "AssetToolsModule.h"
#include "Character/BDFRCharacterAssetUserData.h"
#include "Engine/SkeletalMesh.h"
#include "Rig/IKRigDefinition.h"
#include "RigEditor/IKRigController.h"
#include "RigEditor/IKRigDefinitionFactory.h"
#include "Retargeter/IKRetargeter.h"
#include "RetargetEditor/IKRetargetFactory.h"
#include "RetargetEditor/IKRetargeterController.h"
#include "RetargetEditor/IKRetargeterPoseGenerator.h"
#include "Skeleton/BDFRReferencePoseConverter.h"
#include "UObject/SoftObjectPath.h"

namespace
{
    const FName BDFRCallingContext(TEXT("BDFR_CharacterBridge"));

    bool IsGamePath(const FString& Path)
    {
        return Path.StartsWith(TEXT("/Game/"));
    }

    FString MakeDefaultAssetName(
        const TCHAR* Prefix,
        const USkeletalMesh* Mesh)
    {
        return FString::Printf(
            TEXT("%s_%s"),
            Prefix,
            Mesh ? *Mesh->GetName() : TEXT("Character"));
    }

    FString MakeRetargeterName(
        const USkeletalMesh* Source,
        const USkeletalMesh* Target)
    {
        return FString::Printf(
            TEXT("RTG_BDFR_%s_to_%s"),
            Source ? *Source->GetName() : TEXT("Source"),
            Target ? *Target->GetName() : TEXT("Target"));
    }

    bool HasBone(const TSet<FName>& Bones, const FName Bone)
    {
        return Bone != NAME_None && Bones.Contains(Bone);
    }

    FName ResolveTargetChainEnd(
        const FName ChainName,
        const FName Preferred,
        const TSet<FName>& TargetBones)
    {
        // UE5 Manny/Quinn commonly extends the torso through spine_05.
        // If available, include it in the Spine retarget chain even though
        // the BDFR canonical mapping currently tops out at spine_04.
        if (ChainName == FName(TEXT("Spine")) &&
            TargetBones.Contains(FName(TEXT("spine_05"))))
        {
            return FName(TEXT("spine_05"));
        }

        return Preferred;
    }

    UIKRigDefinition* CreateIKRigAsset(
        const FString& DestinationPath,
        const FString& AssetName,
        const bool bOverwriteExisting)
    {
        IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
        UIKRigDefinitionFactory* Factory =
            NewObject<UIKRigDefinitionFactory>();

        return Cast<UIKRigDefinition>(
            AssetTools.CreateAsset(
                AssetName,
                DestinationPath,
                UIKRigDefinition::StaticClass(),
                Factory,
                BDFRCallingContext,
                bOverwriteExisting));
    }

    UIKRetargeter* CreateRetargeterAsset(
        const FString& DestinationPath,
        const FString& AssetName,
        const bool bOverwriteExisting)
    {
        IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
        UIKRetargetFactory* Factory = NewObject<UIKRetargetFactory>();

        return Cast<UIKRetargeter>(
            AssetTools.CreateAsset(
                AssetName,
                DestinationPath,
                UIKRetargeter::StaticClass(),
                Factory,
                BDFRCallingContext,
                bOverwriteExisting));
    }

    bool ConfigureIKRig(
        UIKRigDefinition* IKRig,
        USkeletalMesh* Mesh,
        const FName RetargetRoot,
        const TArray<FBDFRResolvedRetargetChain>& Chains,
        const bool bUseSourceBones,
        const bool bTryAutoFBIK,
        TArray<FString>& OutWarnings,
        TArray<FString>& OutErrors)
    {
        if (!IKRig || !Mesh)
        {
            OutErrors.Add(TEXT("Invalid IK Rig or Skeletal Mesh."));
            return false;
        }

        UIKRigController* Controller =
            UIKRigController::GetController(IKRig);

        if (!Controller)
        {
            OutErrors.Add(FString::Printf(
                TEXT("Could not acquire IK Rig controller for %s."),
                *IKRig->GetName()));
            return false;
        }

        if (!Controller->SetSkeletalMesh(Mesh))
        {
            OutErrors.Add(FString::Printf(
                TEXT("IK Rig %s rejected Skeletal Mesh %s."),
                *IKRig->GetName(),
                *Mesh->GetName()));
            return false;
        }

        if (!Controller->SetRetargetRoot(RetargetRoot))
        {
            OutErrors.Add(FString::Printf(
                TEXT("Could not set retarget root '%s' on IK Rig %s."),
                *RetargetRoot.ToString(),
                *IKRig->GetName()));
            return false;
        }

        for (const FBDFRResolvedRetargetChain& Chain : Chains)
        {
            const FName StartBone =
                bUseSourceBones
                    ? Chain.SourceStartBone
                    : Chain.TargetStartBone;

            const FName EndBone =
                bUseSourceBones
                    ? Chain.SourceEndBone
                    : Chain.TargetEndBone;

            const FName AddedChain =
                Controller->AddRetargetChain(
                    Chain.ChainName,
                    StartBone,
                    EndBone,
                    NAME_None);

            if (AddedChain == NAME_None)
            {
                OutWarnings.Add(FString::Printf(
                    TEXT("IK Rig %s could not add chain %s (%s -> %s)."),
                    *IKRig->GetName(),
                    *Chain.ChainName.ToString(),
                    *StartBone.ToString(),
                    *EndBone.ToString()));
            }
        }

        if (bTryAutoFBIK && !Controller->ApplyAutoFBIK())
        {
            OutWarnings.Add(FString::Printf(
                TEXT("UE 5.8 Auto FBIK did not recognize %s. Retarget chains remain valid; BDFR-specific Daz FBIK setup can be generated later."),
                *Mesh->GetName()));
        }

        IKRig->MarkPackageDirty();
        return true;
    }

    void PersistGeneratedAssets(
        USkeletalMesh* SourceMesh,
        const EBDFRCharacterSourceProfile SourceProfile,
        UIKRigDefinition* SourceIKRig,
        UIKRigDefinition* TargetIKRig,
        UIKRetargeter* Retargeter,
        const FName RetargetPoseName)
    {
        if (!SourceMesh)
        {
            return;
        }

        SourceMesh->Modify();

        UBDFRCharacterAssetUserData* UserData =
            Cast<UBDFRCharacterAssetUserData>(
                SourceMesh->GetAssetUserDataOfClass(
                    UBDFRCharacterAssetUserData::StaticClass()));

        if (!UserData)
        {
            UserData = NewObject<UBDFRCharacterAssetUserData>(
                SourceMesh,
                NAME_None,
                RF_Transactional);

            SourceMesh->AddAssetUserData(UserData);
        }

        UserData->Modify();
        UserData->SourceProfile = SourceProfile;
        UserData->GeneratedSourceIKRig =
            FSoftObjectPath(SourceIKRig);
        UserData->GeneratedTargetIKRig =
            FSoftObjectPath(TargetIKRig);
        UserData->GeneratedIKRetargeter =
            FSoftObjectPath(Retargeter);
        UserData->GeneratedTargetRetargetPose =
            RetargetPoseName;

        SourceMesh->MarkPackageDirty();
    }
}

TArray<FName> FBDFRRetargetAssetGenerator::CollectRawBoneNames(
    const USkeletalMesh* Mesh)
{
    TArray<FName> Names;

    if (!Mesh)
    {
        return Names;
    }

    const FReferenceSkeleton& RefSkeleton = Mesh->GetRefSkeleton();
    const TArray<FMeshBoneInfo>& BoneInfo =
        RefSkeleton.GetRawRefBoneInfo();

    Names.Reserve(BoneInfo.Num());

    for (const FMeshBoneInfo& Bone : BoneInfo)
    {
        Names.Add(Bone.Name);
    }

    return Names;
}

TArray<FBDFRResolvedRetargetChain>
FBDFRRetargetAssetGenerator::ResolveCompatibleChains(
    const FBDFRSkeletonProfile& Profile,
    const TArray<FName>& SourceBones,
    const TArray<FName>& TargetBones,
    TArray<FString>& OutWarnings)
{
    TArray<FBDFRResolvedRetargetChain> ResolvedChains;

    TSet<FName> SourceBoneSet;
    for (const FName Bone : SourceBones)
    {
        SourceBoneSet.Add(Bone);
    }

    TSet<FName> TargetBoneSet;
    for (const FName Bone : TargetBones)
    {
        TargetBoneSet.Add(Bone);
    }

    for (const FBDFRRetargetChainDefinition& Chain :
         Profile.RetargetChains)
    {
        const FName TargetEnd =
            ResolveTargetChainEnd(
                Chain.ChainName,
                Chain.CanonicalEndBone,
                TargetBoneSet);

        const bool bSourceValid =
            HasBone(SourceBoneSet, Chain.SourceStartBone) &&
            HasBone(SourceBoneSet, Chain.SourceEndBone);

        const bool bTargetValid =
            HasBone(TargetBoneSet, Chain.CanonicalStartBone) &&
            HasBone(TargetBoneSet, TargetEnd);

        if (!bSourceValid || !bTargetValid)
        {
            OutWarnings.Add(FString::Printf(
                TEXT("Skipping retarget chain %s. Source [%s -> %s] valid=%s, Target [%s -> %s] valid=%s."),
                *Chain.ChainName.ToString(),
                *Chain.SourceStartBone.ToString(),
                *Chain.SourceEndBone.ToString(),
                bSourceValid ? TEXT("true") : TEXT("false"),
                *Chain.CanonicalStartBone.ToString(),
                *TargetEnd.ToString(),
                bTargetValid ? TEXT("true") : TEXT("false")));

            continue;
        }

        FBDFRResolvedRetargetChain Resolved;
        Resolved.ChainName = Chain.ChainName;
        Resolved.SourceStartBone = Chain.SourceStartBone;
        Resolved.SourceEndBone = Chain.SourceEndBone;
        Resolved.TargetStartBone = Chain.CanonicalStartBone;
        Resolved.TargetEndBone = TargetEnd;
        ResolvedChains.Add(Resolved);
    }

    return ResolvedChains;
}

FBDFRRetargetAssetGenerationResult
FBDFRRetargetAssetGenerator::Generate(
    const FBDFRRetargetAssetGenerationRequest& Request)
{
    FBDFRRetargetAssetGenerationResult Result;

    if (!Request.SourceMesh || !Request.TargetMesh)
    {
        Result.Errors.Add(
            TEXT("SourceMesh and TargetMesh are required."));
        return Result;
    }

    if (!IsGamePath(Request.DestinationPath))
    {
        Result.Errors.Add(FString::Printf(
            TEXT("DestinationPath must be under /Game/: %s"),
            *Request.DestinationPath));
        return Result;
    }

    const TArray<FName> SourceBones =
        CollectRawBoneNames(Request.SourceMesh);
    const TArray<FName> TargetBones =
        CollectRawBoneNames(Request.TargetMesh);

    EBDFRCharacterSourceProfile SourceProfile =
        Request.SourceProfile;

    if (SourceProfile ==
        EBDFRCharacterSourceProfile::Unknown)
    {
        const FString DetectionHint =
            !Request.SourceAssetHint.IsEmpty()
                ? Request.SourceAssetHint
                : Request.SourceMesh->GetName();

        SourceProfile =
            FBDFRDazSkeletonAdapter::DetectSourceProfile(
                SourceBones,
                DetectionHint);
    }

    if (SourceProfile ==
        EBDFRCharacterSourceProfile::Unknown)
    {
        Result.Errors.Add(
            TEXT("Could not identify a supported Daz Genesis 8/8.1/9 source profile."));
        return Result;
    }

    const FBDFRSkeletonProfile Profile =
        FBDFRDazSkeletonAdapter::BuildProfile(
            SourceProfile,
            SourceBones);

    Result.ResolvedChains =
        ResolveCompatibleChains(
            Profile,
            SourceBones,
            TargetBones,
            Result.Warnings);

    if (Result.ResolvedChains.Num() == 0)
    {
        Result.Errors.Add(
            TEXT("No compatible source/target retarget chains were resolved."));
        return Result;
    }

    const FString SourceIKRigName =
        Request.SourceIKRigName.IsEmpty()
            ? MakeDefaultAssetName(
                TEXT("IK_BDFR"),
                Request.SourceMesh)
            : Request.SourceIKRigName;

    const FString TargetIKRigName =
        Request.TargetIKRigName.IsEmpty()
            ? MakeDefaultAssetName(
                TEXT("IK_BDFR"),
                Request.TargetMesh)
            : Request.TargetIKRigName;

    const FString RetargeterName =
        Request.RetargeterName.IsEmpty()
            ? MakeRetargeterName(
                Request.SourceMesh,
                Request.TargetMesh)
            : Request.RetargeterName;

    Result.SourceIKRig =
        CreateIKRigAsset(
            Request.DestinationPath,
            SourceIKRigName,
            Request.bOverwriteExisting);

    Result.TargetIKRig =
        CreateIKRigAsset(
            Request.DestinationPath,
            TargetIKRigName,
            Request.bOverwriteExisting);

    if (!Result.SourceIKRig || !Result.TargetIKRig)
    {
        Result.Errors.Add(
            TEXT("Failed to create source and/or target IK Rig asset."));
        return Result;
    }

    const bool bSourceIKRigConfigured =
        ConfigureIKRig(
            Result.SourceIKRig,
            Request.SourceMesh,
            Profile.SourceRetargetRoot,
            Result.ResolvedChains,
            true,
            Request.bTryAutoFBIK,
            Result.Warnings,
            Result.Errors);

    const bool bTargetIKRigConfigured =
        ConfigureIKRig(
            Result.TargetIKRig,
            Request.TargetMesh,
            Profile.CanonicalRetargetRoot,
            Result.ResolvedChains,
            false,
            Request.bTryAutoFBIK,
            Result.Warnings,
            Result.Errors);

    if (!bSourceIKRigConfigured ||
        !bTargetIKRigConfigured)
    {
        return Result;
    }

    Result.Retargeter =
        CreateRetargeterAsset(
            Request.DestinationPath,
            RetargeterName,
            Request.bOverwriteExisting);

    if (!Result.Retargeter)
    {
        Result.Errors.Add(
            TEXT("Failed to create IK Retargeter asset."));
        return Result;
    }

    UIKRetargeterController* RetargetController =
        UIKRetargeterController::GetController(
            Result.Retargeter);

    if (!RetargetController)
    {
        Result.Errors.Add(
            TEXT("Could not acquire IK Retargeter controller."));
        return Result;
    }

    RetargetController->SetIKRig(
        ERetargetSourceOrTarget::Source,
        Result.SourceIKRig);

    RetargetController->SetIKRig(
        ERetargetSourceOrTarget::Target,
        Result.TargetIKRig);

    RetargetController->SetPreviewMesh(
        ERetargetSourceOrTarget::Source,
        Request.SourceMesh);

    RetargetController->SetPreviewMesh(
        ERetargetSourceOrTarget::Target,
        Request.TargetMesh);

    RetargetController->AddDefaultOps();

    RetargetController->AssignIKRigToAllOps(
        ERetargetSourceOrTarget::Source,
        Result.SourceIKRig);

    RetargetController->AssignIKRigToAllOps(
        ERetargetSourceOrTarget::Target,
        Result.TargetIKRig);

    RetargetController->AutoMapChains(
        EAutoMapChainType::Exact,
        true,
        NAME_None);

    const FName RequestedPoseName(TEXT("BDFR_Aligned"));
    Result.TargetRetargetPoseName =
        RetargetController->CreateRetargetPose(
            RequestedPoseName,
            ERetargetSourceOrTarget::Target);

    if (Result.TargetRetargetPoseName != NAME_None)
    {
        RetargetController->SetCurrentRetargetPose(
            Result.TargetRetargetPoseName,
            ERetargetSourceOrTarget::Target);

        if (Request.bUseUnrealAutoAlign)
        {
            RetargetController->AutoAlignAllBones(
                ERetargetSourceOrTarget::Target,
                ERetargetAutoAlignMethod::ChainToChain);
        }
        else if (Request.bApplyBDFRReferencePosePlan)
        {
            const FBDFRReferencePoseConversionPlan PosePlan =
                FBDFRReferencePoseConverter::BuildPlan(
                    Request.SourceMesh->GetRefSkeleton(),
                    Request.TargetMesh->GetRefSkeleton(),
                    Profile);

            for (const FBDFRPoseBoneCorrection& Correction :
                 PosePlan.BoneCorrections)
            {
                if (Correction.RetargetPolicy ==
                        EBDFRBoneRetargetPolicy::DeformationOnly ||
                    Correction.RetargetPolicy ==
                        EBDFRBoneRetargetPolicy::Excluded)
                {
                    continue;
                }

                RetargetController
                    ->SetRotationOffsetForRetargetPoseBone(
                        Correction.TargetBone,
                        Correction.LocalReferenceRotationDelta,
                        ERetargetSourceOrTarget::Target);
            }

            if (PosePlan.bHasValidRetargetRoots)
            {
                RetargetController
                    ->SetRootOffsetInRetargetPose(
                        PosePlan.TargetRootTranslationDelta,
                        ERetargetSourceOrTarget::Target);
            }

            Result.Warnings.Add(
                TEXT("Applied BDFR reference-pose deltas directly. UE 5.8 ChainToChain Auto Align is recommended for rigs with different local bone axes."));
        }
    }
    else
    {
        Result.Warnings.Add(
            TEXT("Retargeter was created, but the BDFR target retarget pose could not be created."));
    }

    Result.Retargeter->MarkPackageDirty();

    PersistGeneratedAssets(
        Request.SourceMesh,
        SourceProfile,
        Result.SourceIKRig,
        Result.TargetIKRig,
        Result.Retargeter,
        Result.TargetRetargetPoseName);

    Result.bSuccess =
        Result.Errors.Num() == 0 &&
        Result.SourceIKRig != nullptr &&
        Result.TargetIKRig != nullptr &&
        Result.Retargeter != nullptr;

    return Result;
}
