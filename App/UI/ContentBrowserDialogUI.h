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

/// Which kind of learning content the dialog is browsing — selects the discover()/
/// preferredContentDir() key, the translation context, and the hardcoded English source
/// strings used by ContentBrowserDialogUi::retranslateUi().
enum class ContentCategory { Exercises, Tours };

class ContentBrowserDialogUi
{
public:
    ContentBrowserDialogUi() = default;
    ContentBrowserDialogUi(const ContentBrowserDialogUi &) = delete;
    ContentBrowserDialogUi &operator=(const ContentBrowserDialogUi &) = delete;

    void setupUi(QDialog *dialog, ContentCategory category);
    void retranslateUi(QDialog *dialog, ContentCategory category);

    QLabel          *titleLabel       = nullptr;
    QListWidget     *contentList      = nullptr;
    QLabel          *descriptionLabel = nullptr;
    QDialogButtonBox *buttonBox       = nullptr;
    QPushButton     *startButton      = nullptr;
    QPushButton     *openFolderButton = nullptr;
};
