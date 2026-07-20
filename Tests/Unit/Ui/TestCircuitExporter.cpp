// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestCircuitExporter.h"

#include <QFile>
#include <QImage>
#include <QTemporaryDir>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Scene/Workspace.h"
#include "App/UI/CircuitExporter.h"
#include "Tests/Common/TestUtils.h"

void TestCircuitExporter::testExporterCreation()
{
    // CircuitExporter is a namespace — test that renderToImage doesn't crash on empty scene
    WorkSpace workspace;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString path = dir.path() + "/test.png";
    CircuitExporter::renderToImage(workspace.scene(), path);
    QVERIFY(true);
}

void TestCircuitExporter::testRenderToImageThrowsOnInvalidPath()
{
    // Path lives under a directory that doesn't exist — pixmap.save returns false
    // and pre-fix that failure was silently swallowed. The throw is the regression.
    WorkSpace workspace;
    const QString path = "/nonexistent/directory/that/does/not/exist/img.png";
    QVERIFY_THROWS(std::exception, CircuitExporter::renderToImage(workspace.scene(), path));
}

void TestCircuitExporter::testRenderToImagePaddingIsTransparentNotGarbage()
{
    // Deliberately no updateTheme() call — Scene's background BRUSH is only set by
    // updateTheme(), so an un-themed scene's padding pixels come straight from the image's
    // initial fill wherever the (always-drawn, theme-independent) grid dots don't land.
    // Pixel (8, 8) sits at scene coordinate (rect.left() + 8, rect.top() + 8): since the
    // empty scene's padded rect starts at a multiple of Constants::gridSize (16), (0, 0)
    // lands exactly on a grid dot, but the 8 px offset falls squarely between dots.
    WorkSpace workspace;
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/test.png";
    CircuitExporter::renderToImage(workspace.scene(), path);

    QImage image(path);
    QVERIFY(!image.isNull());
    QCOMPARE(qAlpha(image.pixel(8, 8)), 0);
}

void TestCircuitExporter::testRenderToImageClampsExtremeSceneDimensions()
{
    WorkSpace workspace;
    auto *sw = new InputSwitch();
    workspace.scene()->addItem(sw);
    sw->setPos(1e7, 1e7);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/extreme.png";
    CircuitExporter::renderToImage(workspace.scene(), path);

    QImage image(path);
    QVERIFY(!image.isNull());
    QVERIFY(image.width() <= CircuitExporter::kMaxImageDimension);
    QVERIFY(image.height() <= CircuitExporter::kMaxImageDimension);
}

void TestCircuitExporter::testRenderToPdfThrowsOnInvalidPath()
{
    // Mirrors testRenderToImageThrowsOnInvalidPath(), but for the PDF path: QPainter::begin()
    // itself fails (rather than a later save() call) when the printer can't open a destination
    // file under a directory that doesn't exist.
    WorkSpace workspace;
    const QString path = "/nonexistent/directory/that/does/not/exist/circuit.pdf";
    QVERIFY_THROWS(std::exception, CircuitExporter::renderToPdf(workspace.scene(), path));
}

void TestCircuitExporter::testRenderScaledImageThrowsOnEmptyPaddedRect()
{
    // An empty paddedRect produces a 0x0 target QImage, which QPainter::begin() rejects as a
    // null paint device -- the one other real trigger of renderScaledImage()'s throw, distinct
    // from the file I/O failure covered by the PDF/PNG path tests above.
    WorkSpace workspace;
    QVERIFY_THROWS(std::exception, CircuitExporter::renderScaledImage(workspace.scene(), QRectF()));
}

void TestCircuitExporter::testRenderToPdfHandlesExtremeSceneDimensionsPromptly()
{
    WorkSpace workspace;
    auto *sw = new InputSwitch();
    workspace.scene()->addItem(sw);
    sw->setPos(1e7, 1e7);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + "/extreme.pdf";
    CircuitExporter::renderToPdf(workspace.scene(), path);
    QVERIFY(QFile::exists(path));
}

void TestCircuitExporter::testRenderScaledImageClampsExtremeDimensionsDirectly()
{
    WorkSpace workspace;
    auto *sw = new InputSwitch();
    workspace.scene()->addItem(sw);
    sw->setPos(1e7, 1e7);

    const QRectF paddedRect = workspace.scene()->itemsBoundingRect().adjusted(-64, -64, 64, 64);
    const QImage image = CircuitExporter::renderScaledImage(workspace.scene(), paddedRect);

    QVERIFY(!image.isNull());
    QVERIFY(image.width() <= CircuitExporter::kMaxImageDimension);
    QVERIFY(image.height() <= CircuitExporter::kMaxImageDimension);
}
