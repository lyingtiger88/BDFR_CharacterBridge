#include "Adapters/Daz/BDFRDazSkeletonAdapter.h"

#include "Containers/Set.h"

namespace
{
    bool HasAll(const TSet<FName>& Bones, std::initializer_list<const TCHAR*> Required)
    {
        for (const TCHAR* Bone : Required)
        {
            if (!Bones.Contains(FName(Bone)))
            {
                return false;
            }
        }
        return true;
    }

    bool HintContains(const FString& Hint, const TCHAR* Value)
    {
        return Hint.Contains(Value, ESearchCase::IgnoreCase);
    }

    void AddMapping(
        FBDFRSkeletonProfile& Profile,
        const TCHAR* Source,
        const TCHAR* Canonical,
        const bool bRequired = false,
        const EBDFRBoneRetargetPolicy Policy = EBDFRBoneRetargetPolicy::Full,
        const float RotationWeight = 1.0f)
    {
        FBDFRBoneMapping Entry;
        Entry.SourceBone = FName(Source);
        Entry.CanonicalBone = FName(Canonical);
        Entry.bRequired = bRequired;
        Entry.RetargetPolicy = Policy;
        Entry.RotationWeight = RotationWeight;
        Profile.BoneMappings.Add(Entry);
    }

    void AddChain(
        FBDFRSkeletonProfile& Profile,
        const TCHAR* ChainName,
        const TCHAR* SourceStart,
        const TCHAR* SourceEnd,
        const TCHAR* CanonicalStart,
        const TCHAR* CanonicalEnd)
    {
        FBDFRRetargetChainDefinition Chain;
        Chain.ChainName = FName(ChainName);
        Chain.SourceStartBone = FName(SourceStart);
        Chain.SourceEndBone = FName(SourceEnd);
        Chain.CanonicalStartBone = FName(CanonicalStart);
        Chain.CanonicalEndBone = FName(CanonicalEnd);
        Profile.RetargetChains.Add(Chain);
    }

    bool MappingExists(const FBDFRSkeletonProfile& Profile, const FName SourceBone)
    {
        return Profile.BoneMappings.ContainsByPredicate(
            [SourceBone](const FBDFRBoneMapping& Mapping)
            {
                return Mapping.SourceBone == SourceBone;
            });
    }

    void AddFingerMappingsGenesis8(FBDFRSkeletonProfile& Profile, const TCHAR SidePrefix, const TCHAR* CanonicalSide)
    {
        const FString Prefix(1, &SidePrefix);

        const TArray<TPair<FString, FString>> Fingers =
        {
            { TEXT("Thumb"), TEXT("thumb") },
            { TEXT("Index"), TEXT("index") },
            { TEXT("Mid"), TEXT("middle") },
            { TEXT("Ring"), TEXT("ring") },
            { TEXT("Pinky"), TEXT("pinky") }
        };

        for (const TPair<FString, FString>& Finger : Fingers)
        {
            for (int32 Index = 1; Index <= 3; ++Index)
            {
                const FString Source = FString::Printf(TEXT("%s%s%d"), *Prefix, *Finger.Key, Index);
                const FString Canonical = FString::Printf(
                    TEXT("%s_%02d_%s"),
                    *Finger.Value,
                    Index,
                    CanonicalSide);

                AddMapping(Profile, *Source, *Canonical);
            }
        }
    }

    void AddFingerMappingsGenesis9(FBDFRSkeletonProfile& Profile, const TCHAR SidePrefix, const TCHAR* CanonicalSide)
    {
        const FString Prefix(1, &SidePrefix);

        const TArray<TPair<FString, FString>> Fingers =
        {
            { TEXT("thumb"), TEXT("thumb") },
            { TEXT("index"), TEXT("index") },
            { TEXT("mid"), TEXT("middle") },
            { TEXT("ring"), TEXT("ring") },
            { TEXT("pinky"), TEXT("pinky") }
        };

        for (const TPair<FString, FString>& Finger : Fingers)
        {
            for (int32 Index = 1; Index <= 3; ++Index)
            {
                const FString Source = FString::Printf(TEXT("%s_%s%d"), *Prefix, *Finger.Key, Index);
                const FString Canonical = FString::Printf(
                    TEXT("%s_%02d_%s"),
                    *Finger.Value,
                    Index,
                    CanonicalSide);

                AddMapping(Profile, *Source, *Canonical);
            }
        }
    }

