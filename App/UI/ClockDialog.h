// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QDialog>

#include "App/UI/ClockDialogUI.h"

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

    std::unique_ptr<ClockDialog_Ui> m_ui;
};
