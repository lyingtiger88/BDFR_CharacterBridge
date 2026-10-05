#include "BDFRBodyAuthoringDialog.h"
#include "BDFRBodyViewer.h"

#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDesktopWidget>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFileDialog>
#include <QtGui/QFormLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QInputDialog>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSlider>
#include <QtGui/QSplitter>
#include <QtGui/QTableWidget>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QTreeWidget>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QVBoxLayout>
#include <QtCore/QFileInfo>

#include <dzapp.h>
#include <dzcontentmgr.h>
#include <dznode.h>
#include <dzscene.h>

namespace
{
QGroupBox* makeGroup(const QString& title, QWidget* parent)
{
    QGroupBox* box = new QGroupBox(title, parent);
    box->setObjectName("BDFRGroup");
    return box;
}

QPushButton* makeButton(const QString& text, QWidget* parent, const QString& objectName = QString())
{
    QPushButton* button = new QPushButton(text, parent);
    if (!objectName.isEmpty())
        button->setObjectName(objectName);
    return button;
}

QString regionDisplayType(const BDFRBodyRegion& region)
{
    return region.type.isEmpty() ? QString("Custom") : region.type;
}
}

BDFRFloatControl::BDFRFloatControl(const QString& label,
                                   double minValue,
                                   double maxValue,
                                   double value,
                                   int decimals,
                                   QWidget* parent)
    : QWidget(parent),
      m_label(new QLabel(label, this)),
      m_slider(new QSlider(Qt::Horizontal, this)),
      m_spin(new QDoubleSpinBox(this)),
      m_min(minValue),
      m_max(maxValue),
      m_updating(false)
{
    QHBoxLayout* row = new QHBoxLayout(this);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(6);

    m_label->setMinimumWidth(92);
    m_slider->setRange(0, 1000);
    m_spin->setRange(minValue, maxValue);
    m_spin->setDecimals(decimals);
    m_spin->setSingleStep((maxValue - minValue) / 100.0);
    m_spin->setMinimumWidth(72);

    row->addWidget(m_label);
    row->addWidget(m_slider, 1);
    row->addWidget(m_spin);

    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderChanged(int)));
    connect(m_spin, SIGNAL(valueChanged(double)), this, SLOT(onSpinChanged(double)));
    setValue(value);
}

double BDFRFloatControl::value() const
{
    return m_spin->value();
}

int BDFRFloatControl::toSlider(double value) const
{
    if (m_max <= m_min)
        return 0;
    return qRound(((value - m_min) / (m_max - m_min)) * 1000.0);
}

double BDFRFloatControl::fromSlider(int value) const
{
    return m_min + ((m_max - m_min) * ((double)value / 1000.0));
}

void BDFRFloatControl::setValue(double value)
{
    m_updating = true;
    value = qBound(m_min, value, m_max);
    m_slider->setValue(toSlider(value));
    m_spin->setValue(value);
    m_updating = false;
}

void BDFRFloatControl::onSliderChanged(int value)
{
    if (m_updating)
        return;
    m_updating = true;
    const double converted = fromSlider(value);
    m_spin->setValue(converted);
    m_updating = false;
    emit valueChanged(converted);
}

void BDFRFloatControl::onSpinChanged(double value)
{
    if (m_updating)
        return;
    m_updating = true;
    m_slider->setValue(toSlider(value));
    m_updating = false;
    emit valueChanged(value);
}

BDFRBodyAuthoringDialog::BDFRBodyAuthoringDialog(QWidget* parent)
    : QDialog(parent),
      m_selectedRegionIndex(-1),
      m_quickRegionType("Muscle"),
      m_genderGroup(0),
      m_autoGender(0),
      m_femaleGender(0),
      m_maleGender(0),
      m_detectedFigureLabel(0),
      m_skeletonLabel(0),
      m_showBones(0),
      m_showJointNames(0),
      m_showMuscles(0),
      m_showSoft(0),
      m_xrayControl(0),
      m_quickRegionCombo(0),
      m_symmetryGroup(0),
      m_femaleViewer(0),
      m_maleViewer(0),
      m_regionName(0),
      m_regionType(0),
      m_regionSide(0),
      m_anchorA(0),
      m_anchorB(0),
      m_radiusX(0),
      m_radiusY(0),
      m_radiusZ(0),
      m_mass(0),
      m_stiffness(0),
      m_damping(0),
      m_compliance(0),
      m_activation(0),
      m_maxBulge(0),
      m_collision(0),
      m_previewRegion(0),
      m_regionTree(0),
      m_clipTable(0),
      m_playPauseButton(0),
      m_exportFilename(0),
      m_includeAnimation(0),
      m_includeMarkers(0),
      m_coordinateConversion(0)
{
    setObjectName("BDFR_BodyAuthoring_Dialog");
    setWindowTitle("BDFR Body Authoring v0.5.0");
    setModal(false);

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(7);

    QWidget* header = new QWidget(this);
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(8, 3, 8, 3);
    QLabel* title = new QLabel("<b style='font-size:18px'>BDFR Body Authoring</b><br/><span style='color:#9fbad0'>Muscle & Soft Tissue Setup for Daz -> Unreal</span>", header);
    headerLayout->addWidget(title, 1);
    QPushButton* help = makeButton("Help", header);
    QPushButton* presets = makeButton("Presets", header);
    QPushButton* settings = makeButton("Settings", header);
    headerLayout->addWidget(help);
    headerLayout->addWidget(presets);
    headerLayout->addWidget(settings);
    connect(help, SIGNAL(clicked()), this, SLOT(onHelp()));
    connect(presets, SIGNAL(clicked()), this, SLOT(onPresets()));
    connect(settings, SIGNAL(clicked()), this, SLOT(onSettings()));
    root->addWidget(header);

    QSplitter* upper = new QSplitter(Qt::Horizontal, this);
    upper->setChildrenCollapsible(false);
    upper->addWidget(buildLeftPanel());
    upper->addWidget(buildCenterPanel());
    upper->addWidget(buildRightPanel());
    upper->setStretchFactor(0, 0);
    upper->setStretchFactor(1, 1);
    upper->setStretchFactor(2, 0);
    root->addWidget(upper, 1);
    root->addWidget(buildBottomPanel());

    applyDarkStyle();
    refreshCharacterInfo();
    refreshViewerActivation();
    refreshRegionTree();

    QRect available = QApplication::desktop()->availableGeometry(this);
    resize(qMin(1400, qMax(980, available.width() - 80)),
           qMin(900, qMax(700, available.height() - 100)));
}