    void BuildGenesis8Body(FBDFRSkeletonProfile& Profile)
    {
        Profile.SourceRetargetRoot = TEXT("hip");
        Profile.CanonicalRetargetRoot = TEXT("pelvis");

        AddMapping(Profile, TEXT("hip"), TEXT("pelvis"), true);
        AddMapping(Profile, TEXT("pelvis"), TEXT("pelvis_aux"), false, EBDFRBoneRetargetPolicy::DeformationOnly);

        AddMapping(Profile, TEXT("abdomenLower"), TEXT("spine_01"), true);
        AddMapping(Profile, TEXT("abdomenUpper"), TEXT("spine_02"), true);
        AddMapping(Profile, TEXT("chestLower"), TEXT("spine_03"));
        AddMapping(Profile, TEXT("chestUpper"), TEXT("spine_04"), true);
        AddMapping(Profile, TEXT("neckLower"), TEXT("neck_01"), true);
        AddMapping(Profile, TEXT("neckUpper"), TEXT("neck_02"));
        AddMapping(Profile, TEXT("head"), TEXT("head"), true);

        AddMapping(Profile, TEXT("lCollar"), TEXT("clavicle_l"), true);
        AddMapping(Profile, TEXT("lShldrBend"), TEXT("upperarm_l"), true);
        AddMapping(Profile, TEXT("lShldrTwist"), TEXT("upperarm_twist_01_l"), false, EBDFRBoneRetargetPolicy::DeformationOnly);
        AddMapping(Profile, TEXT("lForearmBend"), TEXT("lowerarm_l"), true);
        AddMapping(Profile, TEXT("lForearmTwist"), TEXT("lowerarm_twist_01_l"), false, EBDFRBoneRetargetPolicy::DeformationOnly);
        AddMapping(Profile, TEXT("lHand"), TEXT("hand_l"), true);

        AddMapping(Profile, TEXT("rCollar"), TEXT("clavicle_r"), true);
        AddMapping(Profile, TEXT("rShldrBend"), TEXT("upperarm_r"), true);
        AddMapping(Profile, TEXT("rShldrTwist"), TEXT("upperarm_twist_01_r"), false, EBDFRBoneRetargetPolicy::DeformationOnly);
        AddMapping(Profile, TEXT("rForearmBend"), TEXT("lowerarm_r"), true);
        AddMapping(Profile, TEXT("rForearmTwist"), TEXT("lowerarm_twist_01_r"), false, EBDFRBoneRetargetPolicy::DeformationOnly);
        AddMapping(Profile, TEXT("rHand"), TEXT("hand_r"), true);

        AddMapping(Profile, TEXT("lThighBend"), TEXT("thigh_l"), true);
        AddMapping(Profile, TEXT("lThighTwist"), TEXT("thigh_twist_01_l"), false, EBDFRBoneRetargetPolicy::DeformationOnly);
        AddMapping(Profile, TEXT("lShin"), TEXT("calf_l"), true);
        AddMapping(Profile, TEXT("lFoot"), TEXT("foot_l"), true);
        AddMapping(Profile, TEXT("lToe"), TEXT("ball_l"), true);

        AddMapping(Profile, TEXT("rThighBend"), TEXT("thigh_r"), true);
        AddMapping(Profile, TEXT("rThighTwist"), TEXT("thigh_twist_01_r"), false, EBDFRBoneRetargetPolicy::DeformationOnly);
        AddMapping(Profile, TEXT("rShin"), TEXT("calf_r"), true);
        AddMapping(Profile, TEXT("rFoot"), TEXT("foot_r"), true);
        AddMapping(Profile, TEXT("rToe"), TEXT("ball_r"), true);

        AddFingerMappingsGenesis8(Profile, TEXT('l'), TEXT("l"));
        AddFingerMappingsGenesis8(Profile, TEXT('r'), TEXT("r"));

        AddChain(Profile, TEXT("Spine"), TEXT("abdomenLower"), TEXT("chestUpper"), TEXT("spine_01"), TEXT("spine_04"));
        AddChain(Profile, TEXT("Head"), TEXT("neckLower"), TEXT("head"), TEXT("neck_01"), TEXT("head"));
        AddChain(Profile, TEXT("LeftClavicle"), TEXT("lCollar"), TEXT("lCollar"), TEXT("clavicle_l"), TEXT("clavicle_l"));
        AddChain(Profile, TEXT("RightClavicle"), TEXT("rCollar"), TEXT("rCollar"), TEXT("clavicle_r"), TEXT("clavicle_r"));
        AddChain(Profile, TEXT("LeftArm"), TEXT("lShldrBend"), TEXT("lHand"), TEXT("upperarm_l"), TEXT("hand_l"));
        AddChain(Profile, TEXT("RightArm"), TEXT("rShldrBend"), TEXT("rHand"), TEXT("upperarm_r"), TEXT("hand_r"));
        AddChain(Profile, TEXT("LeftLeg"), TEXT("lThighBend"), TEXT("lToe"), TEXT("thigh_l"), TEXT("ball_l"));
        AddChain(Profile, TEXT("RightLeg"), TEXT("rThighBend"), TEXT("rToe"), TEXT("thigh_r"), TEXT("ball_r"));
        AddChain(Profile, TEXT("LeftThumb"), TEXT("lThumb1"), TEXT("lThumb3"), TEXT("thumb_01_l"), TEXT("thumb_03_l"));
        AddChain(Profile, TEXT("RightThumb"), TEXT("rThumb1"), TEXT("rThumb3"), TEXT("thumb_01_r"), TEXT("thumb_03_r"));
        AddChain(Profile, TEXT("LeftIndex"), TEXT("lIndex1"), TEXT("lIndex3"), TEXT("index_01_l"), TEXT("index_03_l"));
        AddChain(Profile, TEXT("RightIndex"), TEXT("rIndex1"), TEXT("rIndex3"), TEXT("index_01_r"), TEXT("index_03_r"));
        AddChain(Profile, TEXT("LeftMiddle"), TEXT("lMid1"), TEXT("lMid3"), TEXT("middle_01_l"), TEXT("middle_03_l"));
        AddChain(Profile, TEXT("RightMiddle"), TEXT("rMid1"), TEXT("rMid3"), TEXT("middle_01_r"), TEXT("middle_03_r"));
        AddChain(Profile, TEXT("LeftRing"), TEXT("lRing1"), TEXT("lRing3"), TEXT("ring_01_l"), TEXT("ring_03_l"));
        AddChain(Profile, TEXT("RightRing"), TEXT("rRing1"), TEXT("rRing3"), TEXT("ring_01_r"), TEXT("ring_03_r"));
        AddChain(Profile, TEXT("LeftPinky"), TEXT("lPinky1"), TEXT("lPinky3"), TEXT("pinky_01_l"), TEXT("pinky_03_l"));
        AddChain(Profile, TEXT("RightPinky"), TEXT("rPinky1"), TEXT("rPinky3"), TEXT("pinky_01_r"), TEXT("pinky_03_r"));
    }

