#include "Import/BDFRCharacterFbxImporter.h"

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

    AttachOrUpdateAssetUserData(
        Result.SkeletalMesh,
        Request,
        Result.MorphTransferPlan);

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

void FBDFRCharacterFbxImporter::AttachOrUpdateAssetUserData(
    USkeletalMesh* SkeletalMesh,
    const FBDFRCharacterFbxImportRequest& Request,
    const FBDFRMorphTransferPlan& MorphTransferPlan)
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
    UserData->SourceFbxFile = FPaths::ConvertRelativePathToFull(Request.FbxFilename);
    UserData->SourceDtuFile = FPaths::ConvertRelativePathToFull(Request.DtuFilename);
    UserData->MorphTransferPlan = MorphTransferPlan;
}