BDFRBodyAuthoringDialog::~BDFRBodyAuthoringDialog()
{
}

QWidget* BDFRBodyAuthoringDialog::buildLeftPanel()
{
    QWidget* panel = new QWidget(this);
    panel->setMinimumWidth(250);
    panel->setMaximumWidth(330);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* character = makeGroup("1. Character & Gender", panel);
    QVBoxLayout* c = new QVBoxLayout(character);
    QHBoxLayout* genders = new QHBoxLayout();
    m_genderGroup = new QButtonGroup(this);
    m_autoGender = new QRadioButton("Auto", character);
    m_femaleGender = new QRadioButton("Female", character);
    m_maleGender = new QRadioButton("Male", character);
    m_autoGender->setChecked(true);
    m_genderGroup->addButton(m_autoGender, 0);
    m_genderGroup->addButton(m_femaleGender, 1);
    m_genderGroup->addButton(m_maleGender, 2);
    genders->addWidget(m_autoGender);
    genders->addWidget(m_femaleGender);
    genders->addWidget(m_maleGender);
    c->addLayout(genders);
    m_detectedFigureLabel = new QLabel("Detected Figure: -", character);
    m_skeletonLabel = new QLabel("Skeleton: -", character);
    c->addWidget(m_detectedFigureLabel);
    c->addWidget(m_skeletonLabel);
    c->addWidget(new QLabel("Scale Unit: Centimeters (cm)", character));
    connect(m_genderGroup, SIGNAL(buttonClicked(int)), this, SLOT(onGenderChanged(int)));
    layout->addWidget(character);

    QGroupBox* view = makeGroup("2. View Mode", panel);
    QVBoxLayout* v = new QVBoxLayout(view);
    m_showBones = new QCheckBox("Show Bones", view);
    m_showBones->setChecked(true);
    m_showJointNames = new QCheckBox("Show Joint Names", view);
    m_showMuscles = new QCheckBox("Show Muscle Regions", view);
    m_showMuscles->setChecked(true);
    m_showSoft = new QCheckBox("Show Soft Tissue Regions", view);
    m_showSoft->setChecked(true);
    m_xrayControl = new BDFRFloatControl("X-Ray View", 0.05, 1.0, 0.72, 2, view);
    v->addWidget(m_showBones);
    v->addWidget(m_showJointNames);
    v->addWidget(m_showMuscles);
    v->addWidget(m_showSoft);
    v->addWidget(m_xrayControl);
    connect(m_showBones, SIGNAL(toggled(bool)), this, SLOT(onShowBonesChanged(bool)));
    connect(m_showJointNames, SIGNAL(toggled(bool)), this, SLOT(onShowJointNamesChanged(bool)));
    connect(m_showMuscles, SIGNAL(toggled(bool)), this, SLOT(onShowMusclesChanged(bool)));
    connect(m_showSoft, SIGNAL(toggled(bool)), this, SLOT(onShowSoftChanged(bool)));
    connect(m_xrayControl, SIGNAL(valueChanged(double)), this, SLOT(onXRayChanged(double)));
    layout->addWidget(view);

    QGroupBox* region = makeGroup("3. Region Type (Quick Add)", panel);
    QVBoxLayout* r = new QVBoxLayout(region);
    m_quickRegionCombo = new QComboBox(region);
    m_quickRegionCombo->addItems(QStringList() << "Muscle" << "SoftTissue" << "FatPad" << "Breast" << "Glute" << "Custom");
    r->addWidget(m_quickRegionCombo);
    connect(m_quickRegionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onRegionTypeQuickAdd(int)));
    layout->addWidget(region);

    QGroupBox* symmetry = makeGroup("4. Symmetry", panel);
    QHBoxLayout* s = new QHBoxLayout(symmetry);
    m_symmetryGroup = new QButtonGroup(this);
    QRadioButton* left = new QRadioButton("Left", symmetry);
    QRadioButton* right = new QRadioButton("Right", symmetry);
    QRadioButton* both = new QRadioButton("Both", symmetry);
    both->setChecked(true);
    m_symmetryGroup->addButton(left, 0);
    m_symmetryGroup->addButton(right, 1);
    m_symmetryGroup->addButton(both, 2);
    s->addWidget(left);
    s->addWidget(right);
    s->addWidget(both);
    connect(m_symmetryGroup, SIGNAL(buttonClicked(int)), this, SLOT(onSymmetryChanged(int)));
    layout->addWidget(symmetry);

    QGroupBox* tools = makeGroup("5. Tools", panel);
    QGridLayout* t = new QGridLayout(tools);
    QPushButton* add = makeButton("Add Region", tools);
    QPushButton* remove = makeButton("Remove Selected", tools);
    QPushButton* clear = makeButton("Clear All", tools);
    QPushButton* fromSelection = makeButton("Create From Selection", tools);
    t->addWidget(add, 0, 0);
    t->addWidget(remove, 0, 1);
    t->addWidget(clear, 1, 0);
    t->addWidget(fromSelection, 1, 1);
    connect(add, SIGNAL(clicked()), this, SLOT(onAddRegion()));
    connect(remove, SIGNAL(clicked()), this, SLOT(onRemoveSelected()));
    connect(clear, SIGNAL(clicked()), this, SLOT(onClearAll()));
    connect(fromSelection, SIGNAL(clicked()), this, SLOT(onCreateFromSelection()));
    layout->addWidget(tools);
    layout->addStretch(1);
    return panel;
}

