// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementPictures.h"

#include <memory>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QScopeGuard>
#include <QSet>
#include <QSvgRenderer>
#include <QTemporaryDir>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInput.h"
#include "App/Element/ElementTypeDescriptor.h"
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
    const PictureVariant *picture = ElementPictures::instance().shown(element, Theme::Light);
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
    QCOMPARE(ElementPictures::instance().shown(first, Theme::Light), ElementPictures::instance().shown(second, Theme::Light));
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
    QVERIFY(!ElementPictures::instance().shown(gate, Theme::Light)->oriented);

    // The file goes before the gate turns: its labels are still counter-oriented, from the source read as it loaded.
    QVERIFY(QFile::remove(path));
    gate.setRotation(90);
    const PictureVariant *turned = ElementPictures::instance().shown(gate, Theme::Light);
    QVERIFY(turned != nullptr);
    QVERIFY(turned->oriented);
    QVERIFY(turned->renderer != nullptr);
}

void TestElementPictures::testTheVariantsShownLatelyStayWithinTheBudget()
{
    // An angle can be any number: the variants of every picture stay within the budget, each charged its own raster and
    // its renderer, those shown least recently going first; a dropped one is built again when its orientation is shown.
    // A picture of 512 by 512 makes each oriented raster a megabyte, so sixty-four angles are four times the budget.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("turned.svg");
    QByteArray source = R"(<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512">)"
                        R"(<rect width="512" height="512" fill="white"/><text x="8" y="40" font-size="24">AB</text></svg>)";
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(source);
    }

    And gate;
    gate.setAppearance(false, path);
    const PictureId id = *gate.appearance().picture(0);
    const qsizetype before = ElementPictures::variantBytes(id);
    gate.setRotation(1);
    QVERIFY(ElementPictures::instance().shown(gate, Theme::Light)->oriented);
    QCOMPARE(ElementPictures::variantBytes(id) - before, (512 * 512 * 4) + (source.size() * 8 / 3));
    // Held here, the first angle's renderer outlives its variant: one built again has another
    const std::shared_ptr<QSvgRenderer> first = ElementPictures::instance().shown(gate, Theme::Light)->renderer;

    for (int angle = 2; angle <= 64; ++angle) {
        gate.setRotation(angle);
        QVERIFY(ElementPictures::instance().shown(gate, Theme::Light)->oriented);
        QVERIFY2(ElementPictures::variantBytes() <= ElementPictures::variantBudget, qPrintable(QString::number(angle)));
    }
    QVERIFY(ElementPictures::variantCount(id) < 64);
    QVERIFY(ElementPictures::variantBytes(id) > ElementPictures::variantBudget / 2);

    // The last angle shown is kept: shown again, it is the same variant
    const std::shared_ptr<QSvgRenderer> last = ElementPictures::instance().shown(gate, Theme::Light)->renderer;
    gate.setRotation(63);
    gate.setRotation(64);
    QCOMPARE(ElementPictures::instance().shown(gate, Theme::Light)->renderer, last);

    // The first angle was dropped long ago: shown again, it is built again, oriented as before
    const qsizetype kept = ElementPictures::variantCount(id);
    gate.setRotation(1);
    const PictureVariant *again = ElementPictures::instance().shown(gate, Theme::Light);
    QVERIFY(again->oriented);
    QVERIFY(again->renderer != nullptr);
    QVERIFY(again->renderer != first);
    QCOMPARE(ElementPictures::variantCount(id), kept);
    QVERIFY(ElementPictures::variantBytes() <= ElementPictures::variantBudget);
}

void TestElementPictures::testAPictureWithoutLabelsHasOneVariant()
{
    // A picture without labels is the same in every orientation, the element's transform turning it whole: every
    // orientation shows the one variant, its renderer parsed once
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("plain.svg");
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(R"(<svg xmlns="http://www.w3.org/2000/svg" width="64" height="64">)"
                   R"(<rect width="64" height="64" fill="white"/><circle cx="32" cy="32" r="16"/></svg>)");
    }

    And gate;
    gate.setAppearance(false, path);
    const PictureId id = *gate.appearance().picture(0);
    const std::shared_ptr<QSvgRenderer> renderer = ElementPictures::instance().shown(gate, Theme::Light)->renderer;
    QVERIFY(renderer != nullptr);
    for (const qreal angle : {0.0, 37.0, 90.0, 180.0}) {
        gate.setRotation(angle);
        gate.setFlippedX(angle > 100);
        const PictureVariant *shown = ElementPictures::instance().shown(gate, Theme::Light);
        QVERIFY(!shown->oriented);
        QCOMPARE(shown->renderer, renderer);
    }
    QCOMPARE(ElementPictures::variantCount(id), 1);
}

