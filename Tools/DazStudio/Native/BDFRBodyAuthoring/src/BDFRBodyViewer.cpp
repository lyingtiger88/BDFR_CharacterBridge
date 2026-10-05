#include "BDFRBodyViewer.h"

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsPixmapItem>
#include <QtGui/QGraphicsEllipseItem>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsLineItem>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtGui/QPixmap>
#include <QtGui/QColor>

namespace
{
    const int RolePresetIndex = 1001;

    QColor femaleAccent() { return QColor(255, 66, 196); }
    QColor maleAccent() { return QColor(20, 139, 255); }
    QColor jointColor() { return QColor(35, 157, 255); }
}

BDFRBodyViewer::BDFRBodyViewer(Gender gender, QWidget* parent)
    : QGraphicsView(parent),
      m_gender(gender),
      m_active(true),
      m_showBones(true),
      m_showJointNames(false),
      m_showMuscleRegions(true),
      m_showSoftTissueRegions(true),
      m_xrayOpacity(0.72),
      m_scene(new QGraphicsScene(this)),
      m_bodyItem(0),
      m_glowItem(0)
{
    setScene(m_scene);
    setFrameShape(QFrame::NoFrame);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMinimumSize(250, 520);
    setBackgroundBrush(QColor(6, 20, 32));
    rebuildScene();
}

void BDFRBodyViewer::rebuildScene()
{
    m_scene->clear();
    m_joints.clear();
    m_boneItems.clear();
    m_jointItems.clear();
    m_jointLabelItems.clear();
    m_hotspotItems.clear();
    m_hotspots.clear();

    const QString resource = (m_gender == Female)
        ? QString(":/BDFRBodyAuthoring/female_body_map.png")
        : QString(":/BDFRBodyAuthoring/male_body_map.png");

    QPixmap pixmap(resource);
    if (pixmap.isNull())
    {
        pixmap = QPixmap(260, 620);
        pixmap.fill(Qt::transparent);
    }

    m_glowItem = m_scene->addPixmap(pixmap);
    m_bodyItem = m_scene->addPixmap(pixmap);

    const QColor accent = (m_gender == Female) ? femaleAccent() : maleAccent();
    m_glowItem->setOpacity(0.14);
    m_glowItem->setZValue(-2.0);
    m_bodyItem->setOpacity(0.96);
    m_bodyItem->setZValue(0.0);

    // A translucent background halo keeps the approved pink/blue visual language
    // without modifying the user's source silhouette image.
    QGraphicsRectItem* haloRect = m_scene->addRect(
        pixmap.rect().adjusted(-10, -10, 10, 10),
        QPen(accent, 3.0),
        QBrush(QColor(accent.red(), accent.green(), accent.blue(), 24)));
    haloRect->setZValue(-3.0);

    m_scene->setSceneRect(pixmap.rect().adjusted(-14, -14, 14, 14));

    buildSkeleton();
    buildHotspots();
    updateOverlayVisibility();
    fitBody();
}

QPointF BDFRBodyViewer::normalizedToScene(const QPointF& p) const
{
    if (!m_bodyItem || m_bodyItem->pixmap().isNull())
        return QPointF();

    const QSize size = m_bodyItem->pixmap().size();
    return QPointF(p.x() * size.width(), p.y() * size.height());
}

QPointF BDFRBodyViewer::sceneToNormalized(const QPointF& p) const
{
    if (!m_bodyItem || m_bodyItem->pixmap().isNull())
        return QPointF();

    const QSize size = m_bodyItem->pixmap().size();
    return QPointF(
        qBound(0.0, p.x() / qMax(1, size.width()), 1.0),
        qBound(0.0, p.y() / qMax(1, size.height()), 1.0));
}

QGraphicsEllipseItem* BDFRBodyViewer::makeJoint(const QString& name, const QPointF& p)
{
    const QPointF s = normalizedToScene(p);
    QGraphicsEllipseItem* item = m_scene->addEllipse(
        QRectF(s.x() - 4.0, s.y() - 4.0, 8.0, 8.0),
        QPen(Qt::white, 1.2), QBrush(jointColor()));
    item->setZValue(4.0);
    m_jointItems.append(item);

    QGraphicsTextItem* label = m_scene->addText(name);
    label->setDefaultTextColor(QColor(210, 235, 255));
    label->setPos(s + QPointF(6.0, -8.0));
    label->setScale(0.65);
    label->setZValue(5.0);
    m_jointLabelItems.append(label);
    return item;
}

void BDFRBodyViewer::makeBone(const QString& a, const QString& b)
{
    if (!m_joints.contains(a) || !m_joints.contains(b))
        return;

    QGraphicsLineItem* item = m_scene->addLine(
        QLineF(normalizedToScene(m_joints[a]), normalizedToScene(m_joints[b])),
        QPen(QColor(238, 226, 190), 3.0, Qt::SolidLine, Qt::RoundCap));
    item->setZValue(3.0);
    m_boneItems.append(item);
}