QWidget* BDFRBodyAuthoringDialog::buildCenterPanel()
{
    QWidget* panel = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(7);

    QGroupBox* female = makeGroup("Female (Genesis)", panel);
    QVBoxLayout* f = new QVBoxLayout(female);
    m_femaleViewer = new BDFRBodyViewer(BDFRBodyViewer::Female, female);
    f->addWidget(m_femaleViewer, 1);
    QLabel* ff = new QLabel("Front  |  Back/Side maps pending approved references", female);
    ff->setAlignment(Qt::AlignCenter);
    f->addWidget(ff);

    QGroupBox* male = makeGroup("Male (Genesis)", panel);
    QVBoxLayout* m = new QVBoxLayout(male);
    m_maleViewer = new BDFRBodyViewer(BDFRBodyViewer::Male, male);
    m->addWidget(m_maleViewer, 1);
    QLabel* mf = new QLabel("Front  |  Back/Side maps pending approved references", male);
    mf->setAlignment(Qt::AlignCenter);
    m->addWidget(mf);

    layout->addWidget(female, 1);
    layout->addWidget(male, 1);

    connect(m_femaleViewer, SIGNAL(anatomyPresetClicked(QString,QString,QPointF)), this, SLOT(onPresetClicked(QString,QString,QPointF)));
    connect(m_maleViewer, SIGNAL(anatomyPresetClicked(QString,QString,QPointF)), this, SLOT(onPresetClicked(QString,QString,QPointF)));
    connect(m_femaleViewer, SIGNAL(viewerActivated(int)), this, SLOT(onViewerActivated(int)));
    connect(m_maleViewer, SIGNAL(viewerActivated(int)), this, SLOT(onViewerActivated(int)));
    return panel;
}

