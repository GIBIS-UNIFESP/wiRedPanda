// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementPainter.h"

#include <cstdlib>
#include <optional>

#include <QImage>
#include <QPainter>
#include <QScopeGuard>
#include <QStyleOptionGraphicsItem>
#include <QTest>

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Presentation/ElementPainter.h"

namespace ElementPainterTestDetail {

/// The area drawn: the element's outline, with room around it.
constexpr QRect kArea(-16, -16, 96, 96);

/// Returns the attributes of \a theme, the current theme left as it was.
ThemeAttributes attributesOf(const Theme theme)
{
    const Theme before = ThemeManager::theme();
    const auto restore = qScopeGuard([before] { ThemeManager::setTheme(before); });
    ThemeManager::setTheme(theme);
    return ThemeManager::attributes();
}

/// Has ElementPainter draw \a element in \a theme, in the element's own coordinates.
QImage drawn(const Element &element, const ThemeAttributes &theme)
{
    QImage image(kArea.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.translate(-kArea.topLeft());
    ElementPainter::paint(painter, element, theme);
    painter.end();
    return image;
}

/// Returns a pixel of \a image inside \a outline, clear of its rounded corners and its border, where \a image is
/// transparent; none when there is none.
std::optional<QPoint> clearPixelIn(const QImage &image, const QRectF &outline)
{
    const QRect inside = outline.adjusted(6, 6, -6, -6).toAlignedRect().translated(-kArea.topLeft());
    for (int y = inside.top(); y <= inside.bottom(); ++y) {
        for (int x = inside.left(); x <= inside.right(); ++x) {
            if (image.pixelColor(x, y).alpha() == 0) {
                return QPoint(x, y);
            }
        }
    }
    return std::nullopt;
}

/// \c true when \a actual is \a expected, within the rounding of a translucent colour in a premultiplied image.
bool sameColour(const QColor &actual, const QColor &expected)
{
    return (actual.alpha() == expected.alpha()) && (std::abs(actual.red() - expected.red()) <= 2)
        && (std::abs(actual.green() - expected.green()) <= 2) && (std::abs(actual.blue() - expected.blue()) <= 2);
}

} // namespace ElementPainterTestDetail

using namespace ElementPainterTestDetail;

void TestElementPainter::testASelectedElementHasItsSelectionUnderItsBody()
{
    // A selected element is drawn over a rounded rectangle in the selection colours, which shows where the body leaves
    // the outline clear; where the body is opaque, it is drawn as it is when the element is not selected
    const ThemeAttributes theme = attributesOf(Theme::Light);
    And gate;
    const QImage plain = drawn(gate, theme);

    gate.setSelected(true);
    QVERIFY(gate.isSelected());
    const QImage selected = drawn(gate, theme);
    QVERIFY(selected != plain);

    const std::optional<QPoint> clear = clearPixelIn(plain, gate.outlineRect());
    QVERIFY(clear);
    QVERIFY2(sameColour(selected.pixelColor(*clear), theme.m_selectionBrush),
             qPrintable(selected.pixelColor(*clear).name(QColor::HexArgb)));

    int opaque = 0;
    for (int y = 0; y < plain.height(); ++y) {
        for (int x = 0; x < plain.width(); ++x) {
            if (plain.pixelColor(x, y).alpha() == 255) {
                ++opaque;
                QCOMPARE(selected.pixelColor(x, y), plain.pixelColor(x, y));
            }
        }
    }
    QVERIFY(opaque > 0);
}

void TestElementPainter::testAnElementIsDrawnWhollyInTheThemeHanded()
{
    // The selection and a themed body take the theme handed, whichever theme is current
    const Theme before = ThemeManager::theme();
    const auto restore = qScopeGuard([before] { ThemeManager::setTheme(before); });
    const ThemeAttributes light = attributesOf(Theme::Light);
    const ThemeAttributes dark = attributesOf(Theme::Dark);
    QVERIFY(light.m_selectionBrush != dark.m_selectionBrush);

    DFlipFlop flipFlop;
    const std::optional<QPoint> clear = clearPixelIn(drawn(flipFlop, light), flipFlop.outlineRect());
    QVERIFY(clear);
    flipFlop.setSelected(true);

    for (const Theme current : {Theme::Light, Theme::Dark}) {
        ThemeManager::setTheme(current);
        for (const ThemeAttributes &theme : {light, dark}) {
            const QImage image = drawn(flipFlop, theme);
            QVERIFY2(sameColour(image.pixelColor(*clear), theme.m_selectionBrush),
                     qPrintable(image.pixelColor(*clear).name(QColor::HexArgb)));
            ThemeManager::setTheme(theme.theme());
            const QImage inItsTheme = drawn(flipFlop, theme);
            ThemeManager::setTheme(current);
            QCOMPARE(image, inItsTheme);
        }
    }
}

void TestElementPainter::testAnElementDrawsItselfWithThePainter()
{
    // While elements are graphics items, an element draws itself as ElementPainter draws it in the current theme
    const Theme before = ThemeManager::theme();
    const auto restore = qScopeGuard([before] { ThemeManager::setTheme(before); });

    for (const Theme current : {Theme::Light, Theme::Dark}) {
        ThemeManager::setTheme(current);
        DFlipFlop flipFlop;
        flipFlop.setSelected(true);

        QImage image(kArea.size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.translate(-kArea.topLeft());
        const QStyleOptionGraphicsItem option;
        flipFlop.paint(&painter, &option, nullptr);
        painter.end();

        QCOMPARE(image, drawn(flipFlop, ThemeManager::attributes()));
    }
}