    void BuildGenesis9Body(FBDFRSkeletonProfile& Profile)
    {
        Profile.SourceRetargetRoot = TEXT("hip");
        Profile.CanonicalRetargetRoot = TEXT("pelvis");

        AddMapping(Profile, TEXT("hip"), TEXT("pelvis"), true);
        AddMapping(Profile, TEXT("pelvis"), TEXT("pelvis_aux"), true, EBDFRBoneRetargetPolicy::DeformationOnly);

        AddMapping(Profile, TEXT("spine1"), TEXT("spine_01"), true);
        AddMapping(Profile, TEXT("spine2"), TEXT("spine_02"));
        AddMapping(Profile, TEXT("spine3"), TEXT("spine_03"));
        AddMapping(Profile, TEXT("spine4"), TEXT("spine_04"), true);
        AddMapping(Profile, TEXT("neck1"), TEXT("neck_01"), true);
        AddMapping(Profile, TEXT("neck2"), TEXT("neck_02"));
        AddMapping(Profile, TEXT("head"), TEXT("head"), true);

        AddMapping(Profile, TEXT("l_shoulder"), TEXT("clavicle_l"), true);
        AddMapping(Profile, TEXT("l_upperarm"), TEXT("upperarm_l"), true);
        AddMapping(Profile, TEXT("l_forearm"), TEXT("lowerarm_l"), true);
        AddMapping(Profile, TEXT("l_hand"), TEXT("hand_l"), true);

        AddMapping(Profile, TEXT("r_shoulder"), TEXT("clavicle_r"), true);
        AddMapping(Profile, TEXT("r_upperarm"), TEXT("upperarm_r"), true);
        AddMapping(Profile, TEXT("r_forearm"), TEXT("lowerarm_r"), true);
        AddMapping(Profile, TEXT("r_hand"), TEXT("hand_r"), true);

        AddMapping(Profile, TEXT("l_thigh"), TEXT("thigh_l"), true);
        AddMapping(Profile, TEXT("l_shin"), TEXT("calf_l"), true);
        AddMapping(Profile, TEXT("l_foot"), TEXT("foot_l"), true);
        AddMapping(Profile, TEXT("l_toes"), TEXT("ball_l"), true);

        AddMapping(Profile, TEXT("r_thigh"), TEXT("thigh_r"), true);
        AddMapping(Profile, TEXT("r_shin"), TEXT("calf_r"), true);
        AddMapping(Profile, TEXT("r_foot"), TEXT("foot_r"), true);
        AddMapping(Profile, TEXT("r_toes"), TEXT("ball_r"), true);

        AddFingerMappingsGenesis9(Profile, TEXT('l'), TEXT("l"));
        AddFingerMappingsGenesis9(Profile, TEXT('r'), TEXT("r"));

        AddChain(Profile, TEXT("Spine"), TEXT("spine1"), TEXT("spine4"), TEXT("spine_01"), TEXT("spine_04"));
        AddChain(Profile, TEXT("Head"), TEXT("neck1"), TEXT("head"), TEXT("neck_01"), TEXT("head"));
        AddChain(Profile, TEXT("LeftClavicle"), TEXT("l_shoulder"), TEXT("l_shoulder"), TEXT("clavicle_l"), TEXT("clavicle_l"));
        AddChain(Profile, TEXT("RightClavicle"), TEXT("r_shoulder"), TEXT("r_shoulder"), TEXT("clavicle_r"), TEXT("clavicle_r"));
        AddChain(Profile, TEXT("LeftArm"), TEXT("l_upperarm"), TEXT("l_hand"), TEXT("upperarm_l"), TEXT("hand_l"));
        AddChain(Profile, TEXT("RightArm"), TEXT("r_upperarm"), TEXT("r_hand"), TEXT("upperarm_r"), TEXT("hand_r"));
        AddChain(Profile, TEXT("LeftLeg"), TEXT("l_thigh"), TEXT("l_toes"), TEXT("thigh_l"), TEXT("ball_l"));
        AddChain(Profile, TEXT("RightLeg"), TEXT("r_thigh"), TEXT("r_toes"), TEXT("thigh_r"), TEXT("ball_r"));
        AddChain(Profile, TEXT("LeftThumb"), TEXT("l_thumb1"), TEXT("l_thumb3"), TEXT("thumb_01_l"), TEXT("thumb_03_l"));
        AddChain(Profile, TEXT("RightThumb"), TEXT("r_thumb1"), TEXT("r_thumb3"), TEXT("thumb_01_r"), TEXT("thumb_03_r"));
        AddChain(Profile, TEXT("LeftIndex"), TEXT("l_index1"), TEXT("l_index3"), TEXT("index_01_l"), TEXT("index_03_l"));
        AddChain(Profile, TEXT("RightIndex"), TEXT("r_index1"), TEXT("r_index3"), TEXT("index_01_r"), TEXT("index_03_r"));
        AddChain(Profile, TEXT("LeftMiddle"), TEXT("l_mid1"), TEXT("l_mid3"), TEXT("middle_01_l"), TEXT("middle_03_l"));
        AddChain(Profile, TEXT("RightMiddle"), TEXT("r_mid1"), TEXT("r_mid3"), TEXT("middle_01_r"), TEXT("middle_03_r"));
        AddChain(Profile, TEXT("LeftRing"), TEXT("l_ring1"), TEXT("l_ring3"), TEXT("ring_01_l"), TEXT("ring_03_l"));
        AddChain(Profile, TEXT("RightRing"), TEXT("r_ring1"), TEXT("r_ring3"), TEXT("ring_01_r"), TEXT("ring_03_r"));
        AddChain(Profile, TEXT("LeftPinky"), TEXT("l_pinky1"), TEXT("l_pinky3"), TEXT("pinky_01_l"), TEXT("pinky_03_l"));
        AddChain(Profile, TEXT("RightPinky"), TEXT("r_pinky1"), TEXT("r_pinky3"), TEXT("pinky_01_r"), TEXT("pinky_03_r"));
    }
}

