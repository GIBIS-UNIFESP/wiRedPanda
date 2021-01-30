#include "clockDialog.h"

clockDialog::clockDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::clockDialog)
{
    ui->setupUi(this);
    setWindowTitle("Clock Frequency Selection");
    setWindowFlags(Qt::Window);
    setModal(true);
}

int clockDialog::getFrequency()
{
    canceled = false;
    exec();
    if (canceled) {
        return (-1);
    }
    return (ui->frequencySpinBox->value());
}

clockDialog::~clockDialog()
{
    delete ui;
}

void clockDialog::on_cancelPushButton_clicked()
{
    canceled = true;
    close();
}

void clockDialog::on_okPushButton_clicked()
{
    close();
}
