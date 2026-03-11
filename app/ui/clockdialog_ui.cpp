// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clockdialog_ui.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>

void ClockDialog_Ui::setupUi(QDialog *ClockDialog)
{
    if (ClockDialog->objectName().isEmpty()) {
        ClockDialog->setObjectName("ClockDialog");
    }

    ClockDialog->resize(184, 116);

    ClockDialog->setWindowIcon(QIcon(":/interface/toolbar/wavyIcon.svg"));

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
    QObject::connect(frequencySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), frequencySlider, &QSlider::setValue);

    QMetaObject::connectSlotsByName(ClockDialog);
}

void ClockDialog_Ui::retranslateUi(QDialog *ClockDialog)
{
    ClockDialog->setWindowTitle(QCoreApplication::translate("ClockDialog", "Dialog"));
    titleLabel->setText(QCoreApplication::translate("ClockDialog", "Clock Frequency"));
    maxLabel->setText(QCoreApplication::translate("ClockDialog", "1024"));
    minLabel->setText(QCoreApplication::translate("ClockDialog", "2"));
}
