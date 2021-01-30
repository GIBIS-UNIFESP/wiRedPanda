/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LENGTHDIALOG_H
#define LENGTHDIALOG_H

#include <QDialog>

#include "ui_lengthDialog.h"

namespace Ui
{
class lengthDialog;
}

class lengthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit lengthDialog(QWidget *parent = nullptr);
    int getFrequency();
    ~lengthDialog() override;

private slots:
    void on_cancelPushButton_clicked();

    void on_okPushButton_clicked();

private:
    Ui::lengthDialog *ui;
    bool canceled;
};

#endif /* LENGTHDIALOG_H */
