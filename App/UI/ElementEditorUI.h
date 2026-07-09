// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ElementEditorUi: hand-written UI class for the ElementEditor widget.
 */

#pragma once

// Specific Qt includes instead of monolithic QtWidgets
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

class LabeledSlider;

class ElementEditorUi
{
public:
    // --- Lifecycle ---

    ElementEditorUi() = default;

    ElementEditorUi(const ElementEditorUi&) = delete;
    ElementEditorUi& operator=(const ElementEditorUi&) = delete;

    // --- Setup ---

    /**
     * \brief Creates and lays out all child widgets inside \a ElementEditor.
     * \param ElementEditor The host QWidget.
     */
    void setupUi(QWidget *ElementEditor);

    /**
     * \brief Updates all translatable strings in the widget.
     * \param ElementEditor The host QWidget.
     */
    void retranslateUi(QWidget *ElementEditor);

    // --- Widgets ---

    // Layout
    QGridLayout *gridLayout = nullptr;
    QGroupBox *groupBox = nullptr;
    QVBoxLayout *verticalLayoutSections = nullptr;
    QHBoxLayout *horizontalLayout = nullptr;
    QHBoxLayout *horizontalLayout_2 = nullptr;

    // Sections — group related properties so the panel isn't one flat 15-row list.
    QGroupBox *groupBoxIdentity = nullptr;
    QGridLayout *gridLayoutIdentity = nullptr;
    QGroupBox *groupBoxPorts = nullptr;
    QGridLayout *gridLayoutPorts = nullptr;
    QGroupBox *groupBoxTiming = nullptr;
    QGridLayout *gridLayoutTiming = nullptr;
    QGroupBox *groupBoxSound = nullptr;
    QGridLayout *gridLayoutSound = nullptr;
    QGroupBox *groupBoxInteraction = nullptr;
    QGridLayout *gridLayoutInteraction = nullptr;
    QGroupBox *groupBoxAppearanceSection = nullptr;
    QGridLayout *gridLayoutAppearanceSection = nullptr;

    // Label editing
    QLabel *labelLabels = nullptr;
    QLineEdit *lineEditElementLabel = nullptr;

    // Color selection
    QLabel *labelColor = nullptr;
    QComboBox *comboBoxColor = nullptr;

    // Input / output size
    QLabel *labelInputs = nullptr;
    QComboBox *comboBoxInputSize = nullptr;
    QLabel *labelOutputs = nullptr;
    QComboBox *comboBoxOutputSize = nullptr;

    // Output value
    QLabel *labelValue = nullptr;
    QComboBox *comboBoxValue = nullptr;

    // Frequency
    QLabel *labelFrequency = nullptr;
    QDoubleSpinBox *doubleSpinBoxFrequency = nullptr;

    // Delay
    QLabel *labelDelay = nullptr;
    LabeledSlider *sliderDelay = nullptr;

    // Trigger
    QLabel *labelTrigger = nullptr;
    QLineEdit *lineEditTrigger = nullptr;

    // Appearance
    QLabel *labelAppearanceState = nullptr;
    QWidget *widgetAppearanceStates = nullptr;
    QGridLayout *gridLayoutAppearanceStates = nullptr;
    QButtonGroup *buttonGroupAppearanceStates = nullptr;
    QPushButton *pushButtonChangeAppearance = nullptr;
    QPushButton *pushButtonDefaultAppearance = nullptr;

    // Truth table
    QPushButton *pushButtonTruthTable = nullptr;

    // Volume
    QLabel *labelVolume = nullptr;
    QSlider *sliderVolume = nullptr;

    // Audio
    QLabel *labelAudioBox = nullptr;
    QLabel *labelAudio = nullptr;
    QComboBox *comboBoxAudio = nullptr;
    QLineEdit *lineCurrentAudioBox = nullptr;
    QPushButton *pushButtonAudioBox = nullptr;

    // Lock
    QLabel *labelLocked = nullptr;
    QCheckBox *checkBoxLocked = nullptr;

    // Wireless mode (Node only)
    QLabel *labelWirelessMode = nullptr;
    QComboBox *comboBoxWirelessMode = nullptr;

    // Blob name (embedded ICs only)
    QLabel *labelBlobName = nullptr;
    QLineEdit *lineEditBlobName = nullptr;
};
