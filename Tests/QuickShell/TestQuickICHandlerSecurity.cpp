// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickICHandlerSecurity.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include "App/Element/ElementFactory.h"
#include "App/IO/Serialization.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "MCP/Server/Handlers/QuickICHandler.h"

namespace {

/// Writes a minimal, valid, loadable IC fixture (one switch, one LED) to \a path. Uses
/// QuickWorkSpace, not the Widgets-side WorkSpace TestICHandlerSecurity.cpp's identical helper
/// reaches for -- WorkSpace is a real QWidget (confirmed the hard way: constructing one here
/// aborted with "QWidget: Cannot create a QWidget without QApplication", since this binary's
/// runner deliberately only ever constructs a QGuiApplication, matching wiredpanda_quick's own
/// production Main.cpp).
void writeLeafIcFixture(const QString &path)
{
    QuickWorkSpace ws;
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
    QVERIFY2(ws.save(path) == QuickWorkSpace::SaveOutcome::Saved, qPrintable(path));
}

/// \copydoc writeLeafIcFixture -- byte-array variant, for registerBlob() instead of a file.
QByteArray leafIcBytes()
{
    QuickWorkSpace ws;
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    ws.save(stream);
    return bytes;
}

} // namespace

void TestQuickICHandlerSecurity::testEmbedIcRejectsPathTraversalBlobName()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString icFileName = "leaf_ic";
    writeLeafIcFixture(tmpDir.path() + "/" + icFileName + ".panda");

    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject instParams{{"ic_name", icFileName}, {"x", 0}, {"y", 0}};
    const QJsonObject instResponse = handler.handleCommand("instantiate_ic", instParams, {});
    QVERIFY2(instResponse.contains("result"), qPrintable(QJsonDocument(instResponse).toJson()));
    const int elementId = instResponse["result"].toObject()["element_id"].toInt();

    const QJsonObject embedParams{{"element_id", elementId}, {"blob_name", "../../evil"}};
    const QJsonObject embedResponse = handler.handleCommand("embed_ic", embedParams, {});
    QVERIFY(embedResponse.contains("error"));
    QVERIFY(!controller.currentTab()->canvas()->icRegistry()->hasBlob("../../evil"));
}

void TestQuickICHandlerSecurity::testInstantiateIcInlineRejectsPathTraversalBlobName()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString icFileName = "leaf_ic2";
    writeLeafIcFixture(tmpDir.path() + "/" + icFileName + ".panda");

    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject params{
        {"ic_name", icFileName}, {"x", 0}, {"y", 0}, {"inline", true}, {"blob_name", "../../evil"}};
    const QJsonObject response = handler.handleCommand("instantiate_ic", params, {});
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(!controller.currentTab()->canvas()->icRegistry()->hasBlob("../../evil"));
}

void TestQuickICHandlerSecurity::testExtractIcConfinesFileNameToProjectDirectory()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    // No IC in the scene references this blob, so extractToFile() never re-parses it after
    // writing -- it only needs to exist for hasBlob() to pass while the confinement check
    // (which runs first) is exercised.
    controller.currentTab()->canvas()->icRegistry()->registerBlob("mytest", leafIcBytes());

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

void TestQuickICHandlerSecurity::testInstantiateIcLeavesNoElementOnLoadFailure()
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
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    CanvasItem *canvas = controller.currentTab()->canvas();
    const qsizetype countBefore = canvas->elements().size();

    const QJsonObject params{{"ic_name", "corrupt"}, {"x", 0}, {"y", 0}};
    const QJsonObject response = handler.handleCommand("instantiate_ic", params, {});
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(canvas->elements().size(), countBefore);
}