EBDFRCharacterSourceProfile FBDFRDazSkeletonAdapter::DetectSourceProfile(
    const TArray<FName>& SourceBones,
    const FString& SourceAssetId)
{
    TSet<FName> BoneSet;
    for (const FName Bone : SourceBones)
    {
        BoneSet.Add(Bone);
    }

    if (HintContains(SourceAssetId, TEXT("Genesis3")))
    {
        return EBDFRCharacterSourceProfile::Unknown;
    }

    const bool bGenesis9 = HasAll(
        BoneSet,
        { TEXT("hip"), TEXT("pelvis"), TEXT("spine1"), TEXT("spine4"), TEXT("l_upperarm"), TEXT("r_upperarm"), TEXT("l_thigh"), TEXT("r_thigh") });

    if (bGenesis9)
    {
        return EBDFRCharacterSourceProfile::DazGenesis9;
    }

    const bool bLooksLikeGenesis3 = HasAll(
        BoneSet,
        { TEXT("hip"), TEXT("abdomenLower"), TEXT("abdomenUpper"), TEXT("chestUpper"), TEXT("lHeel") });

    if (bLooksLikeGenesis3)
    {
        return EBDFRCharacterSourceProfile::Unknown;
    }

    const bool bGenesis8Family = HasAll(
        BoneSet,
        { TEXT("hip"), TEXT("abdomenLower"), TEXT("abdomenUpper"), TEXT("chestUpper"), TEXT("lShldrBend"), TEXT("rShldrBend"), TEXT("lThighBend"), TEXT("rThighBend") });

    if (bGenesis8Family)
    {
        if (HintContains(SourceAssetId, TEXT("Genesis8_1")) ||
            HintContains(SourceAssetId, TEXT("Genesis81")) ||
            HintContains(SourceAssetId, TEXT("Genesis 8.1")) ||
            HintContains(SourceAssetId, TEXT("Genesis8.1")))
        {
            return EBDFRCharacterSourceProfile::DazGenesis81;
        }

        return EBDFRCharacterSourceProfile::DazGenesis8;
    }

    return EBDFRCharacterSourceProfile::Unknown;
}

