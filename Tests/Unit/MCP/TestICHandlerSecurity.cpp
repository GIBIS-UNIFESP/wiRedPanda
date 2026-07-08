// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestICHandlerSecurity.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/IO/Serialization.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "MCP/Server/Handlers/ICHandler.h"

namespace {

/// Writes a minimal, valid, loadable IC fixture (one switch, one LED) to \a path.
void writeLeafIcFixture(const QString &path)
{
    WorkSpace ws;
    ws.scene()->addItem(new InputSwitch());
    ws.scene()->addItem(new Led());
    QFile file(path);
    QVERIFY2(file.open(QIODevice::WriteOnly), qPrintable(path));
    QDataStream stream(&file);
    Serialization::writePandaHeader(stream);
    ws.save(stream);
}

/// Returns minimal, valid, self-contained panda bytes (one switch, one LED) — registerBlob()
/// parses these via makeBlobSelfContained() to inline nested dependencies, so garbage bytes
/// abort the whole test process rather than failing gracefully.
QByteArray leafIcBytes()
{
    WorkSpace ws;
    ws.scene()->addItem(new InputSwitch());
    ws.scene()->addItem(new Led());
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    ws.save(stream);
    return bytes;
}

} // namespace

void TestICHandlerSecurity::testEmbedIcRejectsPathTraversalBlobName()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString icFileName = "leaf_ic";
    writeLeafIcFixture(tmpDir.path() + "/" + icFileName + ".panda");

    const QString projectPath = tmpDir.path() + "/project.panda";
    WorkSpace().save(projectPath);

    MainWindow window;
    window.loadPandaFile(projectPath);
    ICHandler handler(&window, nullptr);

    const QJsonObject instParams{{"ic_name", icFileName}, {"x", 0}, {"y", 0}};
    const QJsonObject instResponse = handler.handleCommand("instantiate_ic", instParams, {});
    QVERIFY2(instResponse.contains("result"), qPrintable(QJsonDocument(instResponse).toJson()));
    const int elementId = instResponse["result"].toObject()["element_id"].toInt();

    const QJsonObject embedParams{{"element_id", elementId}, {"blob_name", "../../evil"}};
    const QJsonObject embedResponse = handler.handleCommand("embed_ic", embedParams, {});
    QVERIFY(embedResponse.contains("error"));
    QVERIFY(!window.currentTab()->scene()->icRegistry()->hasBlob("../../evil"));
}

void TestICHandlerSecurity::testInstantiateIcInlineRejectsPathTraversalBlobName()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString icFileName = "leaf_ic2";
    writeLeafIcFixture(tmpDir.path() + "/" + icFileName + ".panda");

    const QString projectPath = tmpDir.path() + "/project.panda";
    WorkSpace().save(projectPath);

    MainWindow window;
    window.loadPandaFile(projectPath);
    ICHandler handler(&window, nullptr);

    const QJsonObject params{
        {"ic_name", icFileName}, {"x", 0}, {"y", 0}, {"inline", true}, {"blob_name", "../../evil"}};
    const QJsonObject response = handler.handleCommand("instantiate_ic", params, {});
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(!window.currentTab()->scene()->icRegistry()->hasBlob("../../evil"));
}

void TestICHandlerSecurity::testExtractIcConfinesFileNameToProjectDirectory()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString projectPath = tmpDir.path() + "/project.panda";
    WorkSpace().save(projectPath);

    MainWindow window;
    window.loadPandaFile(projectPath);
    ICHandler handler(&window, nullptr);

    // No IC in the scene references this blob, so extractToFile() never re-parses it after
    // writing — it only needs to exist for hasBlob() to pass while the confinement check
    // (which runs first) is exercised.
    window.currentTab()->scene()->icRegistry()->registerBlob("mytest", leafIcBytes());

    // Relative traversal escaping the project directory.
    const QJsonObject traversalParams{{"blob_name", "mytest"}, {"file_name", "../evil.panda"}};
    const QJsonObject traversalResponse = handler.handleCommand("extract_ic", traversalParams, {});
    QVERIFY(traversalResponse.contains("error"));
    QVERIFY(!QFile::exists(QDir::cleanPath(tmpDir.path() + "/../evil.panda")));

    // Absolute path escaping the project directory outright.
    const QString outsidePath = QDir::cleanPath(tmpDir.path() + "/../outside_evil.panda");
    const QJsonObject absoluteParams{{"blob_name", "mytest"}, {"file_name", outsidePath}};
    const QJsonObject absoluteResponse = handler.handleCommand("extract_ic", absoluteParams, {});
    QVERIFY(absoluteResponse.contains("error"));
    QVERIFY(!QFile::exists(outsidePath));

    // A legitimate subdirectory of the project directory remains allowed.
    QVERIFY(QDir(tmpDir.path()).mkpath("subdir"));
    const QJsonObject legitParams{{"blob_name", "mytest"}, {"file_name", "subdir/out.panda"}};
    const QJsonObject legitResponse = handler.handleCommand("extract_ic", legitParams, {});
    QVERIFY2(legitResponse.contains("result"), qPrintable(QJsonDocument(legitResponse).toJson()));
    QVERIFY(QFile::exists(tmpDir.path() + "/subdir/out.panda"));
}

void TestICHandlerSecurity::testInstantiateIcLeavesNoElementOnLoadFailure()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString corruptPath = tmpDir.path() + "/corrupt.panda";
    {
        QFile file(corruptPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("not a panda file");
    }

    const QString projectPath = tmpDir.path() + "/project.panda";
    WorkSpace().save(projectPath);

    MainWindow window;
    window.loadPandaFile(projectPath);
    ICHandler handler(&window, nullptr);

    Scene *scene = window.currentTab()->scene();
    const qsizetype countBefore = scene->elements().size();

    const QJsonObject params{{"ic_name", "corrupt"}, {"x", 0}, {"y", 0}};
    const QJsonObject response = handler.handleCommand("instantiate_ic", params, {});
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(scene->elements().size(), countBefore);
}
