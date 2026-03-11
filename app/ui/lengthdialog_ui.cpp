// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lengthdialog_ui.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>

void LengthDialog_Ui::setupUi(QDialog *LengthDialog)
{
    if (LengthDialog->objectName().isEmpty()) {
        LengthDialog->setObjectName("LengthDialog");
    }

    LengthDialog->resize(184, 116);

    LengthDialog->setWindowIcon(QIcon(":/interface/toolbar/wavyIcon.svg"));

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
    QObject::connect(lengthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), lengthSlider, &QSlider::setValue);

    QMetaObject::connectSlotsByName(LengthDialog);
}

void LengthDialog_Ui::retranslateUi(QDialog *LengthDialog)
{
    LengthDialog->setWindowTitle(QCoreApplication::translate("LengthDialog", "Dialog"));
    minLabel->setText(QCoreApplication::translate("LengthDialog", "2"));
    titleLabel->setText(QCoreApplication::translate("LengthDialog", "Simulation Length"));
    maxLabel->setText(QCoreApplication::translate("LengthDialog", "2048"));
}
