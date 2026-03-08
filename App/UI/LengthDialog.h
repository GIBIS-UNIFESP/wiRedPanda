// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief LengthDialog: dialog for setting the BeWavedDolphin simulation length.
 */

#pragma once

#include <memory>

#include <QDialog>

#include "App/UI/LengthDialogUI.h"

/**
 * \class LengthDialog
 * \brief Modal dialog for setting the BeWavedDolphin simulation length (number of time steps).
 *
 * \details Provides a slider and spin-box for entering the desired column count.
 */
class LengthDialog : public QDialog
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /** \brief Constructs the dialog.
     * \param currentLength The initial simulation length value.
     * \param parent Optional parent widget.
     */
    explicit LengthDialog(const int currentLength, QWidget *parent = nullptr);
    ~LengthDialog() override;

    // --- Result Access ---

    /// Returns the simulation length value selected by the user.
    int length();

private:
    Q_DISABLE_COPY(LengthDialog)

    // --- Members ---

    std::unique_ptr<LengthDialogUi> m_ui;
};

