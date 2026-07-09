// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ElementEditorUI.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>
#include <QtGui/QIcon>

#include "App/UI/LabeledSlider.h"

// Constructs a nested section QGroupBox (with its own compact QGridLayout) inside \a parent
// and appends it to \a sections so setupUi() can stack them without repeating boilerplate.
static QGridLayout *addSection(QGroupBox *&sectionBox, QWidget *parent, QVBoxLayout *sections, const char *objectName)
{
    sectionBox = new QGroupBox(parent);
    sectionBox->setObjectName(objectName);
    auto *grid = new QGridLayout(sectionBox);
    sections->addWidget(sectionBox);
    return grid;
}

void ElementEditorUi::setupUi(QWidget *ElementEditor)
{
    if (ElementEditor->objectName().isEmpty()) {
        ElementEditor->setObjectName("ElementEditor");
    }

    // Design size — actual size is controlled by the parent splitter.
    ElementEditor->resize(304, 395);

    // Zero margins/spacing so the group box fills the widget edge-to-edge,
    // keeping the panel compact inside the narrow left splitter pane.
    gridLayout = new QGridLayout(ElementEditor);
    gridLayout->setSpacing(0);
    gridLayout->setObjectName("gridLayout");
    gridLayout->setContentsMargins(0, 0, 0, 0);

    // The group box title shows the element type name (set dynamically by
    // ElementEditor::setCurrentElements).  Bold font makes it stand out.
    groupBox = new QGroupBox(ElementEditor);
    groupBox->setObjectName("groupBox");
    QFont font;
    font.setBold(true);
    groupBox->setFont(font);

    // Properties are grouped into labeled sections (Identity/Ports/Timing/Sound/
    // Interaction/Appearance) instead of one flat list, so the panel doesn't read
    // as an undifferentiated wall of fields.
    verticalLayoutSections = new QVBoxLayout(groupBox);
    verticalLayoutSections->setObjectName("verticalLayoutSections");

    gridLayoutIdentity           = addSection(groupBoxIdentity,           groupBox, verticalLayoutSections, "groupBoxIdentity");
    gridLayoutPorts              = addSection(groupBoxPorts,              groupBox, verticalLayoutSections, "groupBoxPorts");
    gridLayoutTiming             = addSection(groupBoxTiming,             groupBox, verticalLayoutSections, "groupBoxTiming");
    gridLayoutSound              = addSection(groupBoxSound,              groupBox, verticalLayoutSections, "groupBoxSound");
    gridLayoutInteraction        = addSection(groupBoxInteraction,        groupBox, verticalLayoutSections, "groupBoxInteraction");
    gridLayoutAppearanceSection  = addSection(groupBoxAppearanceSection,  groupBox, verticalLayoutSections, "groupBoxAppearanceSection");

    // --- Identity: Label, Color ---

    labelLabels = new QLabel(groupBoxIdentity);
    labelLabels->setObjectName("labelLabels");
    gridLayoutIdentity->addWidget(labelLabels, 0, 0, 1, 1);

    lineEditElementLabel = new QLineEdit(groupBoxIdentity);
    lineEditElementLabel->setObjectName("lineEditElementLabel");
    gridLayoutIdentity->addWidget(lineEditElementLabel, 0, 1, 1, 1);

    labelColor = new QLabel(groupBoxIdentity);
    labelColor->setObjectName("labelColor");
    gridLayoutIdentity->addWidget(labelColor, 1, 0, 1, 1);

    comboBoxColor = new QComboBox(groupBoxIdentity);
    comboBoxColor->setObjectName("comboBoxColor");
    gridLayoutIdentity->addWidget(comboBoxColor, 1, 1, 1, 1);

    // --- Ports: Inputs, Outputs, Value, Locked ---

    labelInputs = new QLabel(groupBoxPorts);
    labelInputs->setObjectName("labelInputs");
    gridLayoutPorts->addWidget(labelInputs, 0, 0, 1, 1);

    comboBoxInputSize = new QComboBox(groupBoxPorts);
    comboBoxInputSize->setObjectName("comboBoxInputSize");
    gridLayoutPorts->addWidget(comboBoxInputSize, 0, 1, 1, 1);

    labelOutputs = new QLabel(groupBoxPorts);
    labelOutputs->setObjectName("labelOutputs");
    gridLayoutPorts->addWidget(labelOutputs, 1, 0, 1, 1);

    comboBoxOutputSize = new QComboBox(groupBoxPorts);
    comboBoxOutputSize->setObjectName("comboBoxOutputSize");
    gridLayoutPorts->addWidget(comboBoxOutputSize, 1, 1, 1, 1);

    labelValue = new QLabel(groupBoxPorts);
    labelValue->setObjectName("labelValue");
    gridLayoutPorts->addWidget(labelValue, 2, 0, 1, 1);

    comboBoxValue = new QComboBox(groupBoxPorts);
    comboBoxValue->setObjectName("comboBoxValue");
    gridLayoutPorts->addWidget(comboBoxValue, 2, 1, 1, 1);

    labelLocked = new QLabel(groupBoxPorts);
    labelLocked->setObjectName("labelLocked");
    gridLayoutPorts->addWidget(labelLocked, 3, 0, 1, 1);

    checkBoxLocked = new QCheckBox(groupBoxPorts);
    checkBoxLocked->setObjectName("checkBoxLocked");
    gridLayoutPorts->addWidget(checkBoxLocked, 3, 1, 1, 1);

    // --- Timing: Frequency, Delay ---

    labelFrequency = new QLabel(groupBoxTiming);
    labelFrequency->setObjectName("labelFrequency");
    gridLayoutTiming->addWidget(labelFrequency, 0, 0, 1, 1);

    doubleSpinBoxFrequency = new QDoubleSpinBox(groupBoxTiming);
    doubleSpinBoxFrequency->setObjectName("doubleSpinBoxFrequency");
    doubleSpinBoxFrequency->setDecimals(1);
    // Minimum 0.0 here; ElementEditor raises it to 0.1 when a real value is shown
    // (0.0 is reserved as the sentinel for "many frequencies").  50 Hz max matches
    // the simulation's 1 ms tick rate (anything faster would be indistinguishable).
    doubleSpinBoxFrequency->setMinimum(0.000000000000000);
    doubleSpinBoxFrequency->setMaximum(50.000000000000000);
    doubleSpinBoxFrequency->setSingleStep(0.100000000000000);
    gridLayoutTiming->addWidget(doubleSpinBoxFrequency, 0, 1, 1, 1);

    labelDelay = new QLabel(groupBoxTiming);
    labelDelay->setObjectName("labelDelay");
    gridLayoutTiming->addWidget(labelDelay, 1, 0, 1, 1);

    // Delay slider range -4 to +4 in integer steps.  Each step represents 1/8 of
    // a clock period, giving a phase offset range of -0.5 to +0.5 periods.
    // ElementEditor converts between slider integer and float delay on read/write.
    sliderDelay = new LabeledSlider(groupBoxTiming);
    sliderDelay->setObjectName("sliderDelay");
    sliderDelay->setOrientation(Qt::Horizontal);
    sliderDelay->setMinimum(-4);
    sliderDelay->setMaximum(4);
    sliderDelay->setValue(0);
    sliderDelay->setSingleStep(1);
    sliderDelay->setPageStep(1);
    sliderDelay->setTickPosition(QSlider::TicksBelow);
    sliderDelay->setTickInterval(1);
    gridLayoutTiming->addWidget(sliderDelay, 1, 1, 1, 1);

    // --- Sound: Tone (Buzzer), Sound file (AudioBox), Volume ---

    labelAudio = new QLabel(groupBoxSound);
    labelAudio->setObjectName("labelAudio");
    gridLayoutSound->addWidget(labelAudio, 0, 0, 1, 1);

    // Pre-populate with 8 empty items so retranslateUi() can fill them in
    // without needing to know the count at construction time.
    comboBoxAudio = new QComboBox(groupBoxSound);

    for (int i = 0; i < 8; ++i) {
        comboBoxAudio->addItem(QString());
    }

    comboBoxAudio->setObjectName("comboBoxAudio");
    gridLayoutSound->addWidget(comboBoxAudio, 0, 1, 1, 1);

    labelAudioBox = new QLabel(groupBoxSound);
    labelAudioBox->setObjectName("labelAudioBox");
    gridLayoutSound->addWidget(labelAudioBox, 1, 0, 1, 1);

    horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setObjectName("horizontalLayout_2");

    // Read-only line edit shows the currently selected audio file path.
    // Editing is done via the "..." button which opens a file dialog.
    lineCurrentAudioBox = new QLineEdit(groupBoxSound);
    lineCurrentAudioBox->setObjectName("lineCurrentAudioBox");
    lineCurrentAudioBox->setReadOnly(true);
    horizontalLayout_2->addWidget(lineCurrentAudioBox);

    // Narrow "..." button beside the audio path — constrained to 28 px wide so
    // it doesn't steal space from the filename display.
    pushButtonAudioBox = new QPushButton(groupBoxSound);
    pushButtonAudioBox->setObjectName("pushButtonAudioBox");
    QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(pushButtonAudioBox->sizePolicy().hasHeightForWidth());
    pushButtonAudioBox->setSizePolicy(sizePolicy1);
    pushButtonAudioBox->setMaximumSize(QSize(28, 16777215));
    horizontalLayout_2->addWidget(pushButtonAudioBox);
    gridLayoutSound->addLayout(horizontalLayout_2, 1, 1, 1, 1);

    labelVolume = new QLabel(groupBoxSound);
    labelVolume->setObjectName("labelVolume");
    gridLayoutSound->addWidget(labelVolume, 2, 0, 1, 1);

    sliderVolume = new QSlider(groupBoxSound);
    sliderVolume->setObjectName("sliderVolume");
    sliderVolume->setOrientation(Qt::Horizontal);
    sliderVolume->setMinimum(0);
    sliderVolume->setMaximum(100);
    sliderVolume->setValue(50);
    sliderVolume->setSingleStep(5);
    sliderVolume->setPageStep(10);
    sliderVolume->setTickPosition(QSlider::TicksBelow);
    sliderVolume->setTickInterval(25);
    gridLayoutSound->addWidget(sliderVolume, 2, 1, 1, 1);

    // --- Interaction: Trigger, (shared row) Truth Table button / Wireless mode ---

    labelTrigger = new QLabel(groupBoxInteraction);
    labelTrigger->setObjectName("labelTrigger");
    gridLayoutInteraction->addWidget(labelTrigger, 0, 0, 1, 1);

    lineEditTrigger = new QLineEdit(groupBoxInteraction);
    lineEditTrigger->setObjectName("lineEditTrigger");
    gridLayoutInteraction->addWidget(lineEditTrigger, 0, 1, 1, 1);

    // Row 1 is shared between the Truth Table button and Wireless mode — mutually
    // exclusive by element type, toggled via setVisible() in ElementEditor, never
    // both visible at once, so overlapping them here costs no real screen space.
    pushButtonTruthTable = new QPushButton(groupBoxInteraction);
    pushButtonTruthTable->setObjectName("pushButtonTruthTable");
    pushButtonTruthTable->setEnabled(true);
    gridLayoutInteraction->addWidget(pushButtonTruthTable, 1, 0, 1, 2);

    labelWirelessMode = new QLabel(groupBoxInteraction);
    labelWirelessMode->setObjectName("labelWirelessMode");
    gridLayoutInteraction->addWidget(labelWirelessMode, 1, 0, 1, 1);

    comboBoxWirelessMode = new QComboBox(groupBoxInteraction);
    comboBoxWirelessMode->setObjectName("comboBoxWirelessMode");
    for (int i = 0; i < 3; ++i) {
        comboBoxWirelessMode->addItem(QString());
    }
    gridLayoutInteraction->addWidget(comboBoxWirelessMode, 1, 1, 1, 1);

    // --- Appearance: state tile grid, (shared row) Change/Default buttons / IC Name ---

    labelAppearanceState = new QLabel(groupBoxAppearanceSection);
    labelAppearanceState->setObjectName("labelAppearanceState");
    gridLayoutAppearanceSection->addWidget(labelAppearanceState, 0, 0, 1, 1);

    // Container for the per-state icon swatch grid; tiles are (re)built by
    // ElementEditor::rebuildAppearanceStateTiles() whenever the selection changes,
    // since the states/icons depend on the currently selected element.
    widgetAppearanceStates = new QWidget(groupBoxAppearanceSection);
    widgetAppearanceStates->setObjectName("widgetAppearanceStates");
    gridLayoutAppearanceStates = new QGridLayout(widgetAppearanceStates);
    gridLayoutAppearanceStates->setContentsMargins(0, 0, 0, 0);
    gridLayoutAppearanceStates->setSpacing(2);
    buttonGroupAppearanceStates = new QButtonGroup(widgetAppearanceStates);
    gridLayoutAppearanceSection->addWidget(widgetAppearanceStates, 0, 1, 1, 1);

    // Row 1 is shared between the appearance buttons and the IC-name field — mutually
    // exclusive (an element either can change appearance or is an embedded IC).
    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(6);
    horizontalLayout->setObjectName("horizontalLayout");
    horizontalLayout->setSizeConstraint(QLayout::SetNoConstraint);

    pushButtonChangeAppearance = new QPushButton(groupBoxAppearanceSection);
    pushButtonChangeAppearance->setObjectName("pushButtonChangeAppearance");
    horizontalLayout->addWidget(pushButtonChangeAppearance);

    pushButtonDefaultAppearance = new QPushButton(groupBoxAppearanceSection);
    pushButtonDefaultAppearance->setObjectName("pushButtonDefaultAppearance");
    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(pushButtonDefaultAppearance->sizePolicy().hasHeightForWidth());
    pushButtonDefaultAppearance->setSizePolicy(sizePolicy);
    pushButtonDefaultAppearance->setIcon(QIcon(":/Interface/Locale/default.svg"));
    pushButtonDefaultAppearance->setIconSize(QSize(16, 16));
    horizontalLayout->addWidget(pushButtonDefaultAppearance);

    gridLayoutAppearanceSection->addLayout(horizontalLayout, 1, 0, 1, 2);

    labelBlobName = new QLabel(groupBoxAppearanceSection);
    labelBlobName->setObjectName("labelBlobName");
    gridLayoutAppearanceSection->addWidget(labelBlobName, 1, 0, 1, 1);

    lineEditBlobName = new QLineEdit(groupBoxAppearanceSection);
    lineEditBlobName->setObjectName("lineEditBlobName");
    gridLayoutAppearanceSection->addWidget(lineEditBlobName, 1, 1, 1, 1);

    gridLayout->addWidget(groupBox, 0, 0, 1, 1);

    retranslateUi(ElementEditor);

    QMetaObject::connectSlotsByName(ElementEditor);
}

