// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief LengthDialogUi: hand-written UI class for the LengthDialog.
 */

#pragma once

// Specific Qt includes instead of monolithic QtWidgets
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>

class LengthDialog_Ui
{
public:
    // --- Lifecycle ---

    LengthDialog_Ui() = default;

    LengthDialog_Ui(const LengthDialog_Ui&) = delete;
    LengthDialog_Ui& operator=(const LengthDialog_Ui&) = delete;

    // --- Setup ---

    /**
     * \brief Creates and lays out all child widgets inside \a LengthDialog.
     * \param LengthDialog The host QDialog.
     */
    void setupUi(QDialog *LengthDialog);

    /**
     * \brief Updates all translatable strings in the dialog.
     * \param LengthDialog The host QDialog.
     */
    void retranslateUi(QDialog *LengthDialog);

    // --- Widgets ---

    // Layout
    QGridLayout *gridLayout_2 = nullptr;

    // Controls
    QSlider *lengthSlider = nullptr;
    QSpinBox *lengthSpinBox = nullptr;

    // Labels
    QLabel *titleLabel = nullptr;
    QLabel *minLabel = nullptr;
    QLabel *maxLabel = nullptr;

    // Buttons
    QDialogButtonBox *buttonBox = nullptr;
};
