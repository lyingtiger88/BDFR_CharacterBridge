#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QPointF>
#include <QtCore/QSizeF>

class DzJsonWriter;

struct BDFRBodyRegion
{
    QString id;
    QString name;
    QString type;
    QString side;
    QString anchorA;
    QString anchorB;
    QPointF normalizedCenter;
    QSizeF radiusCm;
    double depthRadiusCm;
    double massKg;
    double stiffness;
    double damping;
    double compliance;
    double activationScale;
    double maxBulge;
    double collisionScale;
    bool enabled;

    BDFRBodyRegion();
};

struct BDFRAnimationClip
{
    QString name;
    QString type;
    QString sourceFile;
    bool loop;

    BDFRAnimationClip();
};

class BDFRBodyProfile
{
public:
    BDFRBodyProfile();

    void clear();
    bool writeJson(const QString& filePath, QString* errorMessage = 0) const;

    QString characterName;
    QString gender;
    QString genesisProfile;
    QList<BDFRBodyRegion> regions;
    QList<BDFRAnimationClip> animationClips;
};

namespace BDFRBodyPresets
{
    BDFRBodyRegion makeRegion(
        const QString& presetId,
        const QString& side,
        const QString& genesisProfile,
        const QPointF& normalizedPoint);

    QStringList supportedPresetIds();
}
