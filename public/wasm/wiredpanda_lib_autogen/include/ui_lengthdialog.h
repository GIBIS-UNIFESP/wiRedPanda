/********************************************************************************
** Form generated from reading UI file 'lengthdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LENGTHDIALOG_H
#define UI_LENGTHDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_LengthDialog
{
public:
    QGridLayout *gridLayout_2;
    QSlider *lengthSlider;
    QLabel *minLabel;
    QLabel *titleLabel;
    QLabel *maxLabel;
    QSpinBox *lengthSpinBox;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *LengthDialog)
    {
        if (LengthDialog->objectName().isEmpty())
            LengthDialog->setObjectName("LengthDialog");
        LengthDialog->resize(184, 116);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/toolbar/wavyIcon.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        LengthDialog->setWindowIcon(icon);
        gridLayout_2 = new QGridLayout(LengthDialog);
        gridLayout_2->setObjectName("gridLayout_2");
        lengthSlider = new QSlider(LengthDialog);
        lengthSlider->setObjectName("lengthSlider");
        lengthSlider->setMinimum(2);
        lengthSlider->setMaximum(2048);
        lengthSlider->setSingleStep(1);
        lengthSlider->setValue(64);
        lengthSlider->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(lengthSlider, 3, 0, 1, 2);

        minLabel = new QLabel(LengthDialog);
        minLabel->setObjectName("minLabel");
        minLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        gridLayout_2->addWidget(minLabel, 4, 0, 1, 1);

        titleLabel = new QLabel(LengthDialog);
        titleLabel->setObjectName("titleLabel");

        gridLayout_2->addWidget(titleLabel, 0, 0, 1, 1);

        maxLabel = new QLabel(LengthDialog);
        maxLabel->setObjectName("maxLabel");
        maxLabel->setAlignment(Qt::AlignRight|Qt::AlignTop|Qt::AlignTrailing);

        gridLayout_2->addWidget(maxLabel, 4, 1, 1, 1);

        lengthSpinBox = new QSpinBox(LengthDialog);
        lengthSpinBox->setObjectName("lengthSpinBox");
        lengthSpinBox->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        lengthSpinBox->setMinimum(2);
        lengthSpinBox->setMaximum(2048);
        lengthSpinBox->setSingleStep(1);
        lengthSpinBox->setValue(64);

        gridLayout_2->addWidget(lengthSpinBox, 0, 1, 1, 1);

        buttonBox = new QDialogButtonBox(LengthDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(buttonBox, 5, 0, 1, 2);


        retranslateUi(LengthDialog);
        QObject::connect(lengthSlider, &QSlider::valueChanged, lengthSpinBox, &QSpinBox::setValue);
        QObject::connect(lengthSpinBox, &QSpinBox::valueChanged, lengthSlider, &QSlider::setValue);

        QMetaObject::connectSlotsByName(LengthDialog);
    } // setupUi

    void retranslateUi(QDialog *LengthDialog)
    {
        LengthDialog->setWindowTitle(QCoreApplication::translate("LengthDialog", "Dialog", nullptr));
        minLabel->setText(QCoreApplication::translate("LengthDialog", "2", nullptr));
        titleLabel->setText(QCoreApplication::translate("LengthDialog", "Simulation Length", nullptr));
        maxLabel->setText(QCoreApplication::translate("LengthDialog", "2048", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LengthDialog: public Ui_LengthDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LENGTHDIALOG_H
