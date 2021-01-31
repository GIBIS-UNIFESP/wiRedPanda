// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clockDialog.h"

#include "ui_clockDialog.h"

clockDialog::clockDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::clockDialog)
{
    m_ui->setupUi(this);
    setWindowTitle("Clock Frequency Selection");
    setWindowFlags(Qt::Window);
    setModal(true);

    connect(m_ui->cancelPushButton, &QPushButton::clicked, this, &clockDialog::cancelRequested);
    connect(m_ui->okPushButton, &QPushButton::clicked, this, &clockDialog::okRequested);
}

int clockDialog::getFrequency()
{
    m_canceled = false;
    exec();
    if (m_canceled) {
        return -1;
    }
    return m_ui->frequencySpinBox->value();
}

clockDialog::~clockDialog()
{
    delete m_ui;
}

void clockDialog::cancelRequested()
{
    m_canceled = true;
    close();
}

void clockDialog::okRequested()
{
    close();
}
