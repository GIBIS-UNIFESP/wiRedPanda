/********************************************************************************
** Form generated from reading UI file 'clockdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLOCKDIALOG_H
#define UI_CLOCKDIALOG_H

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

class Ui_ClockDialog
{
public:
    QGridLayout *gridLayout_2;
    QDialogButtonBox *buttonBox;
    QSpinBox *frequencySpinBox;
    QSlider *frequencySlider;
    QLabel *titleLabel;
    QLabel *maxLabel;
    QLabel *minLabel;

    void setupUi(QDialog *ClockDialog)
    {
        if (ClockDialog->objectName().isEmpty())
            ClockDialog->setObjectName("ClockDialog");
        ClockDialog->resize(184, 116);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/toolbar/wavyIcon.svg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        ClockDialog->setWindowIcon(icon);
        gridLayout_2 = new QGridLayout(ClockDialog);
        gridLayout_2->setObjectName("gridLayout_2");
        buttonBox = new QDialogButtonBox(ClockDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(buttonBox, 6, 0, 1, 2);

        frequencySpinBox = new QSpinBox(ClockDialog);
        frequencySpinBox->setObjectName("frequencySpinBox");
        frequencySpinBox->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        frequencySpinBox->setMinimum(2);
        frequencySpinBox->setMaximum(1024);
        frequencySpinBox->setSingleStep(2);

        gridLayout_2->addWidget(frequencySpinBox, 0, 1, 1, 1);

        frequencySlider = new QSlider(ClockDialog);
        frequencySlider->setObjectName("frequencySlider");
        frequencySlider->setMinimum(2);
        frequencySlider->setMaximum(1024);
        frequencySlider->setSingleStep(2);
        frequencySlider->setOrientation(Qt::Horizontal);

        gridLayout_2->addWidget(frequencySlider, 3, 0, 1, 2);

        titleLabel = new QLabel(ClockDialog);
        titleLabel->setObjectName("titleLabel");

        gridLayout_2->addWidget(titleLabel, 0, 0, 1, 1);

        maxLabel = new QLabel(ClockDialog);
        maxLabel->setObjectName("maxLabel");
        maxLabel->setAlignment(Qt::AlignRight|Qt::AlignTop|Qt::AlignTrailing);

        gridLayout_2->addWidget(maxLabel, 4, 1, 1, 1);

        minLabel = new QLabel(ClockDialog);
        minLabel->setObjectName("minLabel");
        minLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        gridLayout_2->addWidget(minLabel, 4, 0, 1, 1);


        retranslateUi(ClockDialog);
        QObject::connect(frequencySlider, &QSlider::valueChanged, frequencySpinBox, &QSpinBox::setValue);
        QObject::connect(frequencySpinBox, &QSpinBox::valueChanged, frequencySlider, &QSlider::setValue);

        QMetaObject::connectSlotsByName(ClockDialog);
    } // setupUi

    void retranslateUi(QDialog *ClockDialog)
    {
        ClockDialog->setWindowTitle(QCoreApplication::translate("ClockDialog", "Dialog", nullptr));
        titleLabel->setText(QCoreApplication::translate("ClockDialog", "Clock Frequency", nullptr));
        maxLabel->setText(QCoreApplication::translate("ClockDialog", "1024", nullptr));
        minLabel->setText(QCoreApplication::translate("ClockDialog", "2", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ClockDialog: public Ui_ClockDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLOCKDIALOG_H
