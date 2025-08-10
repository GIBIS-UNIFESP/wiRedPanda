// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clockdialog.h"
#include "clockdialog_ui.h"

ClockDialog::ClockDialog(const int currentFrequency, QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<ClockDialog_Ui>())
{
    m_ui->setupUi(this);

    m_ui->frequencySpinBox->setValue(currentFrequency);

    setWindowTitle(tr("Clock Frequency Selection"));

    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ClockDialog::~ClockDialog()
{
}

int ClockDialog::frequency()
{
    return (exec() == QDialog::Accepted) ? m_ui->frequencySpinBox->value() : -1;
}
