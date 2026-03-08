// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ClockDialogUi: hand-written UI class for the ClockDialog.
 */

#pragma once

// Specific Qt includes instead of monolithic QtWidgets
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>

class ClockDialogUi
{
public:
    // --- Lifecycle ---

    ClockDialogUi() = default;

    ClockDialogUi(const ClockDialogUi&) = delete;
    ClockDialogUi& operator=(const ClockDialogUi&) = delete;

    // --- Setup ---

    /**
     * \brief Creates and lays out all child widgets inside \a ClockDialog.
     * \param ClockDialog The host QDialog.
     */
    void setupUi(QDialog *ClockDialog);

    /**
     * \brief Updates all translatable strings in the dialog.
     * \param ClockDialog The host QDialog.
     */
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

