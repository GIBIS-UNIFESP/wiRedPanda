// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ClockDialog.h"

#include "App/UI/ClockDialogUI.h"

ClockDialog::ClockDialog(const int currentFrequency, QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<ClockDialog_Ui>())
{
    m_ui->setupUi(this);

    // Pre-populate with the last used frequency so the user doesn't have to re-enter it
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
    // Returns -1 on cancel so callers can distinguish "user chose a frequency" from
    // "user dismissed the dialog" without needing a separate success/failure flag
    return (exec() == QDialog::Accepted) ? m_ui->frequencySpinBox->value() : -1;
}
