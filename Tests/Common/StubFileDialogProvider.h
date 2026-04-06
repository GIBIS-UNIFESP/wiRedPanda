// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief Test stub for FileDialogProvider with parameter capture.

#pragma once

#include "App/UI/FileDialogProvider.h"

/// Test stub that returns pre-configured results and captures call parameters.
class StubFileDialogProvider : public FileDialogProvider
{
public:
    // --- Pre-configured return values (set before calling code-under-test) ---

    QString openResult;          ///< Path that getOpenFileName will return.
    FileDialogResult saveResult; ///< Result that getSaveFileName will return.

    // --- Captured parameters (read after calling code-under-test) ---

    struct LastCall {
        QString caption;
        QString dir;
        QString filter;
        QWidget *parent = nullptr;
    };

    LastCall lastOpenCall;   ///< Parameters from the last getOpenFileName call.
    LastCall lastSaveCall;   ///< Parameters from the last getSaveFileName call.
    int openCallCount = 0;   ///< Number of times getOpenFileName was called.
    int saveCallCount = 0;   ///< Number of times getSaveFileName was called.

    QString getOpenFileName(QWidget *parent, const QString &caption,
                            const QString &dir, const QString &filter) override
    {
        lastOpenCall = {caption, dir, filter, parent};
        ++openCallCount;
        return openResult;
    }

    FileDialogResult getSaveFileName(QWidget *parent, const QString &caption,
                                     const QString &dir, const QString &filter) override
    {
        lastSaveCall = {caption, dir, filter, parent};
        ++saveCallCount;
        return saveResult;
    }
};

/// RAII guard that installs a StubFileDialogProvider and restores the previous
/// provider on destruction.  Prevents forgetting to restore in test cleanup.
///
/// Usage:
/// \code
///     ScopedFileDialogStub guard;
///     guard.stub.openResult = "/tmp/test.panda";
///     // ... call code-under-test ...
///     QCOMPARE(guard.stub.openCallCount, 1);
/// \endcode
class ScopedFileDialogStub
{
public:
    StubFileDialogProvider stub;

    ScopedFileDialogStub()
        : m_old(FileDialogs::setProvider(&stub))
    {
    }

    ~ScopedFileDialogStub()
    {
        FileDialogs::setProvider(m_old);
    }

    ScopedFileDialogStub(const ScopedFileDialogStub &) = delete;
    ScopedFileDialogStub &operator=(const ScopedFileDialogStub &) = delete;

private:
    FileDialogProvider *m_old;
};

