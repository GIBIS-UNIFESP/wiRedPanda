// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickICHandler.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

#include "App/Core/Application.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Handlers/QuickICHandler.h"

namespace {

/// Writes a minimal, valid, loadable IC fixture (one switch, one LED) to \a path. Uses
/// QuickWorkSpace, not the Widgets-side WorkSpace TestICHandler.cpp's identical helper reaches
/// for -- WorkSpace is a real QWidget, which would abort with "QWidget: Cannot create a QWidget
/// without QApplication" against this binary's QGuiApplication-only runner.
void writeLeafIcHandlerFixture(const QString &path)
{
    QuickWorkSpace ws;
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    ws.canvas()->addItem(ElementFactory::buildElement(ElementType::Led));
    QVERIFY2(ws.save(path) == QuickWorkSpace::SaveOutcome::Saved, qPrintable(path));
}

/// Returns minimal, valid, self-contained panda bytes (one switch, one LED).
QByteArray leafIcHandlerBytes()
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

/// Restores \a path's permissions to read+write+execute on destruction (including on an early
/// return from a failed QVERIFY/QCOMPARE) -- the caller narrows permissions itself (so its own
/// QVERIFY reports a real failure there), this guard only guarantees the restore isn't skipped.
struct ScopedDirPermissionsRestore {
    QString path;

    ~ScopedDirPermissionsRestore()
    {
        QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    }
};

} // namespace

void TestQuickICHandler::cleanup()
{
    // Some tests flip this to mirror real MCP execution (see
    // testHandleCreateIcFailsOnUnwritableTarget) -- restore the default so it can't leak into
    // unrelated tests run afterward in the same process.
    Application::interactiveMode = true;
}

void TestQuickICHandler::testHandleCommandRejectsUnknownCommand()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("not_a_real_command", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}

// ============================================================================
// create_ic
// ============================================================================

