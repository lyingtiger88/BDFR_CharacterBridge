#pragma once

#include "CoreMinimal.h"
#include "BDFRMorphTypes.generated.h"

UENUM(BlueprintType)
enum class EBDFRMorphSemantic : uint8
{
    Unknown,
    Body,
    Head,
    Expression,
    Viseme,
    Corrective,
    Control
};

USTRUCT(BlueprintType)
struct FBDFRMorphKey
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    float Angle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    float Value = 0.0f;
};

USTRUCT(BlueprintType)
struct FBDFRJointDrivenMorphLink
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    FName Bone = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    FName Morph = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    FName Axis = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    float Scalar = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    float Alpha = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    TArray<FBDFRMorphKey> Keys;
};

USTRUCT(BlueprintType)
struct FBDFRMorphDescriptor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    FName SourceInternalName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    FString SourceLabel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    FName ImportedMorphName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    FName TargetMorphName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    EBDFRMorphSemantic Semantic = EBDFRMorphSemantic::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    bool bJointDriven = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    bool bFoundOnImportedMesh = false;
};

USTRUCT(BlueprintType)
struct FBDFRMorphTransferPlan
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    TArray<FBDFRMorphDescriptor> Morphs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    TArray<FBDFRJointDrivenMorphLink> JointDrivenLinks;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    TArray<FName> MissingRequestedMorphs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BDFR|Morph")
    TArray<FName> UnlistedImportedMorphs;

    bool IsUsable() const
    {
        return Morphs.Num() > 0 && MissingRequestedMorphs.Num() == 0;
    }
};
