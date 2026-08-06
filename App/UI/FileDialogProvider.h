// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief Abstract file dialog interface for testability.

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
/// Production code uses the global accessor FileDialogs::provider() which
/// returns a RealFileDialogProvider by default.  Tests swap in a stub via
/// FileDialogs::setProvider().
class FileDialogProvider
{
public:
    // Same D0/D2 ABI-variant gap as App/Wiring/Port.h's destructor: FileDialogProvider is
    // abstract (getOpenFileName()/getSaveFileName() are pure virtual), so it can never be the
    // most-derived type of a real object -- its own vtable "deleting destructor" (D0) slot is
    // dead by construction. Every real instance (the static s_realProvider, or a test's
    // stack-local stub passed by address to FileDialogs::setProvider()) is destroyed via normal
    // scope/static-storage teardown, never `delete`d through this base pointer.
    virtual ~FileDialogProvider() = default; // LCOV_EXCL_LINE

    /// Shows an "Open File" dialog and returns the selected file path.
    /// Returns an empty string if the user cancels.
    virtual QString getOpenFileName(QWidget *parent, const QString &caption,
                                    const QString &dir, const QString &filter) = 0;

    /// Shows a "Save File" dialog and returns the selected path and filter.
    /// Returns an empty fileName if the user cancels.
    virtual FileDialogResult getSaveFileName(QWidget *parent, const QString &caption,
                                             const QString &dir, const QString &filter) = 0;
};

/// Production implementation that delegates to QFileDialog.
class RealFileDialogProvider : public FileDialogProvider
{
public:
    QString getOpenFileName(QWidget *parent, const QString &caption,
                            const QString &dir, const QString &filter) override;

    FileDialogResult getSaveFileName(QWidget *parent, const QString &caption,
                                     const QString &dir, const QString &filter) override;
};

/// Global accessor for the current FileDialogProvider.
namespace FileDialogs {

/// Returns the active provider.  Never null.
FileDialogProvider *provider();

/// Replaces the active provider.  Pass nullptr to restore the default.
/// Returns the previous provider (for RAII restoration in tests).
FileDialogProvider *setProvider(FileDialogProvider *newProvider);

} // namespace FileDialogs
