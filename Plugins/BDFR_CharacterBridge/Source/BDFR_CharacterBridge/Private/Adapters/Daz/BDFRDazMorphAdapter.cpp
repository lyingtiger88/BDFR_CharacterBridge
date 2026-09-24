#include "Adapters/Daz/BDFRDazMorphAdapter.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    FString Lower(const FString& Value)
    {
        return Value.ToLower();
    }

    bool ContainsAny(const FString& Value, std::initializer_list<const TCHAR*> Tokens)
    {
        const FString LowerValue = Value.ToLower();
        for (const TCHAR* Token : Tokens)
        {
            if (LowerValue.Contains(FString(Token).ToLower()))
            {
                return true;
            }
        }
        return false;
    }

    const FBDFRMorphDescriptor* FindMorphBySourceOrImportedName(
        const TArray<FBDFRMorphDescriptor>& Morphs,
        const FName MorphName)
    {
        return Morphs.FindByPredicate(
            [MorphName](const FBDFRMorphDescriptor& Morph)
            {
                return Morph.SourceInternalName == MorphName ||
                       Morph.ImportedMorphName == MorphName ||
                       Morph.TargetMorphName == MorphName;
            });
    }
}

FName FBDFRDazMorphAdapter::NormalizeDazMorphName(const FString& MorphName)
{
    FString Normalized = MorphName;

    // DazToUnreal notes that Daz Studio may strip the prefix before a period
    // while exporting the morph to FBX. Mirror that behavior in our manifest.
    int32 PeriodIndex = INDEX_NONE;
    if (Normalized.FindChar(TEXT('.'), PeriodIndex) && PeriodIndex + 1 < Normalized.Len())
    {
        Normalized = Normalized.Mid(PeriodIndex + 1);
    }

    Normalized.TrimStartAndEndInline();
    return FName(*Normalized);
}

FName FBDFRDazMorphAdapter::NormalizeFbxMorphChannelName(const FString& ChannelName)
{
    FString Normalized = ChannelName;

    // Daz FBX blend-shape channels can be namespaced as Object__MorphName.
    const int32 SeparatorIndex = Normalized.Find(TEXT("__"), ESearchCase::CaseSensitive);
    if (SeparatorIndex != INDEX_NONE && SeparatorIndex + 2 < Normalized.Len())
    {
        Normalized = Normalized.Mid(SeparatorIndex + 2);
    }

    Normalized.TrimStartAndEndInline();
    return FName(*Normalized);
}

bool FBDFRDazMorphAdapter::IsLikelyJointCorrectiveMorph(const FString& MorphName)
{
    const FString Name = Lower(MorphName);

    return Name.StartsWith(TEXT("pjcm")) ||
           Name.Contains(TEXT("jcm")) ||
           Name.EndsWith(TEXT("_dq2lb"));
}

EBDFRMorphSemantic FBDFRDazMorphAdapter::ClassifyMorph(
    const FString& MorphName,
    const FString& MorphLabel)
{
    const FString Combined = Lower(MorphName + TEXT(" ") + MorphLabel);

    if (IsLikelyJointCorrectiveMorph(MorphName))
    {
        return EBDFRMorphSemantic::Corrective;
    }

    if (ContainsAny(Combined, { TEXT("viseme"), TEXT("phoneme"), TEXT("mouth aaa"), TEXT("mouth eee"), TEXT("mouth ooo") }))
    {
        return EBDFRMorphSemantic::Viseme;
    }

    if (Combined.StartsWith(TEXT("ectrl")) ||
        ContainsAny(Combined, { TEXT("expression"), TEXT("smile"), TEXT("frown"), TEXT("blink"), TEXT("brow"), TEXT("squint"), TEXT("surprise"), TEXT("angry") }))
    {
        return EBDFRMorphSemantic::Expression;
    }

    if (Combined.StartsWith(TEXT("fbm")) ||
        Combined.StartsWith(TEXT("pbm")) ||
        ContainsAny(Combined, { TEXT("bodybuilder"), TEXT("body"), TEXT("muscular"), TEXT("weight"), TEXT("height") }))
    {
        return EBDFRMorphSemantic::Body;
    }

    if (Combined.StartsWith(TEXT("fhm")) ||
        Combined.StartsWith(TEXT("phm")) ||
        ContainsAny(Combined, { TEXT("head"), TEXT("face"), TEXT("nose"), TEXT("jaw"), TEXT("chin"), TEXT("cheek") }))
    {
        return EBDFRMorphSemantic::Head;
    }

    if (Combined.StartsWith(TEXT("ctrl")) ||
        Combined.Contains(TEXT("control")))
    {
        return EBDFRMorphSemantic::Control;
    }

    return EBDFRMorphSemantic::Unknown;
}

