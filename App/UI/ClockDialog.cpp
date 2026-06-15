// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ClockDialog.h"

#include "App/UI/ClockDialogUI.h"

ClockDialog::ClockDialog(const int currentPeriod, QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<ClockDialogUi>())
{
    m_ui->setupUi(this);

    // Pre-populate with the last used period so the user doesn't have to re-enter it
    m_ui->periodSpinBox->setValue(currentPeriod);

    setWindowTitle(tr("Clock Period Selection"));

    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

ClockDialog::~ClockDialog()
{
}

int ClockDialog::period() const
{
    // Pure accessor: the caller runs exec() and reads this only on QDialog::Accepted.
    return m_ui->periodSpinBox->value();
}
