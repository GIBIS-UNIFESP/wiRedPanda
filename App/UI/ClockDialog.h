// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ClockDialog: dialog for selecting a clock wave's period (in time-step columns).
 */

#pragma once

#include <memory>

#include <QDialog>

#include "App/UI/ClockDialogUI.h"

/**
 * \class ClockDialog
 * \brief Modal dialog for selecting the period of a beWavedDolphin clock wave.
 *
 * \details Presents a slider and spin-box linked to the same period value (measured in
 * waveform time-step columns). Run the dialog with exec() and, on QDialog::Accepted,
 * read the chosen value via period().
 */
class ClockDialog : public QDialog
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the dialog with \a currentPeriod (in time-step columns) as the initial value.
    explicit ClockDialog(const int currentPeriod, QWidget *parent = nullptr);
    ~ClockDialog() override;

    // --- Result Access ---

    /// Returns the selected clock period, in waveform time-step columns.
    int period() const;

private:
    Q_DISABLE_COPY(ClockDialog)

    // --- Members ---

    std::unique_ptr<ClockDialogUi> m_ui;
};