void ElementEditorUi::retranslateUi(QWidget *ElementEditor)
{
    ElementEditor->setWindowTitle(QCoreApplication::translate("ElementEditor", "Form"));
    groupBox->setTitle(QCoreApplication::translate("ElementEditor", "Title"));
    groupBoxIdentity->setTitle(QCoreApplication::translate("ElementEditor", "Identity"));
    groupBoxPorts->setTitle(QCoreApplication::translate("ElementEditor", "Ports"));
    groupBoxTiming->setTitle(QCoreApplication::translate("ElementEditor", "Timing"));
    groupBoxSound->setTitle(QCoreApplication::translate("ElementEditor", "Sound"));
    groupBoxInteraction->setTitle(QCoreApplication::translate("ElementEditor", "Interaction"));
    groupBoxAppearanceSection->setTitle(QCoreApplication::translate("ElementEditor", "Appearance"));
    comboBoxAudio->setItemText(0, QCoreApplication::translate("ElementEditor", "C6"));
    comboBoxAudio->setItemText(1, QCoreApplication::translate("ElementEditor", "D6"));
    comboBoxAudio->setItemText(2, QCoreApplication::translate("ElementEditor", "E6"));
    comboBoxAudio->setItemText(3, QCoreApplication::translate("ElementEditor", "F6"));
    comboBoxAudio->setItemText(4, QCoreApplication::translate("ElementEditor", "G6"));
    comboBoxAudio->setItemText(5, QCoreApplication::translate("ElementEditor", "A7"));
    comboBoxAudio->setItemText(6, QCoreApplication::translate("ElementEditor", "B7"));
    comboBoxAudio->setItemText(7, QCoreApplication::translate("ElementEditor", "C7"));

    checkBoxLocked->setText(QString());
    labelLabels->setText(QCoreApplication::translate("ElementEditor", "Label:"));
    labelTrigger->setText(QCoreApplication::translate("ElementEditor", "Trigger:"));
    pushButtonTruthTable->setText(QCoreApplication::translate("ElementEditor", "Edit Truth Table"));
    labelAppearanceState->setText(QCoreApplication::translate("ElementEditor", "Appearance for:"));
    pushButtonChangeAppearance->setText(QCoreApplication::translate("ElementEditor", "Change appearance to ..."));
    pushButtonDefaultAppearance->setToolTip(QCoreApplication::translate("ElementEditor", "Default"));
    pushButtonDefaultAppearance->setText(QString());
    labelValue->setText(QCoreApplication::translate("ElementEditor", "Value:"));
    doubleSpinBoxFrequency->setSpecialValueText(QString());
    doubleSpinBoxFrequency->setSuffix(QCoreApplication::translate("ElementEditor", " Hz"));
    labelFrequency->setText(QCoreApplication::translate("ElementEditor", "Frequency:"));
    labelColor->setText(QCoreApplication::translate("ElementEditor", "Color:"));
    pushButtonAudioBox->setText(QCoreApplication::translate("ElementEditor", "..."));
    labelInputs->setText(QCoreApplication::translate("ElementEditor", "Inputs:"));
    comboBoxColor->setCurrentText(QString());
    labelAudioBox->setText(QCoreApplication::translate("ElementEditor", "Sound file:"));
    labelOutputs->setText(QCoreApplication::translate("ElementEditor", "Outputs:"));
    labelLocked->setText(QCoreApplication::translate("ElementEditor", "Locked:"));
    labelAudio->setText(QCoreApplication::translate("ElementEditor", "Tone:"));
    labelDelay->setText(QCoreApplication::translate("ElementEditor", "Delay:"));
    labelWirelessMode->setText(QCoreApplication::translate("ElementEditor", "Wireless:"));
    comboBoxWirelessMode->setItemText(0, QCoreApplication::translate("ElementEditor", "None"));
    comboBoxWirelessMode->setItemText(1, QCoreApplication::translate("ElementEditor", "Transmit (Tx)"));
    comboBoxWirelessMode->setItemText(2, QCoreApplication::translate("ElementEditor", "Receive (Rx)"));
    labelBlobName->setText(QCoreApplication::translate("ElementEditor", "IC Name:"));
    labelVolume->setText(QCoreApplication::translate("ElementEditor", "Volume:"));

    // Tooltips for controls whose meaning isn't self-evident from the label alone.
    const QString lockedTip = QCoreApplication::translate(
        "ElementEditor", "Prevent this element from being toggled by clicking it during simulation.");
    labelLocked->setToolTip(lockedTip);
    checkBoxLocked->setToolTip(lockedTip);

    const QString wirelessTip = QCoreApplication::translate(
        "ElementEditor", "Nodes with the same label connect wirelessly — Transmit sends its input, Receive outputs it.");
    labelWirelessMode->setToolTip(wirelessTip);
    comboBoxWirelessMode->setToolTip(wirelessTip);

    const QString valueTip = QCoreApplication::translate("ElementEditor", "The value this input element outputs.");
    labelValue->setToolTip(valueTip);
    comboBoxValue->setToolTip(valueTip);

    const QString triggerTip = QCoreApplication::translate(
        "ElementEditor", "Keyboard key that toggles this element during simulation.");
    labelTrigger->setToolTip(triggerTip);
    lineEditTrigger->setToolTip(triggerTip);

    const QString delayTip = QCoreApplication::translate(
        "ElementEditor", "Phase offset from the driving clock, as a fraction of its period.");
    labelDelay->setToolTip(delayTip);
    sliderDelay->setToolTip(delayTip);
}
