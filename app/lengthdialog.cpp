// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lengthdialog.h"
#include "ui_lengthdialog.h"

LengthDialog::LengthDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::LengthDialog)
{
    m_ui->setupUi(this);
    setWindowTitle("Simulation length Selection");
    setWindowFlags(Qt::Window);
    setModal(true);
    connect(m_ui->cancelPushButton, &QPushButton::clicked, this, &LengthDialog::cancelClicked);
    connect(m_ui->okPushButton, &QPushButton::clicked, this, &LengthDialog::okClicked);
}

int LengthDialog::getFrequency()
{
    m_canceled = false;
    exec();
    if (m_canceled) {
        return -1;
    }
    return m_ui->lengthSpinBox->value();
}

LengthDialog::~LengthDialog()
{
    delete m_ui;
}

void LengthDialog::cancelClicked()
{
    m_canceled = true;
    close();
}

void LengthDialog::okClicked()
{
    close();
}
