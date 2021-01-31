/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CLOCKDIALOG_H
#define CLOCKDIALOG_H

#include <QDialog>

namespace Ui
{
class clockDialog;
}

//!
//! \brief The clockDialog class handles dialogs for setting the frequency at which a clock ticks
//!
class clockDialog : public QDialog
{
    Q_OBJECT

public:
    explicit clockDialog(QWidget *parent = nullptr);
    //! Returns the clock frequency (in Hz)
    int getFrequency();
    ~clockDialog() override;

private slots:
    void cancelRequested();
    void okRequested();

private:
    Ui::clockDialog * m_ui;
    bool m_canceled;
};

#endif /* CLOCKDIALOG_H */
