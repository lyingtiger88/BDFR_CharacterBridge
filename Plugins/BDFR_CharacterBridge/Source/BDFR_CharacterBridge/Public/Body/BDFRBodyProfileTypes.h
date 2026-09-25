#pragma once

#include "CoreMinimal.h"
#include "BDFRBodyProfileTypes.generated.h"

UENUM(BlueprintType)
enum class EBDFRBodyRegionType : uint8
{
    Muscle,
    SoftTissue,
    FatPad,
    Breast,
    Glute,
    Abdomen,
    Custom
};

UENUM(BlueprintType)
enum class EBDFRBodySide : uint8
{
    Center,
    Left,
    Right
};

USTRUCT(BlueprintType)
struct FBDFRBodyRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FName Id = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FString Label;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FName Name = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    EBDFRBodyRegionType Type = EBDFRBodyRegionType::Custom;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    EBDFRBodySide Side = EBDFRBodySide::Center;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FName AnchorA = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FName AnchorB = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FVector SourceLocalPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FQuat SourceLocalRotation = FQuat::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FVector SourceWorldPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FQuat SourceWorldRotation = FQuat::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FVector RadiusCm = FVector(5.0, 5.0, 5.0);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    float MassKg = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    float Stiffness = 0.65f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    float Damping = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    float Compliance = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    float ActivationScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    float MaxBulge = 0.12f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    float CollisionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    bool bEnabled = true;
};

USTRUCT(BlueprintType)
struct FBDFRBodyProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    int32 Version = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FString Units = TEXT("cm");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FString SourceCoordinateSystem = TEXT("DazStudio");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    FString CharacterLabel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BDFR|Body")
    TArray<FBDFRBodyRegion> Regions;

    bool IsUsable() const
    {
        return Version > 0 && Regions.Num() > 0;
    }
};
