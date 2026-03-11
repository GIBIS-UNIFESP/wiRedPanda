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
    // --- Lifecycle ---

    ClockDialog_Ui() = default;

    ClockDialog_Ui(const ClockDialog_Ui&) = delete;
    ClockDialog_Ui& operator=(const ClockDialog_Ui&) = delete;

    // --- Setup ---

    void setupUi(QDialog *ClockDialog);
    void retranslateUi(QDialog *ClockDialog);

    // --- Widgets ---

    // Layout
    QGridLayout *gridLayout_2 = nullptr;

    // Controls
    QSpinBox *frequencySpinBox = nullptr;
    QSlider *frequencySlider = nullptr;

    // Labels
    QLabel *titleLabel = nullptr;
    QLabel *maxLabel = nullptr;
    QLabel *minLabel = nullptr;

    // Buttons
    QDialogButtonBox *buttonBox = nullptr;
};
