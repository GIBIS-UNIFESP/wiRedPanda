// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementAppearance.h"

#include <memory>

#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

namespace {
QString writeTempSvg(QTemporaryFile &file, const QString &body = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\"/>")
{
    [[maybe_unused]] const bool opened = file.open();
    file.write(body.toUtf8());
    file.close();
    return file.fileName();
}
} // namespace

void TestElementAppearance::testSetAppearanceAtOutOfBoundsIsNoOp()
{
    InputSwitch sw;
    QVERIFY(sw.externalFiles().isEmpty());

    sw.setAppearanceAt(-1, "/some/path.svg");
    sw.setAppearanceAt(99, "/some/path.svg");

    QVERIFY2(sw.externalFiles().isEmpty(), "out-of-bounds setAppearanceAt() must not apply anything");
}

void TestElementAppearance::testSetAppearanceAtEmptyFileNameFallsBackToDefault()
{
    InputSwitch sw;
    QTemporaryFile tempFile("XXXXXX.svg");
    const QString path = writeTempSvg(tempFile);

    sw.setAppearanceAt(0, path);
    QCOMPARE(sw.externalFiles(), QStringList{path});

    sw.setAppearanceAt(0, "");
    QVERIFY2(sw.externalFiles().isEmpty(), "an empty fileName must reset that slot back to its default");
}

void TestElementAppearance::testSetAppearanceAtCustomFileName()
{
    InputSwitch sw;
    QTemporaryFile tempFile("XXXXXX.svg");
    const QString path = writeTempSvg(tempFile);

    sw.setAppearanceAt(1, path);
    QCOMPARE(sw.externalFiles(), QStringList{path});
}

void TestElementAppearance::testExternalFilesReflectsCustomAppearance()
{
    InputSwitch sw;
    QVERIFY(sw.externalFiles().isEmpty());

    QTemporaryFile tempFile("XXXXXX.svg");
    const QString path = writeTempSvg(tempFile);
    sw.setAppearanceAt(0, path);

    QCOMPARE(sw.externalFiles(), QStringList{path});
}

void TestElementAppearance::testRenderRasterAppearance()
{
    InputSwitch sw;

    QTemporaryFile tempFile("XXXXXX.png");
    QVERIFY(tempFile.open());
    QPixmap raster(8, 8);
    raster.fill(Qt::red);
    QVERIFY(raster.save(tempFile.fileName(), "PNG"));
    tempFile.close();

    sw.setAppearanceAt(1, tempFile.fileName());
    sw.setOn(true); // switches to appearance index 1, the raster one just installed

    QPixmap canvas(64, 64);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    QStyleOptionGraphicsItem option;
    sw.paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(canvas), "paint() must draw the raster (non-SVG) appearance");
}

void TestElementAppearance::testMissingPictureShowsItsSlotsDefault()
{
    // The switch turns on with a custom on picture that does not exist: it draws the on default, not the off one, and
    // the slot keeps its path
    InputSwitch sw;
    sw.setAppearanceAt(1, "/nonexistent/directory/nope.svg");
    sw.setOn(true);

    InputSwitch fresh;
    fresh.setOn(true);
    QCOMPARE(SnapshotAccess::pixmapIdentity(&sw), SnapshotAccess::pixmapIdentity(&fresh));
    QCOMPARE(sw.externalFiles(), QStringList{"/nonexistent/directory/nope.svg"});
}

void TestElementAppearance::testUndecodablePictureShowsItsSlotsDefault()
{
    // A real, readable file whose content QPixmap::load() cannot decode
    QTemporaryFile tempFile("XXXXXX.png");
    QVERIFY(tempFile.open());
    tempFile.write("this is not a valid image file at all");
    tempFile.close();

    InputSwitch sw;
    sw.setOn(true);
    sw.setAppearanceAt(1, tempFile.fileName());

    InputSwitch fresh;
    fresh.setOn(true);
    QCOMPARE(SnapshotAccess::pixmapIdentity(&sw), SnapshotAccess::pixmapIdentity(&fresh));
    QCOMPARE(sw.externalFiles(), QStringList{tempFile.fileName()});
}

void TestElementAppearance::testPictureOfAnotherSizeKeepsPortsWhateverTheOrder_data()
{
    // A gate turns its whole item, so a flip is a transform about the centre; an LED keeps its picture upright and
    // turns only its ports about the centre.
    QTest::addColumn<ElementType>("type");
    QTest::newRow("And") << ElementType::And;
    QTest::newRow("Led") << ElementType::Led;
}

