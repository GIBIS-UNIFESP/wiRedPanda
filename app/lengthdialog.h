/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LENGTHDIALOG_H
#define LENGTHDIALOG_H

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

#endif /* LENGTHDIALOG_H */
