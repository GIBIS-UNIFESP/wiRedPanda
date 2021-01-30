#ifndef CLOCKDIALOG_H
#define CLOCKDIALOG_H

#include "ui_clockDialog.h"
#include <QDialog>

namespace Ui
{
class clockDialog;
}

class clockDialog : public QDialog
{
    Q_OBJECT

public:
    explicit clockDialog(QWidget *parent = nullptr);
    int getFrequency();
    ~clockDialog();

private slots:
    void on_cancelPushButton_clicked();

    void on_okPushButton_clicked();

private:
    Ui::clockDialog *ui;
    bool canceled;
};

#endif /* CLOCKDIALOG_H */
