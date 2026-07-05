// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Exercise/ExerciseBrowserDialogUI.h"

void ExerciseBrowserDialogUi::setupUi(QDialog *dialog)
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

    exerciseList = new QListWidget(dialog);
    exerciseList->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(exerciseList, 1);

    descriptionLabel = new QLabel(dialog);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setStyleSheet("color: gray; font-size: 11px;");
    mainLayout->addWidget(descriptionLabel);

    buttonBox = new QDialogButtonBox(dialog);
    openFolderButton = new QPushButton(dialog);
    buttonBox->addButton(openFolderButton, QDialogButtonBox::ActionRole);
    startButton = new QPushButton(dialog);
    startButton->setEnabled(false);
    buttonBox->addButton(startButton, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(QDialogButtonBox::Close);
    mainLayout->addWidget(buttonBox);

    retranslateUi(dialog);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
}

void ExerciseBrowserDialogUi::retranslateUi(QDialog *dialog)
{
    dialog->setWindowTitle(QCoreApplication::translate("ExerciseBrowserDialog", "Circuit Exercises"));
    titleLabel->setText(QCoreApplication::translate("ExerciseBrowserDialog", "Choose an Exercise"));
    startButton->setText(QCoreApplication::translate("ExerciseBrowserDialog", "Start"));
    openFolderButton->setText(QCoreApplication::translate("ExerciseBrowserDialog", "Open My Exercises Folder"));
}