void BDFRBodyViewer::buildSkeleton()
{
    // Normalized Genesis-style front-view skeleton projection. The overlay is
    // intentionally semantic, not a replacement for the real Daz scene rig.
    m_joints["head"] = QPointF(0.50, 0.105);
    m_joints["neck"] = QPointF(0.50, 0.190);
    m_joints["chest"] = QPointF(0.50, 0.285);
    m_joints["spine"] = QPointF(0.50, 0.410);
    m_joints["pelvis"] = QPointF(0.50, 0.525);

    m_joints["shoulder_l"] = QPointF(0.35, 0.245);
    m_joints["elbow_l"] = QPointF(0.26, 0.405);
    m_joints["wrist_l"] = QPointF(0.20, 0.555);
    m_joints["shoulder_r"] = QPointF(0.65, 0.245);
    m_joints["elbow_r"] = QPointF(0.74, 0.405);
    m_joints["wrist_r"] = QPointF(0.80, 0.555);

    m_joints["hip_l"] = QPointF(0.43, 0.555);
    m_joints["knee_l"] = QPointF(0.40, 0.735);
    m_joints["ankle_l"] = QPointF(0.37, 0.920);
    m_joints["hip_r"] = QPointF(0.57, 0.555);
    m_joints["knee_r"] = QPointF(0.60, 0.735);
    m_joints["ankle_r"] = QPointF(0.63, 0.920);

    makeBone("head", "neck");
    makeBone("neck", "chest");
    makeBone("chest", "spine");
    makeBone("spine", "pelvis");

    makeBone("chest", "shoulder_l");
    makeBone("shoulder_l", "elbow_l");
    makeBone("elbow_l", "wrist_l");
    makeBone("chest", "shoulder_r");
    makeBone("shoulder_r", "elbow_r");
    makeBone("elbow_r", "wrist_r");

    makeBone("pelvis", "hip_l");
    makeBone("hip_l", "knee_l");
    makeBone("knee_l", "ankle_l");
    makeBone("pelvis", "hip_r");
    makeBone("hip_r", "knee_r");
    makeBone("knee_r", "ankle_r");

    QMap<QString,QPointF>::const_iterator it = m_joints.constBegin();
    for (; it != m_joints.constEnd(); ++it)
        makeJoint(it.key(), it.value());
}

void BDFRBodyViewer::buildHotspots()
{
    const bool female = m_gender == Female;

    Hotspot h;
    h = Hotspot(); h.presetId = female ? "Breast" : "Pectoral"; h.side = "Left"; h.normalizedCenter = QPointF(0.42, 0.305); h.normalizedSize = QSizeF(0.18,0.10); h.softTissue = female; m_hotspots << h;
    h = Hotspot(); h.presetId = female ? "Breast" : "Pectoral"; h.side = "Right"; h.normalizedCenter = QPointF(0.58, 0.305); h.normalizedSize = QSizeF(0.18,0.10); h.softTissue = female; m_hotspots << h;

    h = Hotspot(); h.presetId = "Biceps"; h.side = "Left"; h.normalizedCenter = QPointF(0.29,0.385); h.normalizedSize = QSizeF(0.10,0.15); h.softTissue = false; m_hotspots << h;
    h = Hotspot(); h.presetId = "Biceps"; h.side = "Right"; h.normalizedCenter = QPointF(0.71,0.385); h.normalizedSize = QSizeF(0.10,0.15); h.softTissue = false; m_hotspots << h;

    h = Hotspot(); h.presetId = "Abdomen"; h.side = "Center"; h.normalizedCenter = QPointF(0.50,0.445); h.normalizedSize = QSizeF(0.20,0.18); h.softTissue = true; m_hotspots << h;

    h = Hotspot(); h.presetId = "Glute"; h.side = "Left"; h.normalizedCenter = QPointF(0.44,0.565); h.normalizedSize = QSizeF(0.17,0.12); h.softTissue = true; m_hotspots << h;
    h = Hotspot(); h.presetId = "Glute"; h.side = "Right"; h.normalizedCenter = QPointF(0.56,0.565); h.normalizedSize = QSizeF(0.17,0.12); h.softTissue = true; m_hotspots << h;

    h = Hotspot(); h.presetId = female ? "Thigh" : "Quadriceps"; h.side = "Left"; h.normalizedCenter = QPointF(0.43,0.660); h.normalizedSize = QSizeF(0.15,0.22); h.softTissue = female; m_hotspots << h;
    h = Hotspot(); h.presetId = female ? "Thigh" : "Quadriceps"; h.side = "Right"; h.normalizedCenter = QPointF(0.57,0.660); h.normalizedSize = QSizeF(0.15,0.22); h.softTissue = female; m_hotspots << h;

    h = Hotspot(); h.presetId = "Calf"; h.side = "Left"; h.normalizedCenter = QPointF(0.40,0.820); h.normalizedSize = QSizeF(0.12,0.20); h.softTissue = false; m_hotspots << h;
    h = Hotspot(); h.presetId = "Calf"; h.side = "Right"; h.normalizedCenter = QPointF(0.60,0.820); h.normalizedSize = QSizeF(0.12,0.20); h.softTissue = false; m_hotspots << h;

    const QColor muscle(255, 82, 92, 135);
    const QColor soft(62, 230, 181, 135);
    const QColor breast(238, 89, 205, 150);
    const QColor glute(139, 89, 238, 150);

    for (int i = 0; i < m_hotspots.count(); ++i)
    {
        const Hotspot& hotspot = m_hotspots.at(i);
        const QPointF center = normalizedToScene(hotspot.normalizedCenter);
        const QSize size = m_bodyItem->pixmap().size();
        const double w = hotspot.normalizedSize.width() * size.width();
        const double hgt = hotspot.normalizedSize.height() * size.height();

        QColor fill = hotspot.softTissue ? soft : muscle;
        if (hotspot.presetId == "Breast") fill = breast;
        if (hotspot.presetId == "Glute") fill = glute;

        QGraphicsEllipseItem* item = m_scene->addEllipse(
            QRectF(center.x() - w*0.5, center.y() - hgt*0.5, w, hgt),
            QPen(fill.lighter(150), 1.4), QBrush(fill));
        item->setData(RolePresetIndex, i);
        item->setZValue(2.0);
        m_hotspotItems.append(item);
    }
}