void TestElementAppearance::testPictureOfAnotherSizeKeepsPortsWhateverTheOrder()
{
    QFETCH(ElementType, type);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("wide.png");
    QImage picture(96, 48, QImage::Format_ARGB32);
    picture.fill(QColor(40, 120, 200));
    QVERIFY(picture.save(path, "PNG"));

    const auto portScenePositions = [](const Element *element) {
        QStringList positions;
        for (auto *port : element->allPorts()) {
            const QPointF pos = port->scenePos();
            positions << QString("%1,%2").arg(pos.x()).arg(pos.y());
        }
        return positions.join(' ');
    };

    for (const qreal rotation : {0.0, 90.0, 180.0, 270.0}) {
        for (const bool flipX : {false, true}) {
            for (const bool flipY : {false, true}) {
                const std::unique_ptr<Element> pictureFirst(ElementFactory::buildElement(type));
                pictureFirst->setAppearance(false, path);
                pictureFirst->setRotation(rotation);
                pictureFirst->setFlippedX(flipX);
                pictureFirst->setFlippedY(flipY);

                const std::unique_ptr<Element> orientedFirst(ElementFactory::buildElement(type));
                orientedFirst->setRotation(rotation);
                orientedFirst->setFlippedX(flipX);
                orientedFirst->setFlippedY(flipY);
                orientedFirst->setAppearance(false, path);

                const QString expected = portScenePositions(pictureFirst.get());
                const QString actual = portScenePositions(orientedFirst.get());
                QVERIFY2(actual == expected, qPrintable(QString("rot=%1 flipX=%2 flipY=%3\n  picture first:  %4\n  oriented first: %5")
                                                            .arg(rotation)
                                                            .arg(flipX)
                                                            .arg(flipY)
                                                            .arg(expected, actual)));
            }
        }
    }
}

void TestElementAppearance::testRotatedMalformedSvgFallsBackToBasePixmap()
{
    // orientedSvgPixmap()/sharedSvgRenderer() each open the resolved path a second time,
    // independently of appearanceChanged()'s own successful load -- reachable if the file is
    // removed between the initial load and a later re-orientation (rotate/flip), which calls
    // applyOrientation() directly rather than going back through appearanceChanged()'s own load.
    // Uses And (rotatesGraphic() defaults true), not InputSwitch (a static input, rotatesGraphic()
    // false) -- ElementOrientation::setRotation() only calls reapplyAppearanceOrientation() for
    // graphics that actually rotate; non-rotatable elements only reposition their ports instead.
    And gate;
    QString path;
    {
        // QTemporaryFile::close() doesn't release its underlying OS handle (by design, so the
        // object can still manage/auto-remove the file later) -- only destroying the object
        // does. On Windows that leftover handle would make the QFile::remove() below fail with
        // a sharing violation, so scope tempFile out (with auto-remove disabled, since removal
        // is done explicitly next) before removing it ourselves.
        QTemporaryFile tempFile("XXXXXX.svg");
        tempFile.setAutoRemove(false);
        path = writeTempSvg(tempFile, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\"><rect width=\"64\" height=\"64\" fill=\"black\"/><text id=\"t\">A</text></svg>");
        gate.setAppearanceAt(0, path);
    }

    QVERIFY(QFile::remove(path));

    // A fresh rotation angle is a cache miss in both orientedSvgPixmap()'s QPixmapCache and
    // sharedSvgRenderer()'s own QHash cache, forcing both to re-open the (now-deleted) file.
    // setRotation() itself triggers reapplyAppearanceOrientation() for a rotatable element.
    gate.setRotation(37);

    // Must fall back to the already-loaded base pixmap (silently, no exception) rather than
    // propagate the failed re-open -- verified below by real pixel content, not just survival.
    QPixmap canvas(64, 64);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    QStyleOptionGraphicsItem option;
    gate.paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(canvas),
              "paint() must have drawn the base pixmap fallback, not left the canvas blank");
}

void TestElementAppearance::testRotatedMalformedSvgFallsBackToBasePixmapContent()
{
    // Same reasoning as the deleted-file case above, but for the "file exists and is readable,
    // yet isn't parseable SVG" failure branches in orientSvgLabels() (its own internal probe),
    // orientedSvgPixmap(), and sharedSvgRenderer() -- all three construct their own QSvgRenderer
    // on the same garbage content and find it invalid. The content deliberately opens a
    // <text> tag so both orientedSvgPixmap() and sharedSvgRenderer() (which only route through
    // orientSvgLabels() when the picture holds a label) actually reach it.
    And gate;
    QTemporaryFile validFile("XXXXXX.svg");
    const QString path = writeTempSvg(validFile, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\"><rect width=\"64\" height=\"64\" fill=\"black\"/><text id=\"t\">A</text></svg>");
    gate.setAppearanceAt(0, path);

    // Overwrite the same (still-existing, still-readable) path with unparseable content.
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write("<text this is not valid svg/xml at all >>>");
    file.close();

    gate.setRotation(53); // fresh angle -> cache miss -> re-parses the now-malformed content

    QPixmap canvas(64, 64);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    QStyleOptionGraphicsItem option;
    gate.paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(canvas),
              "paint() must have drawn the base pixmap fallback, not left the canvas blank");
}
