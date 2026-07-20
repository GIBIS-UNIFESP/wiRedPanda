// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/**
 * \class TestICRegistry
 * \brief CanvasICRegistry equivalent of the Widgets ICRegistry's own TestICRegistry.
 *
 * \details testICFileWatcher (QFileSystemWatcher-driven hot-reload) and testRecursiveICLoading
 * (a legitimate short dependency chain resolving via registerBlob()'s recursive
 * makeBlobSelfContained() inlining) are dropped, not ported: CanvasICRegistry.h's own doc
 * comment confirms both file watching and registerBlob()'s recursive dependency-chain handling
 * are deliberately not ported -- Quick's registerBlob() stores blob bytes as-is, with no
 * recursive walk to test. testMakeBlobSelfContainedRejectsDeepDependencyChain is the same
 * deferral (already tracked via QuickICController.h's own doc comment: "no toolbar-button UI
 * trigger exists yet" for makeBlobSelfContained()).
 */
class TestICRegistry : public QObject
{
    Q_OBJECT

private slots:

    void testICRegistration();
    void testICValidation();
};