QWidget* BDFRBodyAuthoringDialog::buildRightPanel()
{
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setMinimumWidth(315);
    scroll->setMaximumWidth(390);

    QWidget* panel = new QWidget(scroll);
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* selected = makeGroup("Selected Region", panel);
    QFormLayout* form = new QFormLayout(selected);
    m_regionName = new QLineEdit(selected);
    m_regionType = new QComboBox(selected);
    m_regionType->addItems(QStringList() << "Muscle" << "SoftTissue" << "FatPad" << "Breast" << "Glute" << "Custom");
    m_regionSide = new QComboBox(selected);
    m_regionSide->addItems(QStringList() << "Left" << "Right" << "Center");
    m_anchorA = new QLineEdit(selected);
    m_anchorB = new QLineEdit(selected);
    form->addRow("Name:", m_regionName);
    form->addRow("Region Type:", m_regionType);
    form->addRow("Side:", m_regionSide);
    form->addRow("Anchor A:", m_anchorA);
    form->addRow("Anchor B:", m_anchorB);
    m_radiusX = new BDFRFloatControl("Radius X", 0.1, 30.0, 6.0, 1, selected);
    m_radiusY = new BDFRFloatControl("Radius Y", 0.1, 30.0, 5.0, 1, selected);
    m_radiusZ = new BDFRFloatControl("Radius Z", 0.1, 30.0, 4.0, 1, selected);
    form->addRow(m_radiusX);
    form->addRow(m_radiusY);
    form->addRow(m_radiusZ);
    layout->addWidget(selected);

    QGroupBox* physics = makeGroup("Physical Parameters", panel);
    QVBoxLayout* p = new QVBoxLayout(physics);
    m_mass = new BDFRFloatControl("Mass (kg)", 0.0, 10.0, 0.4, 2, physics);
    m_stiffness = new BDFRFloatControl("Stiffness", 0.0, 1.0, 0.8, 2, physics);
    m_damping = new BDFRFloatControl("Damping", 0.0, 1.0, 0.3, 2, physics);
    m_compliance = new BDFRFloatControl("Compliance", 0.0, 1.0, 0.2, 2, physics);
    m_activation = new BDFRFloatControl("Activation Scale", 0.0, 2.0, 1.0, 2, physics);
    m_maxBulge = new BDFRFloatControl("Max Bulge", 0.0, 1.0, 0.15, 2, physics);
    m_collision = new BDFRFloatControl("Collision Scale", 0.1, 2.0, 1.0, 2, physics);
    m_previewRegion = new QCheckBox("Preview Region", physics);
    m_previewRegion->setChecked(true);
    p->addWidget(m_mass);
    p->addWidget(m_stiffness);
    p->addWidget(m_damping);
    p->addWidget(m_compliance);
    p->addWidget(m_activation);
    p->addWidget(m_maxBulge);
    p->addWidget(m_collision);
    p->addWidget(m_previewRegion);
    layout->addWidget(physics);

    QGroupBox* list = makeGroup("Region List", panel);
    QVBoxLayout* l = new QVBoxLayout(list);
    m_regionTree = new QTreeWidget(list);
    m_regionTree->setColumnCount(3);
    m_regionTree->setHeaderLabels(QStringList() << "Name" << "Type" << "Side");
    m_regionTree->header()->setResizeMode(0, QHeaderView::Stretch);
    m_regionTree->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    m_regionTree->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    l->addWidget(m_regionTree);
    connect(m_regionTree, SIGNAL(itemSelectionChanged()), this, SLOT(onRegionTreeSelectionChanged()));
    layout->addWidget(list, 1);

    connect(m_regionName, SIGNAL(textEdited(QString)), this, SLOT(onInspectorChanged()));
    connect(m_regionType, SIGNAL(currentIndexChanged(int)), this, SLOT(onInspectorChanged()));
    connect(m_regionSide, SIGNAL(currentIndexChanged(int)), this, SLOT(onInspectorChanged()));
    connect(m_anchorA, SIGNAL(textEdited(QString)), this, SLOT(onInspectorChanged()));
    connect(m_anchorB, SIGNAL(textEdited(QString)), this, SLOT(onInspectorChanged()));
    connect(m_radiusX, SIGNAL(valueChanged(double)), this, SLOT(onInspectorChanged()));
    connect(m_radiusY, SIGNAL(valueChanged(double)), this, SLOT(onInspectorChanged()));
    connect(m_radiusZ, SIGNAL(valueChanged(double)), this, SLOT(onInspectorChanged()));
    connect(m_mass, SIGNAL(valueChanged(double)), this, SLOT(onInspectorChanged()));
    connect(m_stiffness, SIGNAL(valueChanged(double)), this, SLOT(onInspectorChanged()));
    connect(m_damping, SIGNAL(valueChanged(double)), this, SLOT(onInspectorChanged()));
    connect(m_compliance, SIGNAL(valueChanged(double)), this, SLOT(onInspectorChanged()));
    connect(m_activation, SIGNAL(valueChanged(double)), this, SLOT(onInspectorChanged()));
    connect(m_maxBulge, SIGNAL(valueChanged(double)), this, SLOT(onInspectorChanged()));
    connect(m_collision, SIGNAL(valueChanged(double)), this, SLOT(onInspectorChanged()));
    connect(m_previewRegion, SIGNAL(toggled(bool)), this, SLOT(onInspectorChanged()));

    scroll->setWidget(panel);
    return scroll;
}

QWidget* BDFRBodyAuthoringDialog::buildBottomPanel()
{
    QWidget* panel = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);

    QGroupBox* clips = makeGroup("Animation Test Clips", panel);
    QVBoxLayout* cl = new QVBoxLayout(clips);
    m_clipTable = new QTableWidget(0, 4, clips);
    m_clipTable->setHorizontalHeaderLabels(QStringList() << "Name" << "Type" << "File Path" << "Loop");
    m_clipTable->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    m_clipTable->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    m_clipTable->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
    m_clipTable->horizontalHeader()->setResizeMode(3, QHeaderView::ResizeToContents);
    cl->addWidget(m_clipTable);
    QHBoxLayout* clipButtons = new QHBoxLayout();
    QPushButton* addClip = makeButton("Add Clip", clips);
    QPushButton* removeClip = makeButton("Remove Clip", clips);
    QPushButton* applyClip = makeButton("Apply Clip", clips);
    clipButtons->addWidget(addClip);
    clipButtons->addWidget(removeClip);
    clipButtons->addWidget(applyClip);
    cl->addLayout(clipButtons);
    connect(addClip, SIGNAL(clicked()), this, SLOT(onAddClip()));
    connect(removeClip, SIGNAL(clicked()), this, SLOT(onRemoveClip()));
    connect(applyClip, SIGNAL(clicked()), this, SLOT(onApplyClip()));

    BDFRAnimationClip walk; walk.name = "Walk (Default)"; walk.type = "Walk"; walk.loop = true;
    BDFRAnimationClip run; run.name = "Run (Default)"; run.type = "Run"; run.loop = true;
    m_profile.animationClips << walk << run;

    for (int i = 0; i < m_profile.animationClips.count(); ++i)
    {
        const BDFRAnimationClip& clip = m_profile.animationClips.at(i);
        const int row = m_clipTable->rowCount();
        m_clipTable->insertRow(row);
        m_clipTable->setItem(row, 0, new QTableWidgetItem(clip.name));
        m_clipTable->setItem(row, 1, new QTableWidgetItem(clip.type));
        m_clipTable->setItem(row, 2, new QTableWidgetItem(clip.sourceFile));
        m_clipTable->setItem(row, 3, new QTableWidgetItem(clip.loop ? "Yes" : "No"));
    }

    QGroupBox* preview = makeGroup("Preview", panel);
    QVBoxLayout* pr = new QVBoxLayout(preview);
    pr->addWidget(new QLabel("Daz Timeline Preview", preview));
    m_playPauseButton = makeButton("Play / Pause", preview);
    pr->addWidget(m_playPauseButton);
    connect(m_playPauseButton, SIGNAL(clicked()), this, SLOT(onPlayPauseTimeline()));

    QGroupBox* exp = makeGroup("Export / Transfer", panel);
    QFormLayout* ex = new QFormLayout(exp);
    m_exportFilename = new QLineEdit("Character.bdfbody.json", exp);
    m_includeAnimation = new QCheckBox("Include Animation Clips", exp);
    m_includeMarkers = new QCheckBox("Include Preview Mesh Markers", exp);
    m_coordinateConversion = new QCheckBox("Apply Coordinate Conversion (Daz -> Unreal)", exp);
    m_includeAnimation->setChecked(true);
    m_includeMarkers->setChecked(true);
    m_coordinateConversion->setChecked(true);
    QPushButton* exportButton = makeButton("Export Body Profile", exp, "BDFRPrimaryButton");
    ex->addRow("File Name:", m_exportFilename);
    ex->addRow(m_includeAnimation);
    ex->addRow(m_includeMarkers);
    ex->addRow(m_coordinateConversion);
    ex->addRow(exportButton);
    connect(exportButton, SIGNAL(clicked()), this, SLOT(onExportProfile()));

    layout->addWidget(clips, 2);
    layout->addWidget(preview, 1);
    layout->addWidget(exp, 1);
    return panel;
}

