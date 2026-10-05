#include "BDFRBodyProfile.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegExp>
#include <dzjsonwriter.h>

BDFRBodyRegion::BDFRBodyRegion()
    : depthRadiusCm(4.0),
      massKg(0.25),
      stiffness(0.65),
      damping(0.35),
      compliance(0.25),
      activationScale(1.0),
      maxBulge(0.12),
      collisionScale(1.0),
      enabled(true)
{
    radiusCm = QSizeF(5.0, 5.0);
}

BDFRAnimationClip::BDFRAnimationClip()
    : loop(true)
{
}

BDFRBodyProfile::BDFRBodyProfile()
{
    clear();
}

void BDFRBodyProfile::clear()
{
    characterName.clear();
    gender = "Auto";
    genesisProfile = "Unknown";
    regions.clear();
    animationClips.clear();
}

bool BDFRBodyProfile::writeJson(const QString& filePath, QString* errorMessage) const
{
    QFileInfo info(filePath);
    QDir().mkpath(info.absolutePath());

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        if (errorMessage)
            *errorMessage = QString("Unable to open %1 for writing.").arg(filePath);
        return false;
    }

    DzJsonWriter writer(&file);
    writer.startObject(true);
    writer.addMember("schema", QString("BDFR.BodyProfile"));
    writer.addMember("version", 3);
    writer.addMember("units", QString("cm"));
    writer.addMember("coordinateSystem", QString("DazStudio"));
    writer.addMember("characterName", characterName);
    writer.addMember("gender", gender);
    writer.addMember("genesisProfile", genesisProfile);

    writer.startMemberArray("animationClips", true);
    for (int i = 0; i < animationClips.count(); ++i)
    {
        const BDFRAnimationClip& clip = animationClips.at(i);
        writer.startObject(true);
        writer.addMember("name", clip.name);
        writer.addMember("type", clip.type);
        writer.addMember("sourceFile", clip.sourceFile);
        writer.addMember("loop", clip.loop);
        writer.finishObject();
    }
    writer.finishArray();

    writer.startMemberArray("regions", true);
    for (int i = 0; i < regions.count(); ++i)
    {
        const BDFRBodyRegion& region = regions.at(i);
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
    file.close();
    return true;
}

namespace
{
void resolveAnchors(const QString& preset,
                    const QString& side,
                    const QString& genesis,
                    QString& anchorA,
                    QString& anchorB)
{
    const bool g9 = genesis.contains("9", Qt::CaseInsensitive);
    const bool left = side.compare("Left", Qt::CaseInsensitive) == 0;
    const QString p = left ? "l" : "r";

    if (preset == "Pectoral" || preset == "Breast")
    {
        anchorA = g9 ? p + "_shoulder" : p + "Collar";
        anchorB = g9 ? p + "_upperarm" : p + "ShldrBend";
    }
    else if (preset == "Biceps" || preset == "Triceps" || preset == "UpperArmSoftTissue")
    {
        anchorA = g9 ? p + "_upperarm" : p + "ShldrBend";
        anchorB = g9 ? p + "_forearm" : p + "ForearmBend";
    }
    else if (preset == "Glute" || preset == "Thigh" || preset == "Quadriceps" || preset == "Hamstring")
    {
        anchorA = g9 ? p + "_thigh" : p + "ThighBend";
        anchorB = g9 ? p + "_shin" : p + "Shin";
    }
    else if (preset == "Calf")
    {
        anchorA = g9 ? p + "_shin" : p + "Shin";
        anchorB = g9 ? p + "_foot" : p + "Foot";
    }
    else
    {
        anchorA = g9 ? "pelvis" : "hip";
        anchorB.clear();
    }
}

QString safeId(QString value)
{
    value.replace(QRegExp("[^A-Za-z0-9_]+"), "_");
    return value;
}
}

QStringList BDFRBodyPresets::supportedPresetIds()
{
    return QStringList()
        << "Pectoral" << "Breast" << "Biceps" << "Triceps"
        << "Abdomen" << "Glute" << "Thigh" << "Quadriceps"
        << "Hamstring" << "Calf" << "UpperArmSoftTissue" << "Custom";
}

BDFRBodyRegion BDFRBodyPresets::makeRegion(
    const QString& presetId,
    const QString& side,
    const QString& genesisProfile,
    const QPointF& normalizedPoint)
{
    BDFRBodyRegion region;
    region.type = "Muscle";
    region.side = side;
    region.name = presetId + ((side == "Left") ? "_L" : ((side == "Right") ? "_R" : ""));
    region.id = safeId(QString("BDFR_%1_%2_%3_%4")
                       .arg(presetId)
                       .arg(side)
                       .arg((int)(normalizedPoint.x() * 1000.0))
                       .arg((int)(normalizedPoint.y() * 1000.0)));
    region.normalizedCenter = normalizedPoint;

    if (presetId == "Breast")
    {
        region.type = "Breast";
        region.radiusCm = QSizeF(6.0, 5.0);
        region.depthRadiusCm = 4.0;
        region.massKg = 0.40;
        region.stiffness = 0.80;
        region.damping = 0.30;
    }
    else if (presetId == "Glute")
    {
        region.type = "Glute";
        region.radiusCm = QSizeF(7.0, 6.0);
        region.depthRadiusCm = 7.0;
        region.massKg = 0.65;
        region.stiffness = 0.60;
        region.damping = 0.40;
    }
    else if (presetId == "Thigh" || presetId == "UpperArmSoftTissue" || presetId == "Abdomen")
    {
        region.type = "SoftTissue";
        region.stiffness = 0.35;
        region.damping = 0.55;
        region.compliance = 0.50;
        if (presetId == "Thigh")
        {
            region.radiusCm = QSizeF(6.5, 13.0);
            region.depthRadiusCm = 6.0;
        }
        else if (presetId == "Abdomen")
        {
            region.side = "Center";
            region.name = "Abdomen";
            region.radiusCm = QSizeF(8.0, 10.0);
            region.depthRadiusCm = 5.5;
        }
        else
        {
            region.radiusCm = QSizeF(4.0, 9.0);
            region.depthRadiusCm = 4.0;
        }
    }
    else if (presetId == "Calf")
    {
        region.radiusCm = QSizeF(4.5, 10.0);
        region.depthRadiusCm = 4.5;
    }
    else if (presetId == "Quadriceps" || presetId == "Hamstring")
    {
        region.radiusCm = QSizeF(5.5, 13.0);
        region.depthRadiusCm = 5.5;
    }
    else if (presetId == "Biceps" || presetId == "Triceps")
    {
        region.radiusCm = QSizeF(3.8, 8.5);
        region.depthRadiusCm = 3.8;
    }
    else if (presetId == "Pectoral")
    {
        region.radiusCm = QSizeF(6.5, 5.0);
        region.depthRadiusCm = 3.8;
    }

    resolveAnchors(presetId, region.side, genesisProfile, region.anchorA, region.anchorB);
    return region;
}
