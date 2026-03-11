// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ClockDialogUI.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>
#include <QtGui/QIcon>

void ClockDialogUi::setupUi(QDialog *ClockDialog)
{
    if (ClockDialog->objectName().isEmpty()) {
        ClockDialog->setObjectName("ClockDialog");
    }

    // Fixed dialog size — no resizing needed since the content is minimal.
    ClockDialog->resize(184, 116);

    ClockDialog->setWindowIcon(QIcon(":/Interface/Toolbar/wavyIcon.svg"));

    gridLayout_2 = new QGridLayout(ClockDialog);
    gridLayout_2->setObjectName("gridLayout_2");

    buttonBox = new QDialogButtonBox(ClockDialog);
    buttonBox->setObjectName("buttonBox");
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    gridLayout_2->addWidget(buttonBox, 6, 0, 1, 2);

    // Minimum of 2 because a clock with period 1 would toggle every step (degenerate);
    // step of 2 enforces even periods so 50% duty cycle divides cleanly
    frequencySpinBox = new QSpinBox(ClockDialog);
    frequencySpinBox->setObjectName("frequencySpinBox");
    frequencySpinBox->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    frequencySpinBox->setMinimum(2);
    frequencySpinBox->setMaximum(1024);
    frequencySpinBox->setSingleStep(2);
    gridLayout_2->addWidget(frequencySpinBox, 0, 1, 1, 1);

    frequencySlider = new QSlider(ClockDialog);
    frequencySlider->setObjectName("frequencySlider");
    // Slider mirrors the spin box constraints exactly so both controls always
    // show the same legal range.
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

    // Bidirectionally sync the slider and spin box so either control can drive the other
    QObject::connect(frequencySlider, &QSlider::valueChanged, frequencySpinBox, &QSpinBox::setValue);
    QObject::connect(frequencySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), frequencySlider, &QSlider::setValue);

    QMetaObject::connectSlotsByName(ClockDialog);
}

void ClockDialogUi::retranslateUi(QDialog *ClockDialog)
{
    ClockDialog->setWindowTitle(QCoreApplication::translate("ClockDialog", "Dialog"));
    titleLabel->setText(QCoreApplication::translate("ClockDialog", "Clock Frequency"));
    maxLabel->setText(QCoreApplication::translate("ClockDialog", "1024"));
    minLabel->setText(QCoreApplication::translate("ClockDialog", "2"));
}

