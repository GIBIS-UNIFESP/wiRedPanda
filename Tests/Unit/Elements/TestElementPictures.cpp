// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementPictures.h"

#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Presentation/ElementPictures.h"

namespace ElementPicturesTestDetail {

/// Writes a picture of \a size to \a path and returns whether it was written.
bool writeImage(const QString &path, const QSize size)
{
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::red);
    return image.save(path, "PNG");
}

/// Returns the size of the picture \a element shows; invalid while it shows none.
QSize shownSize(const Element &element)
{
    const PictureVariant *picture = ElementPictures::instance().shown(element);
    return picture ? picture->pixmap.size() : QSize();
}

} // namespace ElementPicturesTestDetail

void TestElementPictures::testElementsShowingOneFileShareItsPicture()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("shared.png");
    QVERIFY(ElementPicturesTestDetail::writeImage(path, QSize(80, 72)));

    InputSwitch first;
    InputSwitch second;
    first.setAppearance(false, path);
    second.setAppearance(false, path);

    QVERIFY(first.appearance().picture().has_value());
    QCOMPARE(first.appearance().picture(), second.appearance().picture());
    QCOMPARE(ElementPictures::instance().shown(first), ElementPictures::instance().shown(second));
    QCOMPARE(ElementPicturesTestDetail::shownSize(first), QSize(80, 72));
}

void TestElementPictures::testAFileChangedOnDiskIsANewVersion()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("edited.png");
    QVERIFY(ElementPicturesTestDetail::writeImage(path, QSize(64, 64)));

    InputSwitch before;
    before.setAppearance(false, path);
    QCOMPARE(ElementPicturesTestDetail::shownSize(before), QSize(64, 64));

    // The file is edited: an element that loads it now shows the new version, one that loaded it before keeps its own.
    QVERIFY(QFile::remove(path));
    QVERIFY(ElementPicturesTestDetail::writeImage(path, QSize(96, 80)));
    InputSwitch after;
    after.setAppearance(false, path);

    QVERIFY(after.appearance().picture() != before.appearance().picture());
    QCOMPARE(ElementPicturesTestDetail::shownSize(after), QSize(96, 80));
    QCOMPARE(ElementPicturesTestDetail::shownSize(before), QSize(64, 64));
}

void TestElementPictures::testVariantsAreBuiltFromTheSourceReadAtLoad()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("labelled.svg");
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(R"(<svg xmlns="http://www.w3.org/2000/svg" width="64" height="64">)"
                   R"(<rect width="64" height="64" fill="white"/><text x="8" y="40" font-size="24">AB</text></svg>)");
    }

    And gate;
    gate.setAppearance(false, path);
    QVERIFY(!ElementPictures::instance().shown(gate)->oriented);

    // The file goes before the gate turns: its labels are still counter-oriented, from the source read as it loaded.
    QVERIFY(QFile::remove(path));
    gate.setRotation(90);
    const PictureVariant *turned = ElementPictures::instance().shown(gate);
    QVERIFY(turned != nullptr);
    QVERIFY(turned->oriented);
    QVERIFY(turned->renderer != nullptr);
}