void BDFRBodyAuthoringDialog::applyDarkStyle()
{
    setStyleSheet(
        "QDialog { background:#071827; color:#dbeafa; }"
        "QWidget { color:#dbeafa; font-size:11px; }"
        "QGroupBox#BDFRGroup { border:1px solid #23445e; border-radius:6px; margin-top:10px; padding-top:8px; background:#0b2234; font-weight:bold; }"
        "QGroupBox#BDFRGroup::title { subcontrol-origin:margin; left:8px; padding:0 4px; color:#f2f7fb; }"
        "QLineEdit,QComboBox,QDoubleSpinBox,QTreeWidget,QTableWidget { background:#0a1d2c; border:1px solid #2c4e67; border-radius:4px; padding:3px; selection-background-color:#0d75ff; }"
        "QPushButton { background:#102c42; border:1px solid #31536c; border-radius:4px; padding:6px 10px; }"
        "QPushButton:hover { background:#173b56; }"
        "QPushButton#BDFRPrimaryButton { background:#0877ff; border-color:#31a0ff; font-weight:bold; }"
        "QCheckBox,QRadioButton { spacing:5px; }"
        "QSlider::groove:horizontal { height:5px; background:#24465e; border-radius:2px; }"
        "QSlider::sub-page:horizontal { background:#168cff; border-radius:2px; }"
        "QSlider::handle:horizontal { width:12px; margin:-4px 0; background:#f6fbff; border-radius:6px; }"
        "QHeaderView::section { background:#0d2638; color:#cce6f8; padding:4px; border:0; border-right:1px solid #28475e; }"
    );
}

QString BDFRBodyAuthoringDialog::detectedGenesisProfile() const
{
    DzNode* node = dzScene ? dzScene->getPrimarySelection() : 0;
    const QString label = node ? node->getLabel() : QString();
    if (label.contains("Genesis 9", Qt::CaseInsensitive) || label.contains("Genesis9", Qt::CaseInsensitive))
        return "Genesis 9";
    if (label.contains("Genesis 8.1", Qt::CaseInsensitive) || label.contains("Genesis81", Qt::CaseInsensitive))
        return "Genesis 8.1";
    if (label.contains("Genesis 8", Qt::CaseInsensitive) || label.contains("Genesis8", Qt::CaseInsensitive))
        return "Genesis 8";
    return "Genesis 9";
}

QString BDFRBodyAuthoringDialog::selectedGenderName() const
{
    if (m_femaleGender && m_femaleGender->isChecked()) return "Female";
    if (m_maleGender && m_maleGender->isChecked()) return "Male";

    DzNode* node = dzScene ? dzScene->getPrimarySelection() : 0;
    const QString label = node ? node->getLabel() : QString();
    if (label.contains("Male", Qt::CaseInsensitive)) return "Male";
    if (label.contains("Female", Qt::CaseInsensitive)) return "Female";
    return "Female";
}

void BDFRBodyAuthoringDialog::refreshCharacterInfo()
{
    DzNode* node = dzScene ? dzScene->getPrimarySelection() : 0;
    const QString label = node ? node->getLabel() : QString("No figure selected");
    const QString genesis = detectedGenesisProfile();
    m_profile.characterName = label;
    m_profile.genesisProfile = genesis;
    m_profile.gender = selectedGenderName();
    if (m_detectedFigureLabel) m_detectedFigureLabel->setText("Detected Figure: " + label);
    if (m_skeletonLabel) m_skeletonLabel->setText("Skeleton: " + genesis);
    if (m_exportFilename && !label.isEmpty() && label != "No figure selected")
    {
        QString safe = label;
        safe.replace(QRegExp("[^A-Za-z0-9_]+"), "_");
        m_exportFilename->setText(safe + ".bdfbody.json");
    }
}

