// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/FileDialogProvider.h"

#include <QFileDialog>

// ---------------------------------------------------------------------------
// Global state
// ---------------------------------------------------------------------------

static RealFileDialogProvider s_realProvider;
static FileDialogProvider *s_provider = &s_realProvider;

FileDialogProvider *FileDialogs::provider()
{
    return s_provider;
}

FileDialogProvider *FileDialogs::setProvider(FileDialogProvider *newProvider)
{
    auto *old = s_provider;
    s_provider = newProvider ? newProvider : &s_realProvider;
    return old;
}

// ---------------------------------------------------------------------------
// RealFileDialogProvider
// ---------------------------------------------------------------------------

QString RealFileDialogProvider::getOpenFileName(
    QWidget *parent, const QString &caption,
    const QString &dir, const QString &filter)
{
    return QFileDialog::getOpenFileName(parent, caption, dir, filter);
}

FileDialogResult RealFileDialogProvider::getSaveFileName(
    QWidget *parent, const QString &caption,
    const QString &dir, const QString &filter)
{
    QFileDialog dialog(parent, caption, dir, filter);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);

    if (dialog.exec() == QDialog::Rejected) {
        return {};
    }

    const auto files = dialog.selectedFiles();
    if (files.isEmpty()) {
        return {};
    }

    return {files.constFirst(), dialog.selectedNameFilter()};
}