void TestQuickICHandler::testHandleCreateIcRejectsMissingParams()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("create_ic", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickICHandler::testHandleCreateIcRejectsNoScene()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("create_ic", {{"name", "MyIC"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestQuickICHandler::testHandleCreateIcRejectsEmptyName()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("create_ic", {{"name", ""}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickICHandler::testHandleCreateIcRejectsNameWithPathSeparator()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("create_ic", {{"name", "sub/dir"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickICHandler::testHandleCreateIcCreatesRealFile()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    CanvasItem *canvas = controller.currentTab()->canvas();
    canvas->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    canvas->addItem(ElementFactory::buildElement(ElementType::Led));

    QuickICHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand(
        "create_ic", {{"name", "MyNewIc"}, {"description", "a test IC"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["name"].toString(), QStringLiteral("MyNewIc"));
    QCOMPARE(result["filename"].toString(), QStringLiteral("MyNewIc.panda"));
    QCOMPARE(result["elements_count"].toInt(), 2);
    QCOMPARE(result["description"].toString(), QStringLiteral("a test IC"));
    QVERIFY(QFile::exists(tmpDir.path() + "/MyNewIc.panda"));
}

void TestQuickICHandler::testHandleCreateIcRejectsExistingFile()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    {
        QFile existing(tmpDir.path() + "/Existing.panda");
        QVERIFY(existing.open(QIODevice::WriteOnly));
        existing.write("x");
    }

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    controller.currentTab()->canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));

    QuickICHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("create_ic", {{"name", "Existing"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::IcError);
}

void TestQuickICHandler::testHandleCreateIcRejectsEmptyCircuit()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);

    QuickICHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("create_ic", {{"name", "Empty"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::IcError);
}

void TestQuickICHandler::testHandleCreateIcFailsOnUnwritableTarget()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot make a directory unwritable on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    controller.currentTab()->canvas()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));

    // Application::interactiveMode is unconditionally false for the whole lifetime of a real
    // MCP server process (Main.cpp sets it before any handler is ever constructed for
    // --mcp-mode/--mcp-gui), so QuickWorkSpace::save()'s open-failure path always throws rather
    // than returning SaveOutcome::ReadOnlyTarget there -- mirror that real state here instead of
    // relying on this test binary's own default (true).
    Application::interactiveMode = false;

    QVERIFY(QFile::setPermissions(tmpDir.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner));
    ScopedDirPermissionsRestore permGuard{tmpDir.path()};

    QuickICHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("create_ic", {{"name", "Unwritable"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::OperationFailed);
#endif
}

// ============================================================================
// instantiate_ic
// ============================================================================

void TestQuickICHandler::testHandleInstantiateIcRejectsMissingParams()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("instantiate_ic", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickICHandler::testHandleInstantiateIcRejectsNoScene()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("instantiate_ic", {{"ic_name", "leaf"}, {"x", 0}, {"y", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestQuickICHandler::testHandleInstantiateIcRejectsEmptyName()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("instantiate_ic", {{"ic_name", ""}, {"x", 0}, {"y", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickICHandler::testHandleInstantiateIcRejectsAbsolutePathWithDotDot()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand(
        "instantiate_ic", {{"ic_name", "/tmp/../etc/evil"}, {"x", 0}, {"y", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickICHandler::testHandleInstantiateIcRejectsRelativeNameWithPathSeparator()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("instantiate_ic", {{"ic_name", "sub/dir"}, {"x", 0}, {"y", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickICHandler::testHandleInstantiateIcRejectsMissingFile()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand(
        "instantiate_ic", {{"ic_name", "does_not_exist"}, {"x", 0}, {"y", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::IcError);
}

void TestQuickICHandler::testHandleInstantiateIcInstantiatesRealFileBackedIc()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/leaf.panda");
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("instantiate_ic", {{"ic_name", "leaf"}, {"x", 13}, {"y", 27}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QVERIFY(result["element_id"].toInt() != 0);
    QCOMPARE(result["ic_name"].toString(), QStringLiteral("leaf"));
    QCOMPARE(result["label"].toString(), QStringLiteral("leaf"));
    QCOMPARE(result["input_count"].toInt(), 1);
    QCOMPARE(result["output_count"].toInt(), 1);
    QVERIFY(!result.contains("inline"));

    auto *ic = dynamic_cast<IC *>(controller.currentTab()->canvas()->itemById(result["element_id"].toInt()));
    QVERIFY(ic);
    QCOMPARE(ic->pos(), QPointF(16, 24)); // snapped to gridSize/2=8: round(13/8)*8=16, round(27/8)*8=24
}

void TestQuickICHandler::testHandleInstantiateIcInstantiatesViaAbsolutePath()
{
    QTemporaryDir icDir;
    QVERIFY(icDir.isValid());
    writeLeafIcHandlerFixture(icDir.path() + "/leaf2.panda");

    QTemporaryDir projectDir;
    QVERIFY(projectDir.isValid());
    const QString projectPath = projectDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QString absoluteIcName = icDir.path() + "/leaf2";
    const QJsonObject response = handler.handleCommand(
        "instantiate_ic", {{"ic_name", absoluteIcName}, {"x", 0}, {"y", 0}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["ic_name"].toString(), absoluteIcName);
}

void TestQuickICHandler::testHandleInstantiateIcInlineRejectsUnreadableFile()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot simulate an unreadable file on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/leaf3.panda");
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);

    QVERIFY(QFile::setPermissions(tmpDir.path() + "/leaf3.panda", QFileDevice::Permissions()));
    {
        QFile f(tmpDir.path() + "/leaf3.panda");
        QVERIFY(!f.open(QIODevice::ReadOnly));
    }

    QuickICHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand(
        "instantiate_ic", {{"ic_name", "leaf3"}, {"x", 0}, {"y", 0}, {"inline", true}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::FileError);

    QFile::setPermissions(tmpDir.path() + "/leaf3.panda", QFileDevice::ReadOwner | QFileDevice::WriteOwner);
#endif
}

void TestQuickICHandler::testHandleInstantiateIcInlineDefaultsBlobNameToBaseName()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/leaf4.panda");
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand(
        "instantiate_ic", {{"ic_name", "leaf4"}, {"x", 0}, {"y", 0}, {"inline", true}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QVERIFY(result["inline"].toBool());
    QCOMPARE(result["blob_name"].toString(), QStringLiteral("leaf4"));
    QVERIFY(controller.currentTab()->canvas()->icRegistry()->hasBlob("leaf4"));
}

void TestQuickICHandler::testHandleInstantiateIcInlineRejectsBlobNameCollision()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/leaf5.panda");
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    controller.currentTab()->canvas()->icRegistry()->registerBlob("dup", leafIcHandlerBytes());

    QuickICHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand(
        "instantiate_ic", {{"ic_name", "leaf5"}, {"x", 0}, {"y", 0}, {"inline", true}, {"blob_name", "dup"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::IcError);
}

void TestQuickICHandler::testHandleInstantiateIcInlineCreatesRealEmbeddedIc()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/leaf6.panda");
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand(
        "instantiate_ic", {{"ic_name", "leaf6"}, {"x", 0}, {"y", 0}, {"inline", true}, {"blob_name", "myblob"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const int elementId = response["result"].toObject()["element_id"].toInt();

    auto *ic = dynamic_cast<IC *>(controller.currentTab()->canvas()->itemById(elementId));
    QVERIFY(ic);
    QVERIFY(ic->isEmbedded());
    QCOMPARE(ic->blobName(), QStringLiteral("myblob"));
}

// ============================================================================
// list_ics
// ============================================================================

void TestQuickICHandler::testHandleListIcsReturnsRealDefinitions()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/leaf7.panda");
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("list_ics", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["directory"].toString(), QDir(tmpDir.path()).absolutePath());

    const QJsonArray ics = result["ics"].toArray();
    QCOMPARE(result["count"].toInt(), ics.size());

    bool foundLeaf = false;
    for (const auto &v : ics) {
        const QJsonObject obj = v.toObject();
        if (obj["name"].toString() == "leaf7") {
            QVERIFY(obj["has_valid_definition"].toBool());
            QCOMPARE(obj["input_count"].toInt(), 1);
            QCOMPARE(obj["output_count"].toInt(), 1);
            foundLeaf = true;
        }
    }
    QVERIFY(foundLeaf);
}

void TestQuickICHandler::testHandleListIcsSkipsUnreadableFile()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot simulate an unreadable file on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/good.panda");
    writeLeafIcHandlerFixture(tmpDir.path() + "/locked.panda");
    QVERIFY(QFile::setPermissions(tmpDir.path() + "/locked.panda", QFileDevice::Permissions()));
    {
        QFile f(tmpDir.path() + "/locked.panda");
        QVERIFY(!f.open(QIODevice::ReadOnly));
    }

    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);
    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("list_ics", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonArray ics = response["result"].toObject()["ics"].toArray();
    bool foundGood = false;
    bool foundLocked = false;
    for (const auto &v : ics) {
        const QString name = v.toObject()["name"].toString();
        if (name == "good") {
            foundGood = true;
        }
        if (name == "locked") {
            foundLocked = true;
        }
    }
    QVERIFY(foundGood);
    QVERIFY(!foundLocked);

    QFile::setPermissions(tmpDir.path() + "/locked.panda", QFileDevice::ReadOwner | QFileDevice::WriteOwner);
#endif
}

void TestQuickICHandler::testHandleListIcsMarksCorruptBodyAsInvalidDefinition()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());

    const QString corruptPath = tmpDir.path() + "/corruptbody.panda";
    {
        QFile file(corruptPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QDataStream stream(&file);
        Serialization::writePandaHeader(stream);
        // No body follows -- a valid header with nothing else lets IC::loadFile() fail
        // parsing the (missing) circuit body without failing the header check itself.
    }

    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);
    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("list_ics", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonArray ics = response["result"].toObject()["ics"].toArray();
    bool found = false;
    for (const auto &v : ics) {
        const QJsonObject obj = v.toObject();
        if (obj["name"].toString() == "corruptbody") {
            QVERIFY(!obj["has_valid_definition"].toBool());
            QCOMPARE(obj["input_count"].toInt(), 0);
            QCOMPARE(obj["output_count"].toInt(), 0);
            found = true;
        }
    }
    QVERIFY(found);
}

void TestQuickICHandler::testHandleListIcsSkipsCorruptHeaderViaOuterCatch()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/good2.panda");

    const QString corruptPath = tmpDir.path() + "/corrupthdr.panda";
    {
        QFile file(corruptPath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QDataStream stream(&file);
        stream.setVersion(QDataStream::Qt_5_12);
        stream << Serialization::MAGIC_HEADER_CIRCUIT;
        // Implausible version-segment count: Serialization::readVersionNumber() throws for
        // count > 8, and that throw happens outside handleListICs' inner try -- only its
        // outer catch(...) can ever see it.
        stream << quint32(99);
    }

    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);
    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("list_ics", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonArray ics = response["result"].toObject()["ics"].toArray();
    bool foundGood = false;
    bool foundCorrupt = false;
    for (const auto &v : ics) {
        const QString name = v.toObject()["name"].toString();
        if (name == "good2") {
            foundGood = true;
        }
        if (name == "corrupthdr") {
            foundCorrupt = true;
        }
    }
    QVERIFY(foundGood);
    QVERIFY(!foundCorrupt);
}

// ============================================================================
// embed_ic
// ============================================================================

void TestQuickICHandler::testHandleEmbedIcRejectsMissingParams()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("embed_ic", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickICHandler::testHandleEmbedIcRejectsNoScene()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("embed_ic", {{"element_id", 1}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestQuickICHandler::testHandleEmbedIcRejectsNonPositiveElementId()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("embed_ic", {{"element_id", 0}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickICHandler::testHandleEmbedIcRejectsUnknownElement()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("embed_ic", {{"element_id", 999999}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ElementNotFound);
}

void TestQuickICHandler::testHandleEmbedIcRejectsNonIcElement()
{
    QuickAppController controller;
    controller.newTab();
    CanvasItem *canvas = controller.currentTab()->canvas();
    auto *elm = new And();
    canvas->addItem(elm);

    QuickICHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("embed_ic", {{"element_id", elm->id()}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestQuickICHandler::testHandleEmbedIcRejectsAlreadyEmbedded()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/leaf8.panda");
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject instResponse = handler.handleCommand(
        "instantiate_ic", {{"ic_name", "leaf8"}, {"x", 0}, {"y", 0}, {"inline", true}}, 1);
    const int id = instResponse["result"].toObject()["element_id"].toInt();

    const QJsonObject response = handler.handleCommand("embed_ic", {{"element_id", id}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::IcError);
}

void TestQuickICHandler::testHandleEmbedIcRejectsNoReferencedFile()
{
    QuickAppController controller;
    controller.newTab();
    CanvasItem *canvas = controller.currentTab()->canvas();
    auto *ic = new IC();
    canvas->addItem(ic);

    QuickICHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand("embed_ic", {{"element_id", ic->id()}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::IcError);
}

void TestQuickICHandler::testHandleEmbedIcRejectsProjectNotSaved()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/leaf9.panda");
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject instResponse = handler.handleCommand("instantiate_ic", {{"ic_name", "leaf9"}, {"x", 0}, {"y", 0}}, 1);
    const int id = instResponse["result"].toObject()["element_id"].toInt();

    CanvasItem *canvas = controller.currentTab()->canvas();
    QVERIFY(!canvas->contextDir().isEmpty());
    canvas->setContextDir("");

    const QJsonObject response = handler.handleCommand("embed_ic", {{"element_id", id}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::IcError);
}

void TestQuickICHandler::testHandleEmbedIcRejectsUnreadableFile()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot simulate an unreadable file on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/leaf10.panda");
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject instResponse = handler.handleCommand("instantiate_ic", {{"ic_name", "leaf10"}, {"x", 0}, {"y", 0}}, 1);
    const int id = instResponse["result"].toObject()["element_id"].toInt();

    QVERIFY(QFile::setPermissions(tmpDir.path() + "/leaf10.panda", QFileDevice::Permissions()));
    {
        QFile f(tmpDir.path() + "/leaf10.panda");
        QVERIFY(!f.open(QIODevice::ReadOnly));
    }

    const QJsonObject response = handler.handleCommand("embed_ic", {{"element_id", id}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::FileError);

    QFile::setPermissions(tmpDir.path() + "/leaf10.panda", QFileDevice::ReadOwner | QFileDevice::WriteOwner);
#endif
}

void TestQuickICHandler::testHandleEmbedIcRejectsBlobNameCollision()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/leaf11.panda");
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    controller.currentTab()->canvas()->icRegistry()->registerBlob("dup2", leafIcHandlerBytes());
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject instResponse = handler.handleCommand("instantiate_ic", {{"ic_name", "leaf11"}, {"x", 0}, {"y", 0}}, 1);
    const int id = instResponse["result"].toObject()["element_id"].toInt();

    const QJsonObject response = handler.handleCommand("embed_ic", {{"element_id", id}, {"blob_name", "dup2"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::IcError);
}

void TestQuickICHandler::testHandleEmbedIcEmbedsRealFileDefaultingBlobName()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    writeLeafIcHandlerFixture(tmpDir.path() + "/leaf12.panda");
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject instResponse = handler.handleCommand("instantiate_ic", {{"ic_name", "leaf12"}, {"x", 0}, {"y", 0}}, 1);
    const int id = instResponse["result"].toObject()["element_id"].toInt();

    const QJsonObject response = handler.handleCommand("embed_ic", {{"element_id", id}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();
    QCOMPARE(result["blob_name"].toString(), QStringLiteral("leaf12"));
    QCOMPARE(result["converted_count"].toInt(), 1);
    QVERIFY(controller.currentTab()->canvas()->icRegistry()->hasBlob("leaf12"));

    auto *ic = dynamic_cast<IC *>(controller.currentTab()->canvas()->itemById(id));
    QVERIFY(ic);
    QVERIFY(ic->isEmbedded());
}

// ============================================================================
// extract_ic
// ============================================================================

void TestQuickICHandler::testHandleExtractIcRejectsMissingParams()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("extract_ic", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickICHandler::testHandleExtractIcRejectsNoScene()
{
    QuickAppController controller;
    QVERIFY(!controller.currentTab());
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("extract_ic", {{"blob_name", "whatever"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestQuickICHandler::testHandleExtractIcRejectsProjectNotSaved()
{
    QuickAppController controller;
    controller.newTab();
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("extract_ic", {{"blob_name", "whatever"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::IcError);
}

void TestQuickICHandler::testHandleExtractIcRejectsEmptyBlobName()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("extract_ic", {{"blob_name", ""}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestQuickICHandler::testHandleExtractIcRejectsUnknownBlob()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("extract_ic", {{"blob_name", "nope"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::IcError);
}

void TestQuickICHandler::testHandleExtractIcAppendsPandaExtension()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    controller.currentTab()->canvas()->icRegistry()->registerBlob("blobext", leafIcHandlerBytes());
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("extract_ic", {{"blob_name", "blobext"}, {"file_name", "myoutput"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["file_name"].toString(), tmpDir.path() + "/myoutput.panda");
    QVERIFY(QFile::exists(tmpDir.path() + "/myoutput.panda"));
}

void TestQuickICHandler::testHandleExtractIcRejectsExistingFileWithoutOverwrite()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    controller.currentTab()->canvas()->icRegistry()->registerBlob("blobexist", leafIcHandlerBytes());

    const QString targetPath = tmpDir.path() + "/existing.panda";
    {
        QFile f(targetPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("x");
    }

    QuickICHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand(
        "extract_ic", {{"blob_name", "blobexist"}, {"file_name", "existing.panda"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::FileError);
}

void TestQuickICHandler::testHandleExtractIcOverwritesExistingFileWhenRequested()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    controller.currentTab()->canvas()->icRegistry()->registerBlob("blobexist2", leafIcHandlerBytes());

    const QString targetPath = tmpDir.path() + "/existing2.panda";
    {
        QFile f(targetPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("x");
    }

    QuickICHandler handler(&controller, nullptr);
    const QJsonObject response = handler.handleCommand(
        "extract_ic", {{"blob_name", "blobexist2"}, {"file_name", "existing2.panda"}, {"overwrite", true}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
}

void TestQuickICHandler::testHandleExtractIcExtractsRealFileWithDefaultName()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString projectPath = tmpDir.path() + "/project.panda";
    QuickWorkSpace().save(projectPath);

    QuickAppController controller;
    controller.newTab();
    controller.openRecentFile(projectPath);
    controller.currentTab()->canvas()->icRegistry()->registerBlob("blobdefault", leafIcHandlerBytes());
    QuickICHandler handler(&controller, nullptr);

    const QJsonObject response = handler.handleCommand("extract_ic", {{"blob_name", "blobdefault"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["file_name"].toString(), tmpDir.path() + "/blobdefault.panda");
    QVERIFY(QFile::exists(tmpDir.path() + "/blobdefault.panda"));
}
