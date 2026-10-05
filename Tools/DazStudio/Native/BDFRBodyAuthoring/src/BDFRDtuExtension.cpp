#include "BDFRDtuExtension.h"

#include <dzjsonwriter.h>

namespace BDFRDtuExtension
{
void writeBodyProfile(DzJsonWriter& writer, const BDFRBodyProfile& profile)
{
    writer.startMemberObject("BDFRBodyProfile");
    writer.addMember("version", 3);
    writer.addMember("units", QString("cm"));
    writer.addMember("coordinateSystem", QString("DazStudio"));
    writer.addMember("characterName", profile.characterName);
    writer.addMember("gender", profile.gender);
    writer.addMember("genesisProfile", profile.genesisProfile);

    writer.startMemberArray("animationClips", true);
    for (int i = 0; i < profile.animationClips.count(); ++i)
    {
        const BDFRAnimationClip& clip = profile.animationClips.at(i);
        writer.startObject(true);
        writer.addMember("name", clip.name);
        writer.addMember("type", clip.type);
        writer.addMember("sourceFile", clip.sourceFile);
        writer.addMember("loop", clip.loop);
        writer.finishObject();
    }
    writer.finishArray();

    writer.startMemberArray("regions", true);
    for (int i = 0; i < profile.regions.count(); ++i)
    {
        const BDFRBodyRegion& region = profile.regions.at(i);
        writer.startObject(true);
        writer.addMember("id", region.id);
        writer.addMember("name", region.name);
        writer.addMember("type", region.type);
        writer.addMember("side", region.side);
        writer.addMember("anchorA", region.anchorA);
        writer.addMember("anchorB", region.anchorB);
        writer.addMember("normalizedX", region.normalizedCenter.x());
        writer.addMember("normalizedY", region.normalizedCenter.y());
        writer.addMember("radiusXcm", region.radiusCm.width());
        writer.addMember("radiusYcm", region.radiusCm.height());
        writer.addMember("radiusZcm", region.depthRadiusCm);
        writer.addMember("massKg", region.massKg);
        writer.addMember("stiffness", region.stiffness);
        writer.addMember("damping", region.damping);
        writer.addMember("compliance", region.compliance);
        writer.addMember("activationScale", region.activationScale);
        writer.addMember("maxBulge", region.maxBulge);
        writer.addMember("collisionScale", region.collisionScale);
        writer.addMember("enabled", region.enabled);
        writer.finishObject();
    }
    writer.finishArray();

    writer.finishObject();
}
}
