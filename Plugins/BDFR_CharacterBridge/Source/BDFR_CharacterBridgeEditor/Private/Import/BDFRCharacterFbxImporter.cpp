#include "Import/BDFRCharacterFbxImporter.h"

#include "Adapters/Daz/BDFRDazBodyProfileAdapter.h"
#include "Adapters/Daz/BDFRDazMorphAdapter.h"
#include "AssetImportTask.h"
#include "AssetToolsModule.h"
#include "Character/BDFRCharacterAssetUserData.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/MorphTarget.h"
#include "Animation/Skeleton.h"
#include "Factories/FbxFactory.h"
#include "Factories/FbxImportUI.h"
#include "Factories/FbxSkeletalMeshImportData.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
    bool ValidateRequest(
        const FBDFRCharacterFbxImportRequest& Request,
        FBDFRCharacterFbxImportResult& Result)
    {
        if (Request.FbxFilename.IsEmpty() ||
            !IFileManager::Get().FileExists(*Request.FbxFilename))
        {
            Result.Errors.Add(FString::Printf(
                TEXT("FBX file does not exist: %s"),
                *Request.FbxFilename));
            return false;
        }

        if (Request.DtuFilename.IsEmpty() ||
            !IFileManager::Get().FileExists(*Request.DtuFilename))
        {
            Result.Errors.Add(FString::Printf(
                TEXT("DTU file does not exist: %s"),
                *Request.DtuFilename));
            return false;
        }

        if (!Request.DestinationPath.StartsWith(TEXT("/Game/")))
        {
            Result.Errors.Add(FString::Printf(
                TEXT("DestinationPath must be a /Game/ content path: %s"),
                *Request.DestinationPath));
            return false;
        }

        return true;
    }
}

