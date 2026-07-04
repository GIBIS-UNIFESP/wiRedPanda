// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/LengthDialog.h"

#include "App/Core/I18n.h"
#include "App/UI/LengthDialogUI.h"

LengthDialog::LengthDialog(const int currentLength, QWidget *parent)
    : QDialog(parent)
    , m_ui(std::make_unique<LengthDialogUi>())
{
    m_ui->setupUi(this);

    // Pre-populate with the existing simulation length so the user sees the current value
    m_ui->lengthSpinBox->setValue(currentLength);

    setWindowTitle(i18n("Simulation Length Selection"));

    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

LengthDialog::~LengthDialog()
{
}

int LengthDialog::length() const
{
    // Pure accessor: the caller runs exec() and reads this only on QDialog::Accepted.
    return m_ui->lengthSpinBox->value();
}
