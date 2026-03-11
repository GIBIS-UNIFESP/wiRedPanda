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

class LengthDialogUi
{
public:
    // Default constructor (required for instantiation)
    LengthDialogUi() = default;

    // Delete copy constructor and assignment operator
    LengthDialogUi(const LengthDialogUi&) = delete;
    LengthDialogUi& operator=(const LengthDialogUi&) = delete;

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

