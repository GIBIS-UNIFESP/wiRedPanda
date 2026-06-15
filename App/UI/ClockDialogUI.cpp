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

    // The value is a clock period in waveform time-step columns. Minimum of 2 because a
    // period of 1 would toggle every step (degenerate); step of 2 enforces even periods so
    // the 50% duty cycle divides cleanly.
    periodSpinBox = new QSpinBox(ClockDialog);
    periodSpinBox->setObjectName("periodSpinBox");
    periodSpinBox->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
    periodSpinBox->setMinimum(2);
    periodSpinBox->setMaximum(1024);
    periodSpinBox->setSingleStep(2);
    gridLayout_2->addWidget(periodSpinBox, 0, 1, 1, 1);

    periodSlider = new QSlider(ClockDialog);
    periodSlider->setObjectName("periodSlider");
    // Slider mirrors the spin box constraints exactly so both controls always
    // show the same legal range.
    periodSlider->setMinimum(2);
    periodSlider->setMaximum(1024);
    periodSlider->setSingleStep(2);
    periodSlider->setOrientation(Qt::Horizontal);
    gridLayout_2->addWidget(periodSlider, 3, 0, 1, 2);

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
    QObject::connect(periodSlider, &QSlider::valueChanged, periodSpinBox, &QSpinBox::setValue);
    QObject::connect(periodSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), periodSlider, &QSlider::setValue);

    QMetaObject::connectSlotsByName(ClockDialog);
}

void ClockDialogUi::retranslateUi(QDialog *ClockDialog)
{
    ClockDialog->setWindowTitle(QCoreApplication::translate("ClockDialog", "Dialog"));
    titleLabel->setText(QCoreApplication::translate("ClockDialog", "Clock Period"));
    maxLabel->setText(QCoreApplication::translate("ClockDialog", "1024"));
    minLabel->setText(QCoreApplication::translate("ClockDialog", "2"));
}
