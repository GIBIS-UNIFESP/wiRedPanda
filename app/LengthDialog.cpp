// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "LengthDialog.h"

lengthDialog::lengthDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::lengthDialog)
{
    m_ui->setupUi(this);
    setWindowTitle("Simulation length Selection");
    setWindowFlags(Qt::Window);
    setModal(true);
    connect(m_ui->cancelPushButton, &QPushButton::clicked, this, &lengthDialog::cancelClicked);
    connect(m_ui->okPushButton, &QPushButton::clicked, this, &lengthDialog::okClicked);
}

int lengthDialog::getFrequency()
{
    m_canceled = false;
    exec();
    if (m_canceled) {
        return -1;
    }
    return m_ui->lengthSpinBox->value();
}

lengthDialog::~lengthDialog()
{
    delete m_ui;
}

void lengthDialog::cancelClicked()
{
    m_canceled = true;
    close();
}

void lengthDialog::okClicked()
{
    close();
}
