#include "Adapters/Daz/BDFRDazBodyProfileAdapter.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    FVector ReadVec3(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field)
    {
        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        if (!Object.IsValid() ||
            !Object->TryGetArrayField(Field, Values) ||
            !Values ||
            Values->Num() < 3)
        {
            return FVector::ZeroVector;
        }

        return FVector(
            static_cast<float>((*Values)[0]->AsNumber()),
            static_cast<float>((*Values)[1]->AsNumber()),
            static_cast<float>((*Values)[2]->AsNumber()));
    }

    FQuat ReadQuat(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field)
    {
        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        if (!Object.IsValid() ||
            !Object->TryGetArrayField(Field, Values) ||
            !Values ||
            Values->Num() < 4)
        {
            return FQuat::Identity;
        }

        FQuat Q(
            static_cast<float>((*Values)[0]->AsNumber()),
            static_cast<float>((*Values)[1]->AsNumber()),
            static_cast<float>((*Values)[2]->AsNumber()),
            static_cast<float>((*Values)[3]->AsNumber()));

        Q.Normalize();
        return Q;
    }

    FString GetString(
        const TSharedPtr<FJsonObject>& Object,
        const TCHAR* Field,
        const FString& Default = FString())
    {
        FString Value;
        return Object.IsValid() && Object->TryGetStringField(Field, Value)
            ? Value
            : Default;
    }

    float GetFloat(
        const TSharedPtr<FJsonObject>& Object,
        const TCHAR* Field,
        const float Default)
    {
        double Value = Default;
        if (Object.IsValid())
        {
            Object->TryGetNumberField(Field, Value);
        }
        return static_cast<float>(Value);
    }
}

EBDFRBodyRegionType FBDFRDazBodyProfileAdapter::ParseRegionType(
    const FString& Value)
{
    if (Value.Equals(TEXT("Muscle"), ESearchCase::IgnoreCase))
        return EBDFRBodyRegionType::Muscle;
    if (Value.Equals(TEXT("SoftTissue"), ESearchCase::IgnoreCase))
        return EBDFRBodyRegionType::SoftTissue;
    if (Value.Equals(TEXT("FatPad"), ESearchCase::IgnoreCase))
        return EBDFRBodyRegionType::FatPad;
    if (Value.Equals(TEXT("Breast"), ESearchCase::IgnoreCase))
        return EBDFRBodyRegionType::Breast;
    if (Value.Equals(TEXT("Glute"), ESearchCase::IgnoreCase))
        return EBDFRBodyRegionType::Glute;
    if (Value.Equals(TEXT("Abdomen"), ESearchCase::IgnoreCase))
        return EBDFRBodyRegionType::Abdomen;
    return EBDFRBodyRegionType::Custom;
}

EBDFRBodySide FBDFRDazBodyProfileAdapter::ParseBodySide(
    const FString& Value)
{
    if (Value.Equals(TEXT("Left"), ESearchCase::IgnoreCase))
        return EBDFRBodySide::Left;
    if (Value.Equals(TEXT("Right"), ESearchCase::IgnoreCase))
        return EBDFRBodySide::Right;
    return EBDFRBodySide::Center;
}

bool FBDFRDazBodyProfileAdapter::ParseBodyProfileJson(
    const FString& JsonText,
    FBDFRBodyProfile& OutProfile,
    TArray<FString>* OutWarnings)
{
    OutProfile = FBDFRBodyProfile();

    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader =
        TJsonReaderFactory<>::Create(JsonText);

    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        return false;
    }

    const FString Schema = GetString(Root, TEXT("schema"));
    if (!Schema.IsEmpty() &&
        !Schema.Equals(TEXT("BDFR.BodyProfile"), ESearchCase::CaseSensitive) &&
        OutWarnings)
    {
        OutWarnings->Add(FString::Printf(
            TEXT("Unexpected body profile schema: %s"),
            *Schema));
    }

    double Version = 1.0;
    Root->TryGetNumberField(TEXT("version"), Version);
    OutProfile.Version = FMath::Max(1, static_cast<int32>(Version));
    OutProfile.Units = GetString(Root, TEXT("units"), TEXT("cm"));
    OutProfile.SourceCoordinateSystem =
        GetString(Root, TEXT("coordinateSystem"), TEXT("DazStudio"));
    OutProfile.CharacterName =
        GetString(Root, TEXT("characterName"));
    OutProfile.CharacterLabel =
        GetString(Root, TEXT("characterLabel"));

    const TArray<TSharedPtr<FJsonValue>>* Regions = nullptr;
    if (!Root->TryGetArrayField(TEXT("regions"), Regions) || !Regions)
    {
        return true;
    }

    for (const TSharedPtr<FJsonValue>& RegionValue : *Regions)
    {
        const TSharedPtr<FJsonObject> RegionObject =
            RegionValue->AsObject();

        if (!RegionObject.IsValid())
        {
            continue;
        }

        FBDFRBodyRegion Region;
        Region.Id = FName(*GetString(RegionObject, TEXT("id")));
        Region.Label = GetString(RegionObject, TEXT("label"));
        Region.Name = FName(*GetString(RegionObject, TEXT("name")));
        Region.Type = ParseRegionType(
            GetString(RegionObject, TEXT("type"), TEXT("Custom")));
        Region.Side = ParseBodySide(
            GetString(RegionObject, TEXT("side"), TEXT("Center")));
        Region.AnchorA =
            FName(*GetString(RegionObject, TEXT("anchorA")));
        Region.AnchorB =
            FName(*GetString(RegionObject, TEXT("anchorB")));

        Region.SourceLocalPosition =
            ReadVec3(RegionObject, TEXT("localPosition"));
        Region.SourceLocalRotation =
            ReadQuat(RegionObject, TEXT("localRotation"));
        Region.SourceWorldPosition =
            ReadVec3(RegionObject, TEXT("worldPosition"));
        Region.SourceWorldRotation =
            ReadQuat(RegionObject, TEXT("worldRotation"));
        Region.RadiusCm =
            ReadVec3(RegionObject, TEXT("radiusCm"));

        Region.MassKg =
            GetFloat(RegionObject, TEXT("massKg"), 0.25f);
        Region.Stiffness =
            GetFloat(RegionObject, TEXT("stiffness"), 0.65f);
        Region.Damping =
            GetFloat(RegionObject, TEXT("damping"), 0.35f);
        Region.Compliance =
            GetFloat(RegionObject, TEXT("compliance"), 0.25f);
        Region.ActivationScale =
            GetFloat(RegionObject, TEXT("activationScale"), 1.0f);
        Region.MaxBulge =
            GetFloat(RegionObject, TEXT("maxBulge"), 0.12f);
        Region.CollisionScale =
            GetFloat(RegionObject, TEXT("collisionScale"), 1.0f);

        bool bEnabled = true;
        RegionObject->TryGetBoolField(TEXT("enabled"), bEnabled);
        Region.bEnabled = bEnabled;

        if (Region.AnchorA == NAME_None && OutWarnings)
        {
            OutWarnings->Add(FString::Printf(
                TEXT("Body region '%s' has no Anchor A."),
                *Region.Name.ToString()));
        }

        OutProfile.Regions.Add(Region);
    }

    return true;
}
