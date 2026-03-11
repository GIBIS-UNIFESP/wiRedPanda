// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Specific Qt includes instead of monolithic QtWidgets
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>

class ClockDialog_Ui
{
public:
    // Default constructor (required for instantiation)
    ClockDialog_Ui() = default;

    // Delete copy constructor and assignment operator
    ClockDialog_Ui(const ClockDialog_Ui&) = delete;
    ClockDialog_Ui& operator=(const ClockDialog_Ui&) = delete;

    void setupUi(QDialog *ClockDialog);
    void retranslateUi(QDialog *ClockDialog);

    QGridLayout *gridLayout_2 = nullptr;
    QDialogButtonBox *buttonBox = nullptr;
    QSpinBox *frequencySpinBox = nullptr;
    QSlider *frequencySlider = nullptr;
    QLabel *titleLabel = nullptr;
    QLabel *maxLabel = nullptr;
    QLabel *minLabel = nullptr;
};