void TestElementPictures::testTheBundledPicturesFitTheBudget()
{
    // Every bundled picture in the editor's sixteen orientations (four turns, each flipped either way or both), in both
    // themes, is kept at once: the budget drops only what MCP's other angles and custom pictures add
    QList<PictureId> ids;
    QSet<PictureId> labelled;
    QDirIterator it(QStringLiteral(":/Components"), {QStringLiteral("*.svg"), QStringLiteral("*.png")}, QDir::Files,
                    QDirIterator::Subdirectories);
    And gate;
    while (it.hasNext()) {
        gate.setAppearance(false, it.next());
        const std::optional<PictureId> id = gate.appearance().picture(0);
        QVERIFY(id.has_value());
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            for (const qreal angle : {0.0, 90.0, 180.0, 270.0}) {
                for (const auto &[flipX, flipY] : {std::pair(false, false), std::pair(true, false),
                                                   std::pair(false, true), std::pair(true, true)}) {
                    gate.setRotation(angle);
                    gate.setFlippedX(flipX);
                    gate.setFlippedY(flipY);
                    const PictureVariant *shown = ElementPictures::instance().shown(gate, theme);
                    QVERIFY(shown != nullptr);
                    if (shown->oriented && (theme == Theme::Light)) {
                        labelled.insert(*id);
                    }
                }
            }
        }
        gate.setRotation(0);
        gate.setFlippedX(false);
        gate.setFlippedY(false);
        ids.append(*id);
    }
    QVERIFY(ids.size() > 50);
    QVERIFY(!labelled.isEmpty());

    qsizetype bytes = 0;
    for (const PictureId id : std::as_const(ids)) {
        QCOMPARE(ElementPictures::variantCount(id), labelled.contains(id) ? 16 : 1);
        bytes += ElementPictures::variantBytes(id);
    }
    qInfo("%s", qPrintable(QStringLiteral("the %1 bundled pictures take %2 bytes in the editor's orientations")
                               .arg(ids.size())
                               .arg(bytes)));
    QVERIFY(bytes <= ElementPictures::variantBudget);
}

void TestElementPictures::testASourceThatIsNotSvgShowsItsRaster()
{
    // A file named .svg that QPixmap reads by its content (a PNG) but QSvgRenderer cannot parse: holding "<text>" (in a
    // PNG text chunk), it reaches the label correction too. Each orientation falls back to the raster, unoriented, with
    // no vector renderer, and draws it.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("not-svg.svg");
    QImage image(QSize(64, 64), QImage::Format_ARGB32);
    image.fill(Qt::red);
    image.setText(QStringLiteral("Comment"), QStringLiteral("<text>A</text>"));
    QVERIFY(image.save(path, "PNG"));
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QVERIFY(file.readAll().contains("<text>"));
    }

    And gate;
    gate.setAppearance(false, path);
    QVERIFY(gate.appearance().picture(0).has_value());
    for (const qreal angle : {0.0, 37.0, 90.0}) {
        gate.setRotation(angle);
        const PictureVariant *shown = ElementPictures::instance().shown(gate, Theme::Light);
        QVERIFY(shown != nullptr);
        QVERIFY2(shown->renderer == nullptr, qPrintable(QString::number(angle)));
        QVERIFY2(!shown->oriented, qPrintable(QString::number(angle)));
        QCOMPARE(shown->pixmap.size(), QSize(64, 64));

        QPixmap canvas(64, 64);
        canvas.fill(Qt::transparent);
        QPainter painter(&canvas);
        ElementPictures::draw(painter, *shown);
        painter.end();
        QCOMPARE(canvas.toImage().pixelColor(32, 32), QColor(Qt::red));
    }
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

