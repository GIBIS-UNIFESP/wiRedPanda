// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Specific Qt includes instead of monolithic QtWidgets
#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>

class ElementEditor_Ui
{
public:
    // Default constructor (required for instantiation)
    ElementEditor_Ui() = default;

    // Delete copy constructor and assignment operator
    ElementEditor_Ui(const ElementEditor_Ui&) = delete;
    ElementEditor_Ui& operator=(const ElementEditor_Ui&) = delete;

    void setupUi(QWidget *ElementEditor);
    void retranslateUi(QWidget *ElementEditor);

    QGridLayout *gridLayout = nullptr;
    QGroupBox *groupBox = nullptr;
    QGridLayout *gridLayout_2 = nullptr;
    QComboBox *comboBoxAudio = nullptr;
    QCheckBox *checkBoxLocked = nullptr;
    QLabel *labelLabels = nullptr;
    QLabel *labelTrigger = nullptr;
    QHBoxLayout *horizontalLayout = nullptr;
    QPushButton *pushButtonTruthTable = nullptr;
    QPushButton *pushButtonChangeSkin = nullptr;
    QPushButton *pushButtonDefaultSkin = nullptr;
    QLineEdit *lineEditTrigger = nullptr;
    QLabel *labelValue = nullptr;
    QDoubleSpinBox *doubleSpinBoxFrequency = nullptr;
    QComboBox *comboBoxInputSize = nullptr;
    QLabel *labelFrequency = nullptr;
    QLabel *labelColor = nullptr;
    QHBoxLayout *horizontalLayout_2 = nullptr;
    QLineEdit *lineCurrentAudioBox = nullptr;
    QPushButton *pushButtonAudioBox = nullptr;
    QLineEdit *lineEditElementLabel = nullptr;
    QLabel *labelInputs = nullptr;
    QComboBox *comboBoxColor = nullptr;
    QLabel *labelAudioBox = nullptr;
    QLabel *labelPriority = nullptr;
    QComboBox *comboBoxValue = nullptr;
    QComboBox *comboBoxOutputSize = nullptr;
    QSpinBox *spinBoxPriority = nullptr;
    QLabel *labelOutputs = nullptr;
    QLabel *labelLocked = nullptr;
    QLabel *labelAudio = nullptr;
    QLabel *labelDelay = nullptr;
    QDoubleSpinBox *doubleSpinBoxDelay = nullptr;
};