bool FBDFRDazMorphAdapter::BuildTransferPlanFromDtuJson(
    const FString& DtuJson,
    const TArray<FName>& ImportedMorphNames,
    FBDFRMorphTransferPlan& OutPlan,
    const bool bUseInternalMorphNames)
{
    OutPlan = FBDFRMorphTransferPlan();

    TSharedPtr<FJsonObject> RootObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DtuJson);

    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        return false;
    }

    TSet<FName> NormalizedImportedNames;
    for (const FName ImportedMorphName : ImportedMorphNames)
    {
        NormalizedImportedNames.Add(
            NormalizeFbxMorphChannelName(ImportedMorphName.ToString()));
    }

    const TArray<TSharedPtr<FJsonValue>>* MorphList = nullptr;
    if (RootObject->TryGetArrayField(TEXT("Morphs"), MorphList) && MorphList)
    {
        for (const TSharedPtr<FJsonValue>& MorphValue : *MorphList)
        {
            const TSharedPtr<FJsonObject> MorphObject = MorphValue->AsObject();
            if (!MorphObject.IsValid())
            {
                continue;
            }

            FString SourceName;
            FString Label;
            MorphObject->TryGetStringField(TEXT("Name"), SourceName);
            MorphObject->TryGetStringField(TEXT("Label"), Label);

            if (SourceName.IsEmpty())
            {
                continue;
            }

            FBDFRMorphDescriptor Descriptor;
            Descriptor.SourceInternalName = NormalizeDazMorphName(SourceName);
            Descriptor.SourceLabel = Label;
            Descriptor.ImportedMorphName = Descriptor.SourceInternalName;
            Descriptor.TargetMorphName = bUseInternalMorphNames || Label.IsEmpty()
                ? Descriptor.SourceInternalName
                : FName(*Label);
            Descriptor.Semantic = ClassifyMorph(
                Descriptor.SourceInternalName.ToString(),
                Descriptor.SourceLabel);
            Descriptor.bFoundOnImportedMesh =
                NormalizedImportedNames.Contains(Descriptor.ImportedMorphName);

            if (!Descriptor.bFoundOnImportedMesh)
            {
                OutPlan.MissingRequestedMorphs.AddUnique(
                    Descriptor.SourceInternalName);
            }

            OutPlan.Morphs.Add(Descriptor);
        }
    }

    const TArray<TSharedPtr<FJsonValue>>* JointLinks = nullptr;
    if (RootObject->TryGetArrayField(TEXT("JointLinks"), JointLinks) && JointLinks)
    {
        for (const TSharedPtr<FJsonValue>& LinkValue : *JointLinks)
        {
            const TSharedPtr<FJsonObject> LinkObject = LinkValue->AsObject();
            if (!LinkObject.IsValid())
            {
                continue;
            }

            FString BoneName;
            FString MorphName;
            FString AxisName;
            LinkObject->TryGetStringField(TEXT("Bone"), BoneName);
            LinkObject->TryGetStringField(TEXT("Morph"), MorphName);
            LinkObject->TryGetStringField(TEXT("Axis"), AxisName);

            if (BoneName.IsEmpty() || MorphName.IsEmpty())
            {
                continue;
            }

            FBDFRJointDrivenMorphLink Link;
            Link.Bone = FName(*BoneName);
            Link.Morph = NormalizeDazMorphName(MorphName);
            Link.Axis = FName(*AxisName);

            double Scalar = 0.0;
            double Alpha = 1.0;
            LinkObject->TryGetNumberField(TEXT("Scalar"), Scalar);
            LinkObject->TryGetNumberField(TEXT("Alpha"), Alpha);
            Link.Scalar = static_cast<float>(Scalar);
            Link.Alpha = static_cast<float>(Alpha);

            const TArray<TSharedPtr<FJsonValue>>* Keys = nullptr;
            if (LinkObject->TryGetArrayField(TEXT("Keys"), Keys) && Keys)
            {
                for (const TSharedPtr<FJsonValue>& KeyValue : *Keys)
                {
                    const TSharedPtr<FJsonObject> KeyObject = KeyValue->AsObject();
                    if (!KeyObject.IsValid())
                    {
                        continue;
                    }

                    double Angle = 0.0;
                    double Value = 0.0;
                    KeyObject->TryGetNumberField(TEXT("Angle"), Angle);
                    KeyObject->TryGetNumberField(TEXT("Value"), Value);

                    FBDFRMorphKey Key;
                    Key.Angle = static_cast<float>(Angle);
                    Key.Value = static_cast<float>(Value);
                    Link.Keys.Add(Key);
                }
            }

            OutPlan.JointDrivenLinks.Add(Link);

            for (FBDFRMorphDescriptor& Morph : OutPlan.Morphs)
            {
                if (Morph.SourceInternalName == Link.Morph ||
                    Morph.ImportedMorphName == Link.Morph)
                {
                    Morph.bJointDriven = true;
                    Morph.Semantic = EBDFRMorphSemantic::Corrective;
                }
            }
        }
    }

    for (const FName ImportedMorphName : NormalizedImportedNames)
    {
        if (!FindMorphBySourceOrImportedName(OutPlan.Morphs, ImportedMorphName))
        {
            OutPlan.UnlistedImportedMorphs.AddUnique(ImportedMorphName);

            FBDFRMorphDescriptor Descriptor;
            Descriptor.SourceInternalName = ImportedMorphName;
            Descriptor.ImportedMorphName = ImportedMorphName;
            Descriptor.TargetMorphName = ImportedMorphName;
            Descriptor.Semantic = ClassifyMorph(
                ImportedMorphName.ToString(),
                FString());
            Descriptor.bJointDriven =
                IsLikelyJointCorrectiveMorph(ImportedMorphName.ToString());
            Descriptor.bFoundOnImportedMesh = true;
            OutPlan.Morphs.Add(Descriptor);
        }
    }

    return true;
}
