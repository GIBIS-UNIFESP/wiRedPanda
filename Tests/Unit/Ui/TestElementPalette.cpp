// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementPalette.h"

#include <QLabel>
#include <QMainWindow>
#include <QTabWidget>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "App/UI/ElementPalette.h"
#include "App/UI/MainWindowUI.h"
#include "Tests/Common/TestUtils.h"

void TestElementPalette::testPaletteSearch()
{
    // Regex metacharacters in the query must be matched literally, not interpreted as pattern
    // syntax. Before the fix (B3), an unescaped query built an invalid QRegularExpression whose
    // match() never succeeded, so the user got zero results even when items matched.

    // A lone '(' is an invalid regex on its own; it must still match a name that contains '('
    // and must not match one that doesn't — never error into matching nothing.
    QVERIFY(ElementPalette::nameMatchesSearch("nand(2)", "("));
    QVERIFY(!ElementPalette::nameMatchesSearch("nand", "("));

    // An IC named with parentheses is findable by a substring that includes them.
    QVERIFY(ElementPalette::nameMatchesSearch("counter(2)", "counter(2)"));
    QVERIFY(ElementPalette::nameMatchesSearch("counter(2)", "(2)"));

    // Ordinary substring matching still works and is case-insensitive.
    QVERIFY(ElementPalette::nameMatchesSearch("Input Switch", "switch"));
    QVERIFY(!ElementPalette::nameMatchesSearch("And", "xyz"));

    // Other metacharacters that would break an unescaped pattern are matched literally.
    QVERIFY(ElementPalette::nameMatchesSearch("a+b", "+"));
    QVERIFY(ElementPalette::nameMatchesSearch("a[b", "["));
    QVERIFY(ElementPalette::nameMatchesSearch("a\\b", "\\"));
}

void TestElementPalette::testPaletteRebuild()
{
    // ElementPalette drives the host window's MainWindowUi widgets; build a real one on a
    // throwaway QMainWindow (no MainWindow needed) and verify populate() fills the category
    // tabs with draggable ElementLabel widgets.
    QMainWindow window;
    MainWindowUi ui;
    ui.setupUi(&window);

    ElementPalette palette(&ui);
    palette.populate();

    QVERIFY(ui.tabElements->count() > 0);
    QVERIFY(!window.findChildren<ElementLabel *>().isEmpty());
}

void TestElementPalette::testElementLabelHasDescriptiveTooltip()
{
    // A gate label describes what the element does and how to place it.
    ElementLabel andLabel(ElementFactory::pixmap(ElementType::And), ElementType::And, QString());
    QVERIFY2(andLabel.toolTip().contains(QStringLiteral("AND")), qPrintable(andLabel.toolTip()));
    QVERIFY2(andLabel.toolTip().contains(QStringLiteral("double-click")), qPrintable(andLabel.toolTip()));

    // The tooltip is mirrored onto the child labels so it shows wherever the pointer rests.
    const auto children = andLabel.findChildren<QLabel *>();
    QVERIFY(!children.isEmpty());
    for (auto *child : children) {
        QCOMPARE(child->toolTip(), andLabel.toolTip());
    }

    // Descriptions are element-specific.
    ElementLabel switchLabel(ElementFactory::pixmap(ElementType::InputSwitch), ElementType::InputSwitch, QString());
    QVERIFY(!switchLabel.toolTip().isEmpty());
    QVERIFY(switchLabel.toolTip() != andLabel.toolTip());
}

void TestElementPalette::testAccessibleNameSet()
{
    QMainWindow window;
    MainWindowUi ui;
    ui.setupUi(&window);

    ElementPalette palette(&ui);

    QVERIFY(!ui.lineEditSearch->accessibleName().isEmpty());
    QVERIFY(!ui.lineEditSearch->whatsThis().isEmpty());
    QVERIFY(!ui.tabElements->accessibleName().isEmpty());
    QVERIFY(!ui.tabElements->whatsThis().isEmpty());
}