void TestElementPictures::testThemedPicturesMatchInSizeAndLabels()
{
    // A body is measured on the Light picture its element names and drawn in either theme's, and keyed by whether the
    // Light one has labels to counter-orient: every built-in picture with a Dark sibling is the sibling's size, and
    // has labels if the sibling has
    int pairs = 0;
    QDirIterator resources(QStringLiteral(":/"), QDir::Files, QDirIterator::Subdirectories);
    while (resources.hasNext()) {
        const QString light = resources.next();
        const QString dark = ElementPictures::themed(light, Theme::Dark);
        if (dark == light) {
            continue;
        }
        ++pairs;
        const QSize lightSize = QPixmap(light).size();
        QVERIFY2(lightSize.isValid(), qPrintable(light));
        QVERIFY2(QPixmap(dark).size() == lightSize, qPrintable(QStringLiteral("%1 and %2").arg(light, dark)));
        const std::optional<PictureId> lightId = ElementPictures::instance().load(light);
        const std::optional<PictureId> darkId = ElementPictures::instance().load(dark);
        QVERIFY2(lightId && darkId, qPrintable(QStringLiteral("%1 and %2").arg(light, dark)));
        QVERIFY2(ElementPictures::isLabelled(*lightId) == ElementPictures::isLabelled(*darkId),
                 qPrintable(QStringLiteral("%1 and %2").arg(light, dark)));
    }
    // The six latches and flip-flops, at least
    QVERIFY2(pairs >= 6, qPrintable(QString::number(pairs)));
}

void TestElementPictures::testOnlyALightResourceIsThemed()
{
    const QString light = QStringLiteral(":/Components/Memory/Light/D-latch.svg");
    const QString dark = QStringLiteral(":/Components/Memory/Dark/D-latch.svg");
    QCOMPARE(ElementPictures::themed(light, Theme::Dark), dark);
    QCOMPARE(ElementPictures::themed(light, Theme::Light), light);
    QCOMPARE(ElementPictures::themed(light, Theme::System), light);

    // A picture with no sibling, and a file on disk in a Light folder, are every theme's
    const QString gate = QStringLiteral(":/Components/Logic/and.svg");
    QCOMPARE(ElementPictures::themed(gate, Theme::Dark), gate);
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("Light")) && QDir(dir.path()).mkpath(QStringLiteral("Dark")));
    const QString onDisk = dir.filePath(QStringLiteral("Light/custom.png"));
    QVERIFY(ElementPicturesTestDetail::writeImage(onDisk, QSize(64, 64)));
    QVERIFY(ElementPicturesTestDetail::writeImage(dir.filePath(QStringLiteral("Dark/custom.png")), QSize(64, 64)));
    QCOMPARE(ElementPictures::themed(onDisk, Theme::Dark), onDisk);

    // An element showing either is shown the same in both themes
    InputSwitch custom;
    custom.setAppearance(false, onDisk);
    QCOMPARE(ElementPictures::instance().shown(custom, Theme::Dark), ElementPictures::instance().shown(custom, Theme::Light));
}

void TestElementPictures::testAPreviewIsTheThemesPicture()
{
    // A picture named by its Light resource previews as the theme handed shows it, as an element draws it
    const QString light = QStringLiteral(":/Components/Memory/Light/D-flipflop.svg");
    const QString dark = QStringLiteral(":/Components/Memory/Dark/D-flipflop.svg");
    const QSize size(24, 24);
    const QImage inDark = ElementPictures::preview(light, Theme::Dark, size).toImage();
    QVERIFY(!inDark.isNull());
    QCOMPARE(inDark, ElementPictures::preview(dark, Theme::Light, size).toImage());
    QVERIFY(inDark != ElementPictures::preview(light, Theme::Light, size).toImage());
}

void TestElementPictures::testTypePictureIsTheCurrentThemes()
{
    const Theme before = ThemeManager::theme();
    const auto restore = qScopeGuard([before] { ThemeManager::setTheme(before); });

    QVERIFY(ElementPictures::typePicture(ElementType::Unknown).isNull());
    QVERIFY(ElementPictures::typePicture(ElementType::IC).isNull());

    const QString light = ElementTypeDescriptor::of(ElementType::DFlipFlop).pixmapPath;
    ThemeManager::setTheme(Theme::Dark);
    QCOMPARE(ElementPictures::typePicture(ElementType::DFlipFlop).toImage(),
             QPixmap(ElementPictures::themed(light, Theme::Dark)).toImage());
    ThemeManager::setTheme(Theme::Light);
    QCOMPARE(ElementPictures::typePicture(ElementType::DFlipFlop).toImage(), QPixmap(light).toImage());
}
