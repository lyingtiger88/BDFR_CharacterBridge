#pragma once

#include <QtGui/QDialog>
#include <QtCore/QString>
#include "BDFRBodyProfile.h"

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSlider;
class QTableWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QLabel;
class BDFRBodyViewer;

class BDFRFloatControl : public QWidget
{
    Q_OBJECT
public:
    BDFRFloatControl(const QString& label, double minValue, double maxValue, double value, int decimals, QWidget* parent = 0);
    double value() const;
    void setValue(double value);
    QLabel* labelWidget() const { return m_label; }

signals:
    void valueChanged(double value);

private slots:
    void onSliderChanged(int value);
    void onSpinChanged(double value);

private:
    int toSlider(double value) const;
    double fromSlider(int value) const;

    QLabel* m_label;
    QSlider* m_slider;
    QDoubleSpinBox* m_spin;
    double m_min;
    double m_max;
    bool m_updating;
};

class BDFRBodyAuthoringDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BDFRBodyAuthoringDialog(QWidget* parent = 0);
    virtual ~BDFRBodyAuthoringDialog();

private slots:
    void onGenderChanged(int id);
    void onViewerActivated(int gender);
    void onPresetClicked(const QString& presetId, const QString& side, const QPointF& normalizedPoint);
    void onRegionTypeQuickAdd(int index);
    void onSymmetryChanged(int id);
    void onAddRegion();
    void onRemoveSelected();
    void onClearAll();
    void onCreateFromSelection();
    void onRegionTreeSelectionChanged();
    void onInspectorChanged();
    void onShowBonesChanged(bool value);
    void onShowJointNamesChanged(bool value);
    void onShowMusclesChanged(bool value);
    void onShowSoftChanged(bool value);
    void onXRayChanged(double value);
    void onAddClip();
    void onRemoveClip();
    void onApplyClip();
    void onPlayPauseTimeline();
    void onExportProfile();
    void onHelp();
    void onPresets();
    void onSettings();

private:
    QWidget* buildLeftPanel();
    QWidget* buildCenterPanel();
    QWidget* buildRightPanel();
    QWidget* buildBottomPanel();

    void applyDarkStyle();
    void refreshCharacterInfo();
    void refreshViewerActivation();
    void refreshRegionTree();
    void refreshInspectorFromSelection();
    void pushInspectorToSelection();
    void selectRegion(int index);
    int selectedRegionIndex() const;
    QString selectedGenderName() const;
    QString detectedGenesisProfile() const;
    QString selectedSymmetry() const;
    void addRegionFromPreset(const QString& presetId, const QString& side, const QPointF& normalizedPoint, bool selectAfterAdd = true);
    void addMirroredRegion(const BDFRBodyRegion& source, const QString& targetSide);
    QString sideForPoint(const QPointF& normalizedPoint) const;

private:
    BDFRBodyProfile m_profile;
    int m_selectedRegionIndex;
    QString m_pendingPresetId;
    QString m_pendingSide;
    QPointF m_pendingNormalizedPoint;
    QString m_quickRegionType;

    QButtonGroup* m_genderGroup;
    QRadioButton* m_autoGender;
    QRadioButton* m_femaleGender;
    QRadioButton* m_maleGender;
    QLabel* m_detectedFigureLabel;
    QLabel* m_skeletonLabel;

    QCheckBox* m_showBones;
    QCheckBox* m_showJointNames;
    QCheckBox* m_showMuscles;
    QCheckBox* m_showSoft;
    BDFRFloatControl* m_xrayControl;

    QComboBox* m_quickRegionCombo;
    QButtonGroup* m_symmetryGroup;

    BDFRBodyViewer* m_femaleViewer;
    BDFRBodyViewer* m_maleViewer;

    QLineEdit* m_regionName;
    QComboBox* m_regionType;
    QComboBox* m_regionSide;
    QLineEdit* m_anchorA;
    QLineEdit* m_anchorB;
    BDFRFloatControl* m_radiusX;
    BDFRFloatControl* m_radiusY;
    BDFRFloatControl* m_radiusZ;
    BDFRFloatControl* m_mass;
    BDFRFloatControl* m_stiffness;
    BDFRFloatControl* m_damping;
    BDFRFloatControl* m_compliance;
    BDFRFloatControl* m_activation;
    BDFRFloatControl* m_maxBulge;
    BDFRFloatControl* m_collision;
    QCheckBox* m_previewRegion;
    QTreeWidget* m_regionTree;

    QTableWidget* m_clipTable;
    QPushButton* m_playPauseButton;
    QLineEdit* m_exportFilename;
    QCheckBox* m_includeAnimation;
    QCheckBox* m_includeMarkers;
    QCheckBox* m_coordinateConversion;
};
