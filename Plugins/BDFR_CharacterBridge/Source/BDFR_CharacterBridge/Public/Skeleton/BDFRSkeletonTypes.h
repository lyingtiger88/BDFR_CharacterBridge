#pragma once

#include "CoreMinimal.h"
#include "BDFRSkeletonTypes.generated.h"

UENUM(BlueprintType)
enum class EBDFRCharacterSourceProfile : uint8
{
    Unknown,
    DazGenesis8,
    DazGenesis81,
    DazGenesis9
};

UENUM(BlueprintType)
enum class EBDFRBoneRetargetPolicy : uint8
{
    Full,
    RotationOnly,
    TranslationOnly,
    DeformationOnly,
    Excluded
};

USTRUCT(BlueprintType)
struct FBDFRBoneMapping
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    FName SourceBone = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    FName CanonicalBone = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    EBDFRBoneRetargetPolicy RetargetPolicy = EBDFRBoneRetargetPolicy::Full;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    bool bRequired = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    float RotationWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FBDFRRetargetChainDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    FName ChainName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    FName SourceStartBone = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    FName SourceEndBone = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    FName CanonicalStartBone = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    FName CanonicalEndBone = NAME_None;
};

USTRUCT(BlueprintType)
struct FBDFRSkeletonProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    EBDFRCharacterSourceProfile SourceProfile = EBDFRCharacterSourceProfile::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    FName ProfileName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    FName SourceRetargetRoot = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    FName CanonicalRetargetRoot = TEXT("pelvis");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    bool bPreserveUnmappedBones = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    bool bPreserveTwistBones = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    TArray<FBDFRBoneMapping> BoneMappings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    TArray<FBDFRRetargetChainDefinition> RetargetChains;
};

USTRUCT(BlueprintType)
struct FBDFRSkeletonValidationReport
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    TArray<FName> MissingRequiredBones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    TArray<FName> UnmappedBones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    TArray<FName> TwistBones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|Skeleton")
    bool bRetargetRootFound = false;

    bool IsValid() const
    {
        return bRetargetRootFound && MissingRequiredBones.Num() == 0;
    }
};