void BDFRBodyAuthoringDialog::onGenderChanged(int)
{
    refreshCharacterInfo();
    refreshViewerActivation();
}

void BDFRBodyAuthoringDialog::onViewerActivated(int gender)
{
    if (gender == (int)BDFRBodyViewer::Female)
        m_femaleGender->setChecked(true);
    else
        m_maleGender->setChecked(true);
    refreshCharacterInfo();
    refreshViewerActivation();
}

void BDFRBodyAuthoringDialog::refreshViewerActivation()
{
    const QString gender = selectedGenderName();
    if (m_femaleViewer) m_femaleViewer->setActive(gender == "Female");
    if (m_maleViewer) m_maleViewer->setActive(gender == "Male");
}

QString BDFRBodyAuthoringDialog::selectedSymmetry() const
{
    if (!m_symmetryGroup) return "Both";
    const int id = m_symmetryGroup->checkedId();
    if (id == 0) return "Left";
    if (id == 1) return "Right";
    return "Both";
}

QString BDFRBodyAuthoringDialog::sideForPoint(const QPointF& normalizedPoint) const
{
    return normalizedPoint.x() < 0.5 ? "Left" : "Right";
}

void BDFRBodyAuthoringDialog::onPresetClicked(const QString& presetId,
                                               const QString& side,
                                               const QPointF& normalizedPoint)
{
    m_pendingPresetId = presetId;
    m_pendingSide = side;
    m_pendingNormalizedPoint = normalizedPoint;

    QString requestedSide = selectedSymmetry();
    if (requestedSide == "Both" && side != "Center")
    {
        addRegionFromPreset(presetId, side, normalizedPoint, true);
        const QString mirrorSide = side == "Left" ? "Right" : "Left";
        QPointF mirror(1.0 - normalizedPoint.x(), normalizedPoint.y());
        addRegionFromPreset(presetId, mirrorSide, mirror, false);
    }
    else
    {
        const QString finalSide = (requestedSide == "Both") ? side : requestedSide;
        QPointF p = normalizedPoint;
        if (finalSide == "Left" && p.x() > 0.5) p.setX(1.0 - p.x());
        if (finalSide == "Right" && p.x() < 0.5) p.setX(1.0 - p.x());
        addRegionFromPreset(presetId, finalSide, p, true);
    }
}

void BDFRBodyAuthoringDialog::addRegionFromPreset(const QString& presetId,
                                                   const QString& side,
                                                   const QPointF& normalizedPoint,
                                                   bool selectAfterAdd)
{
    BDFRBodyRegion region = BDFRBodyPresets::makeRegion(
        presetId,
        side,
        detectedGenesisProfile(),
        normalizedPoint);

    if (!m_quickRegionType.isEmpty() && presetId == "Custom")
        region.type = m_quickRegionType;

    m_profile.regions.append(region);
    refreshRegionTree();
    if (selectAfterAdd)
        selectRegion(m_profile.regions.count() - 1);
}

void BDFRBodyAuthoringDialog::addMirroredRegion(const BDFRBodyRegion& source, const QString& targetSide)
{
    BDFRBodyRegion mirrored = source;
    mirrored.side = targetSide;
    mirrored.name = source.name;
    if (targetSide == "Left") mirrored.name.replace("_R", "_L");
    if (targetSide == "Right") mirrored.name.replace("_L", "_R");
    mirrored.normalizedCenter.setX(1.0 - source.normalizedCenter.x());
    mirrored.id = source.id + "_" + targetSide;
    m_profile.regions.append(mirrored);
}

void BDFRBodyAuthoringDialog::onRegionTypeQuickAdd(int index)
{
    if (m_quickRegionCombo)
        m_quickRegionType = m_quickRegionCombo->itemText(index);
}

void BDFRBodyAuthoringDialog::onSymmetryChanged(int)
{
}

void BDFRBodyAuthoringDialog::onAddRegion()
{
    const QString preset = m_pendingPresetId.isEmpty() ? QString("Custom") : m_pendingPresetId;
    const QString side = m_pendingSide.isEmpty() ? selectedSymmetry() : m_pendingSide;
    const QPointF p = m_pendingNormalizedPoint.isNull() ? QPointF(0.5, 0.5) : m_pendingNormalizedPoint;
    addRegionFromPreset(preset, side == "Both" ? sideForPoint(p) : side, p, true);
}

int BDFRBodyAuthoringDialog::selectedRegionIndex() const
{
    if (!m_regionTree)
        return -1;
    QList<QTreeWidgetItem*> selected = m_regionTree->selectedItems();
    if (selected.isEmpty())
        return -1;
    bool ok = false;
    int index = selected.first()->data(0, Qt::UserRole).toInt(&ok);
    return ok ? index : -1;
}

void BDFRBodyAuthoringDialog::selectRegion(int index)
{
    if (!m_regionTree || index < 0 || index >= m_regionTree->topLevelItemCount())
        return;
    m_regionTree->setCurrentItem(m_regionTree->topLevelItem(index));
}

