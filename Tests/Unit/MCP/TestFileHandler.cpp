// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestFileHandler.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTest>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Handlers/FileHandler.h"

namespace {

void addAndGate(Scene *scene)
{
    scene->receiveCommand(new AddItemsCommand({new And()}, scene));
}

} // namespace

void TestFileHandler::testHandleLoadCircuitRejectsMissingFilename()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("load_circuit", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestFileHandler::testHandleLoadCircuitRejectsEmptyFilename()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("load_circuit", {{"filename", ""}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestFileHandler::testHandleLoadCircuitRejectsNoMainWindow()
{
    FileHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("load_circuit", {{"filename", "x.panda"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InternalError);
}

void TestFileHandler::testHandleLoadCircuitLoadsRealFile()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    addAndGate(window.currentTab()->scene());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("circuit.panda");
    QVERIFY(handler.handleCommand("save_circuit", {{"filename", path}}, 1).contains("result"));

    const QJsonObject response = handler.handleCommand("load_circuit", {{"filename", path}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
}

void TestFileHandler::testHandleSaveCircuitRejectsMissingFilename()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("save_circuit", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestFileHandler::testHandleSaveCircuitRejectsEmptyFilename()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("save_circuit", {{"filename", ""}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestFileHandler::testHandleSaveCircuitRejectsWrongExtension()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("save_circuit", {{"filename", "circuit.txt"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestFileHandler::testHandleSaveCircuitRejectsNoMainWindow()
{
    FileHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("save_circuit", {{"filename", "x.panda"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InternalError);
}

void TestFileHandler::testHandleSaveCircuitSavesRealFile()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    addAndGate(window.currentTab()->scene());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("circuit.panda");

    const QJsonObject response = handler.handleCommand("save_circuit", {{"filename", path}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(QFile::exists(path));
    QVERIFY(QFileInfo(path).size() > 0);
}

void TestFileHandler::testHandleNewCircuitRejectsNoMainWindow()
{
    FileHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("new_circuit", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InternalError);
}

void TestFileHandler::testHandleNewCircuitCreatesRealTab()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    const int countBefore = tabs->count();

    const QJsonObject response = handler.handleCommand("new_circuit", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(tabs->count(), countBefore + 1);
}

void TestFileHandler::testHandleCloseCircuitRejectsNoMainWindow()
{
    FileHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("close_circuit", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InternalError);
}

void TestFileHandler::testHandleCloseCircuitRejectsNoTabs()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QCOMPARE(tabs->count(), 0);

    const QJsonObject response = handler.handleCommand("close_circuit", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::OperationFailed);
}

void TestFileHandler::testHandleCloseCircuitClosesRealTab()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    QCOMPARE(tabs->count(), 1);

    const QJsonObject response = handler.handleCommand("close_circuit", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(tabs->count(), 0);
}

void TestFileHandler::testHandleGetTabCountRejectsNoMainWindow()
{
    FileHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("get_tab_count", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InternalError);
}

void TestFileHandler::testHandleGetTabCountReturnsRealCount()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    window.createNewTab();
    window.createNewTab();

    const QJsonObject response = handler.handleCommand("get_tab_count", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["tab_count"].toInt(), 3);
}

void TestFileHandler::testHandleExportImageRejectsMissingParams()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("export_image", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestFileHandler::testHandleExportImageRejectsEmptyFilenameOrFormat()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    QCOMPARE(handler.handleCommand("export_image", {{"filename", ""}, {"format", "png"}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::InvalidParams);
    QCOMPARE(handler.handleCommand("export_image", {{"filename", "x.png"}, {"format", ""}}, 1)["error"].toObject()["code"].toInt(),
             JsonRpcError::InvalidParams);
}

void TestFileHandler::testHandleExportImageRejectsNoScene()
{
    MainWindow window;
    auto *tabs = window.findChild<QTabWidget *>();
    QVERIFY(tabs);
    emit tabs->tabCloseRequested(0);
    QVERIFY(!window.currentTab());

    FileHandler handler(&window, nullptr);
    const QJsonObject response = handler.handleCommand("export_image", {{"filename", "x.png"}, {"format", "png"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::SceneNotAvailable);
}

void TestFileHandler::testHandleExportImageRejectsUnsupportedFormat()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    addAndGate(window.currentTab()->scene());

    const QJsonObject response = handler.handleCommand("export_image", {{"filename", "x.xyz"}, {"format", "xyz"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestFileHandler::testHandleExportImageExportsRealPng()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    addAndGate(window.currentTab()->scene());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("circuit.png");

    const QJsonObject response = handler.handleCommand("export_image", {{"filename", path}, {"format", "png"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(QFile::exists(path));
    QVERIFY(QFileInfo(path).size() > 0);
}

void TestFileHandler::testHandleExportImageExportsRealSvg()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    addAndGate(window.currentTab()->scene());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("circuit.svg");

    const QJsonObject response = handler.handleCommand("export_image", {{"filename", path}, {"format", "svg"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(QFile::exists(path));
    QVERIFY(QFileInfo(path).size() > 0);
}

void TestFileHandler::testHandleExportImageExportsRealPdf()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    addAndGate(window.currentTab()->scene());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("circuit.pdf");

    const QJsonObject response = handler.handleCommand("export_image", {{"filename", path}, {"format", "pdf"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY(QFile::exists(path));
    QVERIFY(QFileInfo(path).size() > 0);
}

void TestFileHandler::testHandleExportImageRejectsSvgOpenFailure()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    addAndGate(window.currentTab()->scene());

    const QJsonObject response = handler.handleCommand("export_image", {{"filename", "/nonexistent/directory/x.svg"}, {"format", "svg"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::FileError);
}

void TestFileHandler::testHandleExportImageRejectsSvgWriteFailure()
{
    // /dev/null is genuinely openable for writing (painter.isActive() succeeds), but reports
    // size() == 0 afterward -- a real way to make the post-write existence/size check fail
    // without needing a contrived disk-full condition.
    MainWindow window;
    FileHandler handler(&window, nullptr);
    addAndGate(window.currentTab()->scene());

    const QJsonObject response = handler.handleCommand("export_image", {{"filename", "/dev/null"}, {"format", "svg"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::FileError);
}

void TestFileHandler::testHandleExportImageRejectsPdfOpenFailure()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    addAndGate(window.currentTab()->scene());

    const QJsonObject response = handler.handleCommand("export_image", {{"filename", "/nonexistent/directory/x.pdf"}, {"format", "pdf"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::FileError);
}

void TestFileHandler::testHandleExportImageRejectsPngSaveFailure()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    addAndGate(window.currentTab()->scene());

    const QJsonObject response = handler.handleCommand("export_image", {{"filename", "/nonexistent/directory/x.png"}, {"format", "png"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::FileError);
}

void TestFileHandler::testHandleExportArduinoRejectsMissingFilename()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("export_arduino", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestFileHandler::testHandleExportArduinoRejectsEmptyFilename()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("export_arduino", {{"filename", ""}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestFileHandler::testHandleExportArduinoRejectsNoMainWindow()
{
    FileHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("export_arduino", {{"filename", "x.ino"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InternalError);
}

void TestFileHandler::testHandleExportArduinoExportsRealFile()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    auto *sw = new InputSwitch();
    auto *led = new Led();
    scene->receiveCommand(new AddItemsCommand({sw, led}, scene));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("circuit.ino");

    const QJsonObject response = handler.handleCommand("export_arduino", {{"filename", path}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["format"].toString(), QStringLiteral("arduino"));
    QVERIFY(QFile::exists(path));
    QVERIFY(QFileInfo(path).size() > 0);
}

void TestFileHandler::testHandleExportSystemVerilogRejectsMissingFilename()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("export_systemverilog", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestFileHandler::testHandleExportSystemVerilogRejectsEmptyFilename()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("export_systemverilog", {{"filename", ""}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestFileHandler::testHandleExportSystemVerilogRejectsNoMainWindow()
{
    FileHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("export_systemverilog", {{"filename", "x.sv"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InternalError);
}

void TestFileHandler::testHandleExportSystemVerilogExportsRealFile()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);
    auto *scene = window.currentTab()->scene();
    auto *sw = new InputSwitch();
    auto *led = new Led();
    scene->receiveCommand(new AddItemsCommand({sw, led}, scene));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("circuit.sv");

    const QJsonObject response = handler.handleCommand("export_systemverilog", {{"filename", path}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["format"].toString(), QStringLiteral("systemverilog"));
    QVERIFY(QFile::exists(path));
    QVERIFY(QFileInfo(path).size() > 0);
}

void TestFileHandler::testHandleCommandRejectsUnknownCommand()
{
    MainWindow window;
    FileHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("totally_made_up", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}
