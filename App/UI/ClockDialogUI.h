// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Specific Qt includes instead of monolithic QtWidgets
#include <QDialog>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QSlider>
#include <QLabel>

class ClockDialogUi
{
public:
    // Default constructor (required for instantiation)
    ClockDialogUi() = default;

    // Delete copy constructor and assignment operator
    ClockDialogUi(const ClockDialogUi&) = delete;
    ClockDialogUi& operator=(const ClockDialogUi&) = delete;

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