void BDFRBodyAuthoringDialog::onRemoveSelected()
{
    const int index = selectedRegionIndex();
    if (index < 0 || index >= m_profile.regions.count())
        return;
    m_profile.regions.removeAt(index);
    m_selectedRegionIndex = -1;
    refreshRegionTree();
    refreshInspectorFromSelection();
}

void BDFRBodyAuthoringDialog::onClearAll()
{
    if (m_profile.regions.isEmpty())
        return;
    if (QMessageBox::question(this, "BDFR Body Authoring", "Clear all authored body regions?",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;
    m_profile.regions.clear();
    m_selectedRegionIndex = -1;
    refreshRegionTree();
    refreshInspectorFromSelection();
}

void BDFRBodyAuthoringDialog::onCreateFromSelection()
{
    DzNode* node = dzScene ? dzScene->getPrimarySelection() : 0;
    if (!node)
    {
        QMessageBox::warning(this, "BDFR Body Authoring", "Select a Daz node/bone first.");
        return;
    }

    BDFRBodyRegion region = BDFRBodyPresets::makeRegion(
        "Custom",
        selectedSymmetry() == "Right" ? "Right" : "Left",
        detectedGenesisProfile(),
        QPointF(0.5, 0.5));
    region.type = m_quickRegionType;
    region.name = node->getLabel();
    region.anchorA = node->getName();
    region.anchorB.clear();
    region.id = "BDFR_Selection_" + QString::number(m_profile.regions.count() + 1);
    m_profile.regions.append(region);
    refreshRegionTree();
    selectRegion(m_profile.regions.count() - 1);
}

void BDFRBodyAuthoringDialog::refreshRegionTree()
{
    if (!m_regionTree)
        return;
    m_regionTree->clear();
    for (int i = 0; i < m_profile.regions.count(); ++i)
    {
        const BDFRBodyRegion& region = m_profile.regions.at(i);
        QTreeWidgetItem* item = new QTreeWidgetItem(m_regionTree);
        item->setText(0, region.name);
        item->setText(1, regionDisplayType(region));
        item->setText(2, region.side);
        item->setData(0, Qt::UserRole, i);
    }
}

void BDFRBodyAuthoringDialog::onRegionTreeSelectionChanged()
{
    m_selectedRegionIndex = selectedRegionIndex();
    refreshInspectorFromSelection();
}

void BDFRBodyAuthoringDialog::refreshInspectorFromSelection()
{
    const bool valid = m_selectedRegionIndex >= 0 && m_selectedRegionIndex < m_profile.regions.count();
    if (!valid)
    {
        if (m_regionName) m_regionName->clear();
        return;
    }

    const BDFRBodyRegion& r = m_profile.regions.at(m_selectedRegionIndex);
    m_regionName->setText(r.name);
    int typeIndex = m_regionType->findText(r.type);
    if (typeIndex < 0) typeIndex = m_regionType->findText("Custom");
    m_regionType->setCurrentIndex(typeIndex);
    int sideIndex = m_regionSide->findText(r.side);
    if (sideIndex >= 0) m_regionSide->setCurrentIndex(sideIndex);
    m_anchorA->setText(r.anchorA);
    m_anchorB->setText(r.anchorB);
    m_radiusX->setValue(r.radiusCm.width());
    m_radiusY->setValue(r.radiusCm.height());
    m_radiusZ->setValue(r.depthRadiusCm);
    m_mass->setValue(r.massKg);
    m_stiffness->setValue(r.stiffness);
    m_damping->setValue(r.damping);
    m_compliance->setValue(r.compliance);
    m_activation->setValue(r.activationScale);
    m_maxBulge->setValue(r.maxBulge);
    m_collision->setValue(r.collisionScale);
    m_previewRegion->setChecked(r.enabled);
}

void BDFRBodyAuthoringDialog::pushInspectorToSelection()
{
    if (m_selectedRegionIndex < 0 || m_selectedRegionIndex >= m_profile.regions.count())
        return;

    BDFRBodyRegion& r = m_profile.regions[m_selectedRegionIndex];
    r.name = m_regionName->text();
    r.type = m_regionType->currentText();
    r.side = m_regionSide->currentText();
    r.anchorA = m_anchorA->text();
    r.anchorB = m_anchorB->text();
    r.radiusCm = QSizeF(m_radiusX->value(), m_radiusY->value());
    r.depthRadiusCm = m_radiusZ->value();
    r.massKg = m_mass->value();
    r.stiffness = m_stiffness->value();
    r.damping = m_damping->value();
    r.compliance = m_compliance->value();
    r.activationScale = m_activation->value();
    r.maxBulge = m_maxBulge->value();
    r.collisionScale = m_collision->value();
    r.enabled = m_previewRegion->isChecked();

    QTreeWidgetItem* item = m_regionTree->topLevelItem(m_selectedRegionIndex);
    if (item)
    {
        item->setText(0, r.name);
        item->setText(1, r.type);
        item->setText(2, r.side);
    }
}

void BDFRBodyAuthoringDialog::onInspectorChanged()
{
    pushInspectorToSelection();
}

void BDFRBodyAuthoringDialog::onShowBonesChanged(bool value)
{
    m_femaleViewer->setShowBones(value);
    m_maleViewer->setShowBones(value);
}

void BDFRBodyAuthoringDialog::onShowJointNamesChanged(bool value)
{
    m_femaleViewer->setShowJointNames(value);
    m_maleViewer->setShowJointNames(value);
}

void BDFRBodyAuthoringDialog::onShowMusclesChanged(bool value)
{
    m_femaleViewer->setShowMuscleRegions(value);
    m_maleViewer->setShowMuscleRegions(value);
}

void BDFRBodyAuthoringDialog::onShowSoftChanged(bool value)
{
    m_femaleViewer->setShowSoftTissueRegions(value);
    m_maleViewer->setShowSoftTissueRegions(value);
}

void BDFRBodyAuthoringDialog::onXRayChanged(double value)
{
    m_femaleViewer->setXRayOpacity(value);
    m_maleViewer->setXRayOpacity(value);
}

void BDFRBodyAuthoringDialog::onAddClip()
{
    const QString file = QFileDialog::getOpenFileName(
        this,
        "Add Daz Animation Clip",
        QString(),
        "Daz/Animation Files (*.duf *.dsf *.dsa *.bvh);;All Files (*.*)");
    if (file.isEmpty())
        return;

    BDFRAnimationClip clip;
    clip.sourceFile = file;
    clip.name = QFileInfo(file).baseName();
    clip.type = clip.name.contains("run", Qt::CaseInsensitive) ? "Run" :
                (clip.name.contains("walk", Qt::CaseInsensitive) ? "Walk" : "Custom");
    clip.loop = true;
    m_profile.animationClips.append(clip);

    const int row = m_clipTable->rowCount();
    m_clipTable->insertRow(row);
    m_clipTable->setItem(row, 0, new QTableWidgetItem(clip.name));
    m_clipTable->setItem(row, 1, new QTableWidgetItem(clip.type));
    m_clipTable->setItem(row, 2, new QTableWidgetItem(clip.sourceFile));
    m_clipTable->setItem(row, 3, new QTableWidgetItem(clip.loop ? "Yes" : "No"));
    m_clipTable->selectRow(row);
}

void BDFRBodyAuthoringDialog::onRemoveClip()
{
    const int row = m_clipTable->currentRow();
    if (row < 0)
        return;
    if (row < m_profile.animationClips.count())
        m_profile.animationClips.removeAt(row);
    m_clipTable->removeRow(row);
}

void BDFRBodyAuthoringDialog::onApplyClip()
{
    const int row = m_clipTable->currentRow();
    if (row < 0 || row >= m_profile.animationClips.count())
        return;

    const QString file = m_profile.animationClips.at(row).sourceFile;
    if (file.isEmpty())
    {
        QMessageBox::information(this, "BDFR Body Authoring",
                                 "This default slot has no file assigned yet. Use Add Clip and choose a .duf/.dsa/.bvh file.");
        return;
    }

    DzContentMgr* content = dzApp ? dzApp->getContentMgr() : 0;
    if (!content)
    {
        QMessageBox::warning(this, "BDFR Body Authoring", "Daz Content Manager is unavailable.");
        return;
    }

    content->openFile(file);
}

void BDFRBodyAuthoringDialog::onPlayPauseTimeline()
{
    if (!dzScene)
        return;
    if (dzScene->isPlaying())
        dzScene->pause();
    else
        dzScene->play(true);
}

void BDFRBodyAuthoringDialog::onExportProfile()
{
    refreshCharacterInfo();
    pushInspectorToSelection();

    QList<BDFRAnimationClip> clips = m_profile.animationClips;
    if (!m_includeAnimation->isChecked())
        m_profile.animationClips.clear();

    QString suggested = m_exportFilename->text().trimmed();
    if (suggested.isEmpty())
        suggested = "Character.bdfbody.json";

    QString path = QFileDialog::getSaveFileName(
        this,
        "Export BDFR Body Profile",
        suggested,
        "BDFR Body Profile (*.bdfbody.json);;JSON (*.json)");
    if (path.isEmpty())
    {
        m_profile.animationClips = clips;
        return;
    }
    if (!path.endsWith(".json", Qt::CaseInsensitive))
        path += ".bdfbody.json";

    QString error;
    const bool ok = m_profile.writeJson(path, &error);
    m_profile.animationClips = clips;

    if (!ok)
        QMessageBox::critical(this, "BDFR Body Authoring", error);
    else
        QMessageBox::information(this, "BDFR Body Authoring",
                                 "Body profile exported successfully:\n" + path);
}

void BDFRBodyAuthoringDialog::onHelp()
{
    QMessageBox::information(this, "BDFR Body Authoring",
        "1. Select/auto-detect gender.\n"
        "2. Click an anatomical region on the active body viewer.\n"
        "3. Tune anchors, radii and physical parameters.\n"
        "4. Optionally load/apply Walk/Run animation clips.\n"
        "5. Export .bdfbody.json for BDFR CharacterBridge.");
}

void BDFRBodyAuthoringDialog::onPresets()
{
    QMessageBox::information(this, "BDFR Body Authoring",
        "Preset profiles are currently generated from the anatomical quick-add map. "
        "Persistent user preset files are scheduled for the next native milestone.");
}

void BDFRBodyAuthoringDialog::onSettings()
{
    QMessageBox::information(this, "BDFR Body Authoring",
        "Current native settings use Daz centimeters and the selected Genesis profile. "
        "DTU embedding and Unreal coordinate conversion are handled by the CharacterBridge transfer layer.");
}