FBDFRCharacterFbxImportResult FBDFRCharacterFbxImporter::ImportCharacter(
    const FBDFRCharacterFbxImportRequest& Request)
{
    FBDFRCharacterFbxImportResult Result;

    if (!ValidateRequest(Request, Result))
    {
        return Result;
    }

    FString DtuJson;
    if (!FFileHelper::LoadFileToString(DtuJson, *Request.DtuFilename))
    {
        Result.Errors.Add(FString::Printf(
            TEXT("Failed to read DTU file: %s"),
            *Request.DtuFilename));
        return Result;
    }

    UFbxFactory* FbxFactory = NewObject<UFbxFactory>();
    if (!FbxFactory || !FbxFactory->ImportUI)
    {
        Result.Errors.Add(TEXT("Failed to initialize Unreal FBX factory/import settings."));
        return Result;
    }

    FbxFactory->SetDetectImportTypeOnImport(false);

    UFbxImportUI* ImportUI = FbxFactory->ImportUI;
    ImportUI->bAutomatedImportShouldDetectType = false;
    ImportUI->bImportAsSkeletal = true;
    ImportUI->bImportMesh = true;
    ImportUI->bImportAnimations = false;
    ImportUI->bImportMaterials = false;
    ImportUI->bImportTextures = false;
    ImportUI->MeshTypeToImport = FBXIT_SkeletalMesh;
    ImportUI->Skeleton = Request.ExistingSkeleton;

    if (!ImportUI->SkeletalMeshImportData)
    {
        Result.Errors.Add(TEXT("UFbxImportUI did not provide SkeletalMeshImportData."));
        return Result;
    }

    ImportUI->SkeletalMeshImportData->bImportMorphTargets = true;
    ImportUI->SkeletalMeshImportData->bImportMeshesInBoneHierarchy =
        Request.bImportMeshesInBoneHierarchy;
    ImportUI->SkeletalMeshImportData->bUseT0AsRefPose =
        Request.bUseT0AsReferencePose;
    ImportUI->SkeletalMeshImportData->bForceFrontXAxis =
        Request.bForceFrontXAxis;
    ImportUI->SkeletalMeshImportData->bConvertScene = true;

    UAssetImportTask* ImportTask = NewObject<UAssetImportTask>();
    ImportTask->Filename = Request.FbxFilename;
    ImportTask->DestinationPath = Request.DestinationPath;
    ImportTask->bAutomated = true;
    ImportTask->bAsync = false;
    ImportTask->bReplaceExisting = Request.bReplaceExisting;
    ImportTask->bReplaceExistingSettings = false;
    ImportTask->bSave = false;
    ImportTask->Factory = FbxFactory;
    ImportTask->Options = ImportUI;

    TArray<UAssetImportTask*> ImportTasks;
    ImportTasks.Add(ImportTask);

    FAssetToolsModule::GetModule().Get().ImportAssetTasks(ImportTasks);

    for (UObject* ImportedObject : ImportTask->GetObjects())
    {
        if (USkeletalMesh* ImportedMesh = Cast<USkeletalMesh>(ImportedObject))
        {
            Result.SkeletalMesh = ImportedMesh;
            break;
        }
    }

    if (!Result.SkeletalMesh)
    {
        Result.Errors.Add(TEXT("FBX import completed without producing a USkeletalMesh."));
        return Result;
    }

    const TArray<FName> ImportedMorphNames =
        CollectImportedMorphNames(Result.SkeletalMesh);

    if (!FBDFRDazMorphAdapter::BuildTransferPlanFromDtuJson(
            DtuJson,
            ImportedMorphNames,
            Result.MorphTransferPlan,
            Request.bUseInternalMorphNames))
    {
        Result.Errors.Add(TEXT("Skeletal Mesh imported, but the DTU morph manifest could not be parsed."));
        return Result;
    }

    if (Result.MorphTransferPlan.Morphs.Num() == 0)
    {
        Result.Warnings.Add(TEXT("No morph targets were found in the DTU manifest or imported Skeletal Mesh."));
    }

    if (Result.MorphTransferPlan.MissingRequestedMorphs.Num() > 0)
    {
        FString MissingNames;
        for (const FName Missing : Result.MorphTransferPlan.MissingRequestedMorphs)
        {
            if (!MissingNames.IsEmpty())
            {
                MissingNames += TEXT(", ");
            }
            MissingNames += Missing.ToString();
        }

        Result.Warnings.Add(FString::Printf(
            TEXT("Requested DTU morphs were not found on the imported Skeletal Mesh: %s"),
            *MissingNames));

        if (Request.bStrictMorphValidation)
        {
            Result.Errors.Add(TEXT("Strict morph validation failed."));
        }
    }

    Result.ResolvedBodyProfileFilename =
        ResolveBodyProfileFilename(Request);

    if (!Result.ResolvedBodyProfileFilename.IsEmpty())
    {
        FString BodyJson;

        if (FFileHelper::LoadFileToString(
                BodyJson,
                *Result.ResolvedBodyProfileFilename))
        {
            TArray<FString> BodyWarnings;

            if (FBDFRDazBodyProfileAdapter::ParseBodyProfileJson(
                    BodyJson,
                    Result.BodyProfile,
                    &BodyWarnings))
            {
                Result.bHasBodyProfile =
                    Result.BodyProfile.IsUsable();

                Result.Warnings.Append(BodyWarnings);

                ValidateBodyProfileAnchors(
                    Result.SkeletalMesh,
                    Result.BodyProfile,
                    Result.Warnings);
            }
            else
            {
                Result.Warnings.Add(FString::Printf(
                    TEXT("BDFR body profile could not be parsed: %s"),
                    *Result.ResolvedBodyProfileFilename));
            }
        }
        else
        {
            Result.Warnings.Add(FString::Printf(
                TEXT("BDFR body profile could not be read: %s"),
                *Result.ResolvedBodyProfileFilename));
        }
    }

    AttachOrUpdateAssetUserData(
        Result.SkeletalMesh,
        Request,
        Result.MorphTransferPlan,
        Result.bHasBodyProfile ? &Result.BodyProfile : nullptr,
        Result.ResolvedBodyProfileFilename);

    Result.SkeletalMesh->MarkPackageDirty();

    Result.bSuccess =
        Result.SkeletalMesh != nullptr &&
        Result.Errors.Num() == 0;

    return Result;
}

