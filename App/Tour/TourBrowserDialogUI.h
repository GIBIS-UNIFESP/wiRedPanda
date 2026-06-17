// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

class TourBrowserDialogUi
{
public:
    TourBrowserDialogUi() = default;
    TourBrowserDialogUi(const TourBrowserDialogUi &) = delete;
    TourBrowserDialogUi &operator=(const TourBrowserDialogUi &) = delete;

    void setupUi(QDialog *dialog);
    void retranslateUi(QDialog *dialog);

    QLabel          *titleLabel       = nullptr;
    QListWidget     *tourList         = nullptr;
    QLabel          *descriptionLabel = nullptr;
    QDialogButtonBox *buttonBox       = nullptr;
    QPushButton     *startButton      = nullptr;
};