void BDFRBodyViewer::setActive(bool active)
{
    m_active = active;
    setStyleSheet(active
        ? QString("QGraphicsView { border: 2px solid %1; border-radius: 6px; }").arg((m_gender == Female) ? "#ff42c4" : "#148bff")
        : QString("QGraphicsView { border: 1px solid #294052; border-radius: 6px; }") );
    setWindowOpacity(active ? 1.0 : 0.86);
}

void BDFRBodyViewer::setShowBones(bool show) { m_showBones = show; updateOverlayVisibility(); }
void BDFRBodyViewer::setShowJointNames(bool show) { m_showJointNames = show; updateOverlayVisibility(); }
void BDFRBodyViewer::setShowMuscleRegions(bool show) { m_showMuscleRegions = show; updateOverlayVisibility(); }
void BDFRBodyViewer::setShowSoftTissueRegions(bool show) { m_showSoftTissueRegions = show; updateOverlayVisibility(); }

void BDFRBodyViewer::setXRayOpacity(double value)
{
    m_xrayOpacity = qBound(0.05, value, 1.0);
    if (m_bodyItem) m_bodyItem->setOpacity(m_xrayOpacity);
}

void BDFRBodyViewer::updateOverlayVisibility()
{
    for (int i = 0; i < m_boneItems.count(); ++i)
        m_boneItems.at(i)->setVisible(m_showBones);
    for (int i = 0; i < m_jointItems.count(); ++i)
        m_jointItems.at(i)->setVisible(m_showBones);
    for (int i = 0; i < m_jointLabelItems.count(); ++i)
        m_jointLabelItems.at(i)->setVisible(m_showBones && m_showJointNames);

    for (int i = 0; i < m_hotspotItems.count() && i < m_hotspots.count(); ++i)
    {
        const bool soft = m_hotspots.at(i).softTissue;
        m_hotspotItems.at(i)->setVisible(soft ? m_showSoftTissueRegions : m_showMuscleRegions);
    }
}

void BDFRBodyViewer::fitBody()
{
    if (!m_scene) return;
    fitInView(m_scene->sceneRect(), Qt::KeepAspectRatio);
}

void BDFRBodyViewer::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    fitBody();
}

void BDFRBodyViewer::mousePressEvent(QMouseEvent* event)
{
    if (!m_active)
        emit viewerActivated((int)m_gender);

    const QPointF scenePoint = mapToScene(event->pos());
    const QList<QGraphicsItem*> clickedItems = m_scene->items(scenePoint);

    for (int i = 0; i < clickedItems.count(); ++i)
    {
        bool ok = false;
        const int index = clickedItems.at(i)->data(RolePresetIndex).toInt(&ok);
        if (ok && index >= 0 && index < m_hotspots.count())
        {
            const Hotspot& hotspot = m_hotspots.at(index);
            m_lastNormalizedClick = sceneToNormalized(scenePoint);
            m_lastPresetId = hotspot.presetId;
            m_lastSide = hotspot.side;
            emit anatomyPresetClicked(hotspot.presetId, hotspot.side, m_lastNormalizedClick);
            event->accept();
            return;
        }
    }

    m_lastNormalizedClick = sceneToNormalized(scenePoint);
    QGraphicsView::mousePressEvent(event);
}