TArray<FName> FBDFRCharacterFbxImporter::CollectImportedMorphNames(
    const USkeletalMesh* SkeletalMesh)
{
    TArray<FName> MorphNames;

    if (!SkeletalMesh)
    {
        return MorphNames;
    }

    for (const TObjectPtr<UMorphTarget>& MorphTarget :
         SkeletalMesh->GetMorphTargets())
    {
        if (MorphTarget)
        {
            MorphNames.AddUnique(MorphTarget->GetFName());
        }
    }

    return MorphNames;
}

FString FBDFRCharacterFbxImporter::ResolveBodyProfileFilename(
    const FBDFRCharacterFbxImportRequest& Request)
{
    if (!Request.BodyProfileFilename.IsEmpty())
    {
        return IFileManager::Get().FileExists(*Request.BodyProfileFilename)
            ? FPaths::ConvertRelativePathToFull(Request.BodyProfileFilename)
            : FString();
    }

    if (!Request.bAutoDetectBodyProfile ||
        Request.DtuFilename.IsEmpty())
    {
        return FString();
    }

    const FString Candidate =
        FPaths::ChangeExtension(
            Request.DtuFilename,
            TEXT("bdfrbody.json"));

    return IFileManager::Get().FileExists(*Candidate)
        ? FPaths::ConvertRelativePathToFull(Candidate)
        : FString();
}

void FBDFRCharacterFbxImporter::ValidateBodyProfileAnchors(
    const USkeletalMesh* SkeletalMesh,
    const FBDFRBodyProfile& BodyProfile,
    TArray<FString>& OutWarnings)
{
    if (!SkeletalMesh)
    {
        return;
    }

    const FReferenceSkeleton& RefSkeleton =
        SkeletalMesh->GetRefSkeleton();

    for (const FBDFRBodyRegion& Region : BodyProfile.Regions)
    {
        if (Region.AnchorA != NAME_None &&
            RefSkeleton.FindRawBoneIndex(Region.AnchorA) == INDEX_NONE)
        {
            OutWarnings.Add(FString::Printf(
                TEXT("Body region '%s' Anchor A '%s' was not found in the imported skeleton."),
                *Region.Name.ToString(),
                *Region.AnchorA.ToString()));
        }

        if (Region.AnchorB != NAME_None &&
            RefSkeleton.FindRawBoneIndex(Region.AnchorB) == INDEX_NONE)
        {
            OutWarnings.Add(FString::Printf(
                TEXT("Body region '%s' Anchor B '%s' was not found in the imported skeleton."),
                *Region.Name.ToString(),
                *Region.AnchorB.ToString()));
        }
    }
}

void FBDFRCharacterFbxImporter::AttachOrUpdateAssetUserData(
    USkeletalMesh* SkeletalMesh,
    const FBDFRCharacterFbxImportRequest& Request,
    const FBDFRMorphTransferPlan& MorphTransferPlan,
    const FBDFRBodyProfile* BodyProfile,
    const FString& BodyProfileFilename)
{
    if (!SkeletalMesh)
    {
        return;
    }

    SkeletalMesh->Modify();

    UBDFRCharacterAssetUserData* UserData =
        Cast<UBDFRCharacterAssetUserData>(
            SkeletalMesh->GetAssetUserDataOfClass(
                UBDFRCharacterAssetUserData::StaticClass()));

    if (!UserData)
    {
        UserData = NewObject<UBDFRCharacterAssetUserData>(
            SkeletalMesh,
            NAME_None,
            RF_Transactional);

        SkeletalMesh->AddAssetUserData(UserData);
    }

    UserData->Modify();
    UserData->SourceFbxFile =
        FPaths::ConvertRelativePathToFull(Request.FbxFilename);
    UserData->SourceDtuFile =
        FPaths::ConvertRelativePathToFull(Request.DtuFilename);
    UserData->MorphTransferPlan = MorphTransferPlan;

    UserData->SourceBodyProfileFile =
        BodyProfileFilename;

    UserData->bHasBodyProfile =
        BodyProfile != nullptr;

    UserData->BodyProfile =
        BodyProfile
            ? *BodyProfile
            : FBDFRBodyProfile();
}
