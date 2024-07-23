// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lengthdialog.h"
#include "ui_lengthdialog.h"

LengthDialog::LengthDialog(const int currentLength, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::LengthDialog)
{
    m_ui->setupUi(this);

    m_ui->lengthSpinBox->setValue(currentLength);

    setWindowTitle(tr("Simulation Length Selection"));

    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

LengthDialog::~LengthDialog()
{
    delete m_ui;
}

int LengthDialog::length()
{
    return (exec() == QDialog::Accepted) ? m_ui->lengthSpinBox->value() : -1;
}
