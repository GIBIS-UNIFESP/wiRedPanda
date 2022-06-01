// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lengthdialog.h"
#include "ui_lengthdialog.h"

LengthDialog::LengthDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::LengthDialog)
{
    m_ui->setupUi(this);

    setWindowTitle(tr("Simulation Length Selection"));

    connect(m_ui->cancelPushButton, &QPushButton::clicked, this, [this] { reject(); });
    connect(m_ui->okPushButton,     &QPushButton::clicked, this, [this] { accept(); });
}

LengthDialog::~LengthDialog()
{
    delete m_ui;
}

int LengthDialog::length()
{
    return (exec() == QDialog::Accepted) ? m_ui->lengthSpinBox->value() : -1;
}

// TODO: order of buttons is wrong on some operating systems
