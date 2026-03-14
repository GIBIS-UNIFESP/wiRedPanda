// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ElementEditorUi: hand-written UI class for the ElementEditor widget.
 */

#pragma once

// Specific Qt includes instead of monolithic QtWidgets
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
#include <QWidget>

class LabeledSlider;

class ElementEditor_Ui
{
public:
    // --- Lifecycle ---

    ElementEditor_Ui() = default;

    ElementEditor_Ui(const ElementEditor_Ui&) = delete;
    ElementEditor_Ui& operator=(const ElementEditor_Ui&) = delete;

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
    QGridLayout *gridLayout_2 = nullptr;
    QHBoxLayout *horizontalLayout = nullptr;
    QHBoxLayout *horizontalLayout_2 = nullptr;

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

    // Skin
    QPushButton *pushButtonChangeSkin = nullptr;
    QPushButton *pushButtonDefaultSkin = nullptr;

    // Truth table
    QPushButton *pushButtonTruthTable = nullptr;

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
};
