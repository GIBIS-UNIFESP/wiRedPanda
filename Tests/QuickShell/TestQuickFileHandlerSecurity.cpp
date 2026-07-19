// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestQuickFileHandlerSecurity.h"

#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include "App/Element/ElementFactory.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/UI/CircuitExporter.h"
#include "MCP/Server/Handlers/QuickFileHandler.h"

void TestQuickFileHandlerSecurity::testExportImagePngClampsExtremeSceneDimensions()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas->addItem(sw);
    sw->setPos(1e9, 1e9);

    QuickFileHandler handler(&controller, nullptr);

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

void TestQuickFileHandlerSecurity::testExportImageClampsExcessivePadding()
{
    QuickAppController controller;
    controller.newTab();
    auto *canvas = controller.currentTab()->canvas();
    canvas->addItem(ElementFactory::buildElement(ElementType::InputSwitch));

    QuickFileHandler handler(&controller, nullptr);

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    const QString path = tmpDir.path() + "/padded.png";

    // Pre-fix (FileHandler.cpp's own history), this unclamped padding value was applied
    // directly to the content rect before sizing the output buffer -- a client could blow up
    // even a trivial circuit through padding alone. QuickFileHandler ports the same clamp
    // (kMaxExportPadding = 2000).
    const QJsonObject params{{"filename", path}, {"format", "png"}, {"padding", 2000000000}};
    const QJsonObject response = handler.handleCommand("export_image", params, {});
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));

    QImage image(path);
    QVERIFY(!image.isNull());
    QVERIFY(image.width() <= CircuitExporter::kMaxImageDimension);
    QVERIFY(image.height() <= CircuitExporter::kMaxImageDimension);
}
