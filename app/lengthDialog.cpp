// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lengthDialog.h"

lengthDialog::lengthDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::lengthDialog)
{
    ui->setupUi(this);
    setWindowTitle("Simulation length Selection");
    setWindowFlags(Qt::Window);
    setModal(true);
}

int lengthDialog::getFrequency()
{
    canceled = false;
    exec();
    if (canceled) {
        return (-1);
    }
    return (ui->lengthSpinBox->value());
}

lengthDialog::~lengthDialog()
{
    delete ui;
}

void lengthDialog::on_cancelPushButton_clicked()
{
    canceled = true;
    close();
}

void lengthDialog::on_okPushButton_clicked()
{
    close();
}
