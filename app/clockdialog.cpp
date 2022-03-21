// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clockdialog.h"
#include "ui_clockdialog.h"

ClockDialog::ClockDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::ClockDialog)
{
    m_ui->setupUi(this);
    setWindowTitle("Clock Frequency Selection");
    setWindowFlags(Qt::Window);
    setModal(true);

    connect(m_ui->cancelPushButton, &QPushButton::clicked, this, &ClockDialog::cancelRequested);
    connect(m_ui->okPushButton, &QPushButton::clicked, this, &ClockDialog::okRequested);
}

int ClockDialog::getFrequency()
{
    m_canceled = false;
    exec();
    if (m_canceled) {
        return -1;
    }
    return m_ui->frequencySpinBox->value();
}

ClockDialog::~ClockDialog()
{
    delete m_ui;
}

void ClockDialog::cancelRequested()
{
    m_canceled = true;
    close();
}

void ClockDialog::okRequested()
{
    close();
}
