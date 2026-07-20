// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief RealFileDialogProvider: the QFileDialog-backed FileDialogProvider (App/Core/
/// FileDialogProvider.h) implementation Widgets uses.

#pragma once

#include "App/Core/FileDialogProvider.h"

/// Production implementation that delegates to QFileDialog.
class RealFileDialogProvider : public FileDialogProvider
{
public:
    QString getOpenFileName(QWidget *parent, const QString &caption,
                            const QString &dir, const QString &filter) override;

    FileDialogResult getSaveFileName(QWidget *parent, const QString &caption,
                                     const QString &dir, const QString &filter) override;
};
