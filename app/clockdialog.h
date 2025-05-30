// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>

namespace Ui
{
class ClockDialog;
}

//!
//! \brief The clockDialog class handles dialogs for setting the frequency at which a clock ticks
//!
class ClockDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClockDialog(const int currentFrequency, QWidget *parent = nullptr);
    ~ClockDialog() override;

    //! Returns the clock frequency in Hz
    int frequency();

private:
    Q_DISABLE_COPY(ClockDialog)

    Ui::ClockDialog *m_ui;
};
