// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementPictures.h"

#include <memory>

#include <QFile>
#include <QImage>
#include <QTemporaryDir>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInput.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Presentation/ElementPictures.h"
#include "App/Wiring/Port.h"

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

/// Returns whether \a element shows a picture of \a size, and took it as the picture changed: its body is that size and
/// it turns about the body's centre.
bool shows(const Element &element, const QSize size)
{
    return (shownSize(element) == size) && (element.body().body.size() == QSizeF(size))
        && (element.transformOriginPoint() == element.body().centre);
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

    QVERIFY(first.appearance().picture(0).has_value());
    QCOMPARE(first.appearance().picture(0), second.appearance().picture(0));
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

    QVERIFY(after.appearance().picture(0) != before.appearance().picture(0));
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

void TestElementPictures::testALevelShowsItsSlot_data()
{
    QTest::addColumn<ElementType>("type");
    QTest::newRow("InputButton") << ElementType::InputButton;
    QTest::newRow("InputSwitch") << ElementType::InputSwitch;
    QTest::newRow("Clock") << ElementType::Clock;
}

void TestElementPictures::testALevelShowsItsSlot()
{
    QFETCH(ElementType, type);
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("on.png");
    QVERIFY(ElementPicturesTestDetail::writeImage(path, QSize(96, 80)));

    const std::unique_ptr<Element> element(ElementFactory::buildElement(type));
    auto *input = qobject_cast<ElementInput *>(element.get());
    QVERIFY(input != nullptr);
    const QSize off = ElementPicturesTestDetail::shownSize(*input);
    QVERIFY(off.isValid());

    // The on picture is custom: it shows while the level is on, and the off picture while it is off.
    input->setOn(true);
    input->setAppearance(false, path);
    QVERIFY(ElementPicturesTestDetail::shows(*input, QSize(96, 80)));
    input->setOn(false);
    QVERIFY(ElementPicturesTestDetail::shows(*input, off));
    input->setOn(true);
    QVERIFY(ElementPicturesTestDetail::shows(*input, QSize(96, 80)));
}

void TestElementPictures::testAnAudioOutputShowsWhetherItPlays_data()
{
    QTest::addColumn<ElementType>("type");
    QTest::newRow("Buzzer") << ElementType::Buzzer;
    QTest::newRow("AudioBox") << ElementType::AudioBox;
}

void TestElementPictures::testAnAudioOutputShowsWhetherItPlays()
{
    QFETCH(ElementType, type);
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("playing.png");
    QVERIFY(ElementPicturesTestDetail::writeImage(path, QSize(96, 80)));

    const std::unique_ptr<Element> element(ElementFactory::buildElement(type));
    const QSize silent = ElementPicturesTestDetail::shownSize(*element);
    QVERIFY(silent.isValid());
    element->setAppearanceAt(1, path);

    QVERIFY(ElementPicturesTestDetail::shows(*element, silent));
    QVERIFY(element->setParam(ParamId::Playing, true));
    QVERIFY(ElementPicturesTestDetail::shows(*element, QSize(96, 80)));
    QVERIFY(element->setParam(ParamId::Playing, false));
    QVERIFY(ElementPicturesTestDetail::shows(*element, silent));
}

void TestElementPictures::testAnLedShowsTheSlotLitAtItsLastRefresh()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("lit.png");
    QVERIFY(ElementPicturesTestDetail::writeImage(path, QSize(96, 80)));

    Led led;
    const QSize off = ElementPicturesTestDetail::shownSize(led);
    QVERIFY(off.isValid());

    // White lit (slot 1) is custom.
    led.inputPort()->setStatus(Status::Active);
    led.setAppearance(false, path);
    QVERIFY(ElementPicturesTestDetail::shows(led, QSize(96, 80)));

    // The LED shows its input going off, then on again, as it refreshes.
    led.inputPort()->setStatus(Status::Inactive);
    QVERIFY(ElementPicturesTestDetail::shows(led, QSize(96, 80)));
    led.refresh();
    QVERIFY(ElementPicturesTestDetail::shows(led, off));
    led.inputPort()->setStatus(Status::Active);
    QVERIFY(ElementPicturesTestDetail::shows(led, off));
    led.refresh();
    QVERIFY(ElementPicturesTestDetail::shows(led, QSize(96, 80)));
}
