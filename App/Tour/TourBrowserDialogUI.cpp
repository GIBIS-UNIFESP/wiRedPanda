// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Tour/TourBrowserDialogUI.h"

void TourBrowserDialogUi::setupUi(QDialog *dialog)
{
    dialog->resize(420, 300);

    auto *mainLayout = new QVBoxLayout(dialog);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    titleLabel = new QLabel(dialog);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    tourList = new QListWidget(dialog);
    tourList->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(tourList, 1);

    descriptionLabel = new QLabel(dialog);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setStyleSheet("color: gray; font-size: 11px;");
    mainLayout->addWidget(descriptionLabel);

    buttonBox = new QDialogButtonBox(dialog);
    startButton = new QPushButton(dialog);
    startButton->setEnabled(false);
    buttonBox->addButton(startButton, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Close);
    mainLayout->addWidget(buttonBox);

    retranslateUi(dialog);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
}

void TourBrowserDialogUi::retranslateUi(QDialog *dialog)
{
    dialog->setWindowTitle(QCoreApplication::translate("TourBrowserDialog", "Interactive Tours"));
    titleLabel->setText(QCoreApplication::translate("TourBrowserDialog", "Choose a Tour"));
    startButton->setText(QCoreApplication::translate("TourBrowserDialog", "Start"));
}
