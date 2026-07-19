// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementAppearance.h"

#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QTemporaryFile>
#include <QTest>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
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

void TestElementAppearance::testSetPixmapNonExistentFileThrows()
{
    InputSwitch sw;

    bool threw = false;
    try {
        sw.setAppearanceAt(0, "/nonexistent/directory/nope.svg");
    } catch (const Pandaception &e) {
        threw = true;
        QVERIFY(QString::fromUtf8(e.what()).contains("does not exist"));
    }
    QVERIFY2(threw, "setAppearanceAt() with a nonexistent path must throw");
}

void TestElementAppearance::testSetPixmapUnreadableFileThrows()
{
    InputSwitch sw;

    // A real, readable, existing file whose content QPixmap::load() can't parse as any known
    // image format -- exists() and isReadable() are both true, so the failure reason falls
    // through to "Unknown reason" rather than either of the other two branches.
    QTemporaryFile tempFile("XXXXXX.png");
    QVERIFY(tempFile.open());
    tempFile.write("this is not a valid image file at all");
    tempFile.close();

    bool threw = false;
    try {
        sw.setAppearanceAt(0, tempFile.fileName());
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY2(threw, "setAppearanceAt() with unparseable image content must throw");
}

void TestElementAppearance::testRotatedMalformedSvgFallsBackToBasePixmap()
{
    // orientedSvgPixmap()/sharedSvgRenderer() each open the resolved path a second time,
    // independently of setPixmap()'s own successful QPixmap::load() -- reachable if the file is
    // removed between the initial load and a later re-orientation (rotate/flip), which calls
    // applyOrientation() directly rather than going back through setPixmap()'s own existence check.
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
        path = writeTempSvg(tempFile, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\"><text id=\"t\">A</text></svg>");
        gate.setAppearanceAt(0, path);
    }

    QVERIFY(QFile::remove(path));

    // A fresh rotation angle is a cache miss in both orientedSvgPixmap()'s QPixmapCache and
    // sharedSvgRenderer()'s own QHash cache, forcing both to re-open the (now-deleted) file.
    // setRotation() itself triggers reapplyAppearanceOrientation() for a rotatable element.
    gate.setRotation(37);

    // Must fall back silently (no crash, no exception) rather than propagate the failed re-open.
    QPixmap canvas(64, 64);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    QStyleOptionGraphicsItem option;
    gate.paint(&painter, &option, nullptr);
    painter.end();
}

void TestElementAppearance::testRotatedMalformedSvgFallsBackToBasePixmapContent()
{
    // Same reasoning as the deleted-file case above, but for the "file exists and is readable,
    // yet isn't parseable SVG" failure branches in orientSvgTextNodes() (its own internal probe),
    // orientedSvgPixmap(), and sharedSvgRenderer() -- all three construct their own QSvgRenderer
    // on the same garbage content and find it invalid. The content deliberately contains the
    // literal substring "<text" so both orientedSvgPixmap() and sharedSvgRenderer() (which only
    // route through orientSvgTextNodes() when that substring is present) actually reach it.
    And gate;
    QTemporaryFile validFile("XXXXXX.svg");
    const QString path = writeTempSvg(validFile, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"64\" height=\"64\"><text id=\"t\">A</text></svg>");
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
}
