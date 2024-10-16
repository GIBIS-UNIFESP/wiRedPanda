
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



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

    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ClockDialog::~ClockDialog()
{
    delete m_ui;
}

int ClockDialog::frequency()
{
    return (exec() == QDialog::Accepted) ? m_ui->frequencySpinBox->value() : -1;
}
