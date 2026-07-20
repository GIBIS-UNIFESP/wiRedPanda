// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief Abstract file dialog interface for testability -- shared between both apps.

#pragma once

#include <QString>

class QWidget;

/// Result from a save file dialog operation.
struct FileDialogResult
{
    QString fileName;        ///< Selected file path, empty if cancelled.
    QString selectedFilter;  ///< The name filter the user chose (e.g. "CSV files (*.csv)").
};

/// Abstract interface for file dialog operations.
///
/// Production code uses the global accessor FileDialogs::provider(). Each app registers its own
/// concrete implementation once, at startup, via FileDialogs::setDefaultProvider() --
/// App/Main.cpp (Widgets) with RealFileDialogProvider (App/UI/FileDialogProvider.h,
/// QFileDialog-backed, Widgets-only), App/QuickShell/Main.cpp (Quick) with QuickDialogProvider
/// (a QML Dialog-backed implementation). This header itself stays framework-agnostic on
/// purpose -- Phase 8c of the qtquick-rewrite plan extracted it out of App/UI/ for exactly that
/// reason -- so it must never hardcode either concrete implementation as a default itself.
/// Tests swap in a stub via FileDialogs::setProvider(), which restores the registered default
/// (not null) when passed nullptr -- see its own doc comment.
class FileDialogProvider
{
public:
    virtual ~FileDialogProvider() = default;

    /// Shows an "Open File" dialog and returns the selected file path.
    /// Returns an empty string if the user cancels.
    virtual QString getOpenFileName(QWidget *parent, const QString &caption,
                                    const QString &dir, const QString &filter) = 0;

    /// Shows a "Save File" dialog and returns the selected path and filter.
    /// Returns an empty fileName if the user cancels.
    virtual FileDialogResult getSaveFileName(QWidget *parent, const QString &caption,
                                             const QString &dir, const QString &filter) = 0;
};

/// Global accessor for the current FileDialogProvider.
namespace FileDialogs {

/// Returns the active provider. Asserts if setDefaultProvider() was never called -- every real
/// app entry point (and the test runners) calls it once at startup; a call before that is a
/// real bug, not a case to silently paper over with an implicit fallback.
FileDialogProvider *provider();

/// Registers \a provider as both the active provider and the one setProvider(nullptr) restores.
/// Called once per process, at startup, by each app/test-runner entry point with its own
/// concrete implementation (see the class doc comment above).
void setDefaultProvider(FileDialogProvider *provider);

/// Replaces the active provider. Pass nullptr to restore the registered default (see
/// setDefaultProvider()) -- e.g. a test's cleanup() undoing a temporary stub installed via
/// setProvider(&stub). Returns the previous provider (for RAII restoration in tests).
FileDialogProvider *setProvider(FileDialogProvider *newProvider);

} // namespace FileDialogs
