// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestICRegistry.h"

#include <QTemporaryDir>

#include "App/Element/ICRegistry.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestICRegistry::testICRegistration()
{
    WorkSpace workspace;
    ICRegistry registry(workspace.scene());
    QVERIFY(registry.blobMap().isEmpty());
}

void TestICRegistry::testICFileWatcher()
{
    WorkSpace workspace;
    ICRegistry registry(workspace.scene());

    // Exercise the cache + invalidate mechanics the file watcher relies on
    // (the QFileSystemWatcher's async fileChanged delivery itself is not
    // deterministically unit-testable).
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("watched.panda");

    auto writeFile = [&path](const QByteArray &content) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
        file.write(content);
    };

    writeFile("v1");
    registry.watchFile(path);
    QCOMPARE(registry.cachedFileBytes(path), QByteArray("v1"));

    // The cache serves stale bytes until invalidated...
    writeFile("v2");
    QCOMPARE(registry.cachedFileBytes(path), QByteArray("v1"));

    // ...and invalidate() (what the watcher slot calls) forces a re-read
    registry.invalidate(path);
    QCOMPARE(registry.cachedFileBytes(path), QByteArray("v2"));
}

void TestICRegistry::testRecursiveICLoading()
{
    WorkSpace workspace;
    ICRegistry registry(workspace.scene());
    QVERIFY(registry.blobMap().isEmpty());
}

void TestICRegistry::testICValidation()
{
    WorkSpace workspace;
    ICRegistry registry(workspace.scene());
    QVERIFY(!registry.hasBlob("nonexistent"));
}
