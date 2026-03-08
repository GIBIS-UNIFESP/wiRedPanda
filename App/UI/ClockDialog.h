// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ClockDialog: dialog for configuring a clock element's frequency.
 */

#pragma once

#include <memory>

#include <QDialog>

#include "App/UI/ClockDialogUI.h"

/**
 * \class ClockDialog
 * \brief Modal dialog for selecting the tick frequency of a Clock element.
 *
 * \details Presents a slider and spin-box linked to the same frequency value.
 * The user accepts or cancels; the chosen frequency is retrieved via frequency().
 */
class ClockDialog : public QDialog
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the dialog with \a currentFrequency as the initial value.
    explicit ClockDialog(const int currentFrequency, QWidget *parent = nullptr);
    ~ClockDialog() override;

    // --- Result Access ---

    //! Returns the clock frequency in Hz
    int frequency();

private:
    Q_DISABLE_COPY(ClockDialog)

    // --- Members ---

    std::unique_ptr<ClockDialog_Ui> m_ui;
};
