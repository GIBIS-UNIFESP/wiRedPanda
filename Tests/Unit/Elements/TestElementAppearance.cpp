// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementAppearance.h"

#include <memory>

#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>

#include "App/Element/ElementFactory.h"
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
    Element sw{ElementType::InputSwitch};
    QVERIFY(sw.externalFiles().isEmpty());

    sw.setAppearanceAt(-1, "/some/path.svg");
    sw.setAppearanceAt(99, "/some/path.svg");

    QVERIFY2(sw.externalFiles().isEmpty(), "out-of-bounds setAppearanceAt() must not apply anything");
}

void TestElementAppearance::testSetAppearanceAtEmptyFileNameFallsBackToDefault()
{
    Element sw{ElementType::InputSwitch};
    QTemporaryFile tempFile("XXXXXX.svg");
    const QString path = writeTempSvg(tempFile);

    sw.setAppearanceAt(0, path);
    QCOMPARE(sw.externalFiles(), QStringList{path});

    sw.setAppearanceAt(0, "");
    QVERIFY2(sw.externalFiles().isEmpty(), "an empty fileName must reset that slot back to its default");
}

void TestElementAppearance::testSetAppearanceAtCustomFileName()
{
    Element sw{ElementType::InputSwitch};
    QTemporaryFile tempFile("XXXXXX.svg");
    const QString path = writeTempSvg(tempFile);

    sw.setAppearanceAt(1, path);
    QCOMPARE(sw.externalFiles(), QStringList{path});
}

void TestElementAppearance::testExternalFilesReflectsCustomAppearance()
{
    Element sw{ElementType::InputSwitch};
    QVERIFY(sw.externalFiles().isEmpty());

    QTemporaryFile tempFile("XXXXXX.svg");
    const QString path = writeTempSvg(tempFile);
    sw.setAppearanceAt(0, path);

    QCOMPARE(sw.externalFiles(), QStringList{path});
}

void TestElementAppearance::testRenderRasterAppearance()
{
    Element sw{ElementType::InputSwitch};

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
    Element sw{ElementType::InputSwitch};
    sw.setAppearanceAt(1, "/nonexistent/directory/nope.svg");
    sw.setOn(true);

    Element fresh{ElementType::InputSwitch};
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

    Element sw{ElementType::InputSwitch};
    sw.setOn(true);
    sw.setAppearanceAt(1, tempFile.fileName());

    Element fresh{ElementType::InputSwitch};
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