FBDFRSkeletonProfile FBDFRDazSkeletonAdapter::BuildProfile(
    const EBDFRCharacterSourceProfile SourceProfile,
    const TArray<FName>& SourceBones)
{
    FBDFRSkeletonProfile Profile;
    Profile.SourceProfile = SourceProfile;
    Profile.bPreserveUnmappedBones = true;
    Profile.bPreserveTwistBones = true;

    switch (SourceProfile)
    {
    case EBDFRCharacterSourceProfile::DazGenesis8:
        Profile.ProfileName = TEXT("Daz_Genesis8");
        BuildGenesis8Body(Profile);
        break;

    case EBDFRCharacterSourceProfile::DazGenesis81:
        Profile.ProfileName = TEXT("Daz_Genesis8_1");
        BuildGenesis8Body(Profile);
        break;

    case EBDFRCharacterSourceProfile::DazGenesis9:
        Profile.ProfileName = TEXT("Daz_Genesis9");
        BuildGenesis9Body(Profile);
        break;

    default:
        Profile.ProfileName = TEXT("Unknown");
        break;
    }

    AddDetectedTwistBones(Profile, SourceBones);
    return Profile;
}

FBDFRSkeletonValidationReport FBDFRDazSkeletonAdapter::Validate(
    const FBDFRSkeletonProfile& Profile,
    const TArray<FName>& SourceBones)
{
    FBDFRSkeletonValidationReport Report;

    TSet<FName> BoneSet;
    for (const FName Bone : SourceBones)
    {
        BoneSet.Add(Bone);

        if (IsTwistBone(Bone))
        {
            Report.TwistBones.AddUnique(Bone);
        }
    }

    Report.bRetargetRootFound =
        Profile.SourceRetargetRoot != NAME_None &&
        BoneSet.Contains(Profile.SourceRetargetRoot);

    for (const FBDFRBoneMapping& Mapping : Profile.BoneMappings)
    {
        if (Mapping.bRequired && !BoneSet.Contains(Mapping.SourceBone))
        {
            Report.MissingRequiredBones.AddUnique(Mapping.SourceBone);
        }
    }

    for (const FName Bone : SourceBones)
    {
        if (!MappingExists(Profile, Bone))
        {
            Report.UnmappedBones.AddUnique(Bone);
        }
    }

    return Report;
}

bool FBDFRDazSkeletonAdapter::IsTwistBone(const FName BoneName)
{
    return BoneName.ToString().Contains(TEXT("twist"), ESearchCase::IgnoreCase);
}

void FBDFRDazSkeletonAdapter::AddDetectedTwistBones(
    FBDFRSkeletonProfile& Profile,
    const TArray<FName>& SourceBones)
{
    if (!Profile.bPreserveTwistBones)
    {
        return;
    }

    for (const FName Bone : SourceBones)
    {
        if (!IsTwistBone(Bone) || MappingExists(Profile, Bone))
        {
            continue;
        }

        FBDFRBoneMapping Entry;
        Entry.SourceBone = Bone;
        Entry.CanonicalBone = Bone;
        Entry.bRequired = false;
        Entry.RetargetPolicy = EBDFRBoneRetargetPolicy::DeformationOnly;
        Entry.RotationWeight = 1.0f;
        Profile.BoneMappings.Add(Entry);
    }
}
