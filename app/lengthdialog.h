/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QDialog>

namespace Ui
{
class LengthDialog;
}

class LengthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LengthDialog(QWidget *parent = nullptr);
    int getFrequency();
    ~LengthDialog() override;

private slots:
    void cancelClicked();
    void okClicked();

private:
    Ui::LengthDialog *m_ui;
    bool m_canceled;
};

