/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

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
    explicit ClockDialog(QWidget *parent = nullptr);
    ~ClockDialog() override;

    //! Returns the clock frequency (in Hz)
    int getFrequency();

private:
    void cancelRequested();
    void okRequested();

    Ui::ClockDialog *m_ui;
    bool m_canceled;
};

