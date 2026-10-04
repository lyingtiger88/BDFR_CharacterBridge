#pragma once

#include <QtGui/QGraphicsView>
#include <QtCore/QMap>
#include <QtCore/QPointF>
#include <QtCore/QList>
#include <QtCore/QSizeF>

class QGraphicsScene;
class QGraphicsPixmapItem;
class QGraphicsEllipseItem;
class QGraphicsTextItem;
class QGraphicsLineItem;
class QResizeEvent;
class QMouseEvent;

class BDFRBodyViewer : public QGraphicsView
{
    Q_OBJECT

public:
    enum Gender { Female, Male };
    explicit BDFRBodyViewer(Gender gender, QWidget* parent = 0);

    void setActive(bool active);
    bool isActive() const { return m_active; }
    Gender gender() const { return m_gender; }

    void setShowBones(bool show);
    void setShowJointNames(bool show);
    void setShowMuscleRegions(bool show);
    void setShowSoftTissueRegions(bool show);
    void setXRayOpacity(double value);

    QPointF lastNormalizedClick() const { return m_lastNormalizedClick; }
    QString lastPresetId() const { return m_lastPresetId; }
    QString lastSide() const { return m_lastSide; }

signals:
    void anatomyPresetClicked(const QString& presetId, const QString& side, const QPointF& normalizedPoint);
    void viewerActivated(int gender);

protected:
    void resizeEvent(QResizeEvent* event);
    void mousePressEvent(QMouseEvent* event);

private:
    struct Hotspot
    {
        QString presetId;
        QString side;
        QPointF normalizedCenter;
        QSizeF normalizedSize;
        bool softTissue;
    };

    void rebuildScene();
    void buildSkeleton();
    void buildHotspots();
    void fitBody();
    QPointF sceneToNormalized(const QPointF& point) const;
    QPointF normalizedToScene(const QPointF& point) const;
    QGraphicsEllipseItem* makeJoint(const QString& name, const QPointF& p);
    void makeBone(const QString& a, const QString& b);
    void updateOverlayVisibility();

    Gender m_gender;
    bool m_active;
    bool m_showBones;
    bool m_showJointNames;
    bool m_showMuscleRegions;
    bool m_showSoftTissueRegions;
    double m_xrayOpacity;

    QGraphicsScene* m_scene;
    QGraphicsPixmapItem* m_bodyItem;
    QGraphicsPixmapItem* m_glowItem;
    QMap<QString, QPointF> m_joints;
    QList<QGraphicsLineItem*> m_boneItems;
    QList<QGraphicsEllipseItem*> m_jointItems;
    QList<QGraphicsTextItem*> m_jointLabelItems;
    QList<QGraphicsEllipseItem*> m_hotspotItems;
    QList<Hotspot> m_hotspots;
    QPointF m_lastNormalizedClick;
    QString m_lastPresetId;
    QString m_lastSide;
};
