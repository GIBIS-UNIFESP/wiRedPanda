// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestFileHandlerSecurity.h"

#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/CircuitExporter.h"
#include "App/UI/MainWindow.h"
#include "MCP/Server/Handlers/FileHandler.h"

void TestFileHandlerSecurity::testExportImagePngClampsExtremeSceneDimensions()
{
    MainWindow window;
    auto *sw = new InputSwitch();
    window.currentTab()->scene()->addItem(sw);
    sw->setPos(1e9, 1e9);

    FileHandler handler(&window, nullptr);

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.path() + "/extreme.png";

    const QJsonObject params{{"filename", path}, {"format", "png"}};
    const QJsonObject response = handler.handleCommand("export_image", params, {});
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));

    QImage image(path);
    QVERIFY(!image.isNull());
    QVERIFY(image.width() <= CircuitExporter::kMaxImageDimension);
    QVERIFY(image.height() <= CircuitExporter::kMaxImageDimension);
}

void TestFileHandlerSecurity::testExportImageClampsExcessivePadding()
{
    MainWindow window;
    window.currentTab()->scene()->addItem(new InputSwitch());

    FileHandler handler(&window, nullptr);

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.path() + "/padded.png";

    // Pre-fix, this unclamped padding value was applied directly to sceneRect before sizing
    // the output buffer — a client could blow up even a trivial circuit through padding alone.
    const QJsonObject params{{"filename", path}, {"format", "png"}, {"padding", 2000000000}};
    const QJsonObject response = handler.handleCommand("export_image", params, {});
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));

    QImage image(path);
    QVERIFY(!image.isNull());
    QVERIFY(image.width() <= CircuitExporter::kMaxImageDimension);
    QVERIFY(image.height() <= CircuitExporter::kMaxImageDimension);
}
