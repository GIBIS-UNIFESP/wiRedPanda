// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clockdialog.h"
#include "ui_clockdialog.h"

ClockDialog::ClockDialog(const int currentFrequency, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::ClockDialog)
{
    m_ui->setupUi(this);

    m_ui->frequencySpinBox->setValue(currentFrequency);

    setWindowTitle(tr("Clock Frequency Selection"));

    connect(m_ui->cancelPushButton, &QPushButton::clicked, this, [this] { reject(); });
    connect(m_ui->okPushButton,     &QPushButton::clicked, this, [this] { accept(); });
}

ClockDialog::~ClockDialog()
{
    delete m_ui;
}

int ClockDialog::frequency()
{
    return (exec() == QDialog::Accepted) ? m_ui->frequencySpinBox->value() : -1;
}

// TODO: order of buttons is wrong on some operating systems
