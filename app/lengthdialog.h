// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDialog>

namespace Ui
{
class LengthDialog;
}

/*!
 * @class LengthDialog
 * @brief Dialog for setting a numeric length value
 *
 * The LengthDialog class provides a simple dialog with a numeric input
 * for setting length values. It is used for configuring
 * simulation length inside beWavedDolphin.
 */
class LengthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LengthDialog(const int currentLength, QWidget *parent = nullptr);
    ~LengthDialog() override;

    int length();

private:
    Q_DISABLE_COPY(LengthDialog)

    Ui::LengthDialog *m_ui;
};
