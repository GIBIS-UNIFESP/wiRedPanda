// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtWidgets/QtWidgets>

class LengthDialog_Ui
{
public:
    // Default constructor (required for instantiation)
    LengthDialog_Ui() = default;

    // Delete copy constructor and assignment operator
    LengthDialog_Ui(const LengthDialog_Ui&) = delete;
    LengthDialog_Ui& operator=(const LengthDialog_Ui&) = delete;

    void setupUi(QDialog *LengthDialog);
    void retranslateUi(QDialog *LengthDialog);

    QGridLayout *gridLayout_2 = nullptr;
    QSlider *lengthSlider = nullptr;
    QLabel *minLabel = nullptr;
    QLabel *titleLabel = nullptr;
    QLabel *maxLabel = nullptr;
    QSpinBox *lengthSpinBox = nullptr;
    QDialogButtonBox *buttonBox = nullptr;
};

