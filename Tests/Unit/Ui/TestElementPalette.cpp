// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementPalette.h"

#include <QLabel>
#include <QMainWindow>
#include <QTabWidget>
#include <QTemporaryDir>

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

void TestElementPalette::testPopulateFallsBackToFirstTabWhenIoTabIsMissing()
{
    // Regression seam: populate() looks up the "io" tab by object name and falls back to
    // tab 0 if it isn't found. Rename the real "io" tab so the lookup misses, exercising
    // both the fallback branch and tabIndex()'s own "not found" return.
    QMainWindow window;
    MainWindowUi ui;
    ui.setupUi(&window);

    bool foundIoTab = false;
    for (int i = 0; i < ui.tabElements->count(); ++i) {
        if (ui.tabElements->widget(i)->objectName() == QLatin1String("io")) {
            ui.tabElements->widget(i)->setObjectName(QStringLiteral("io_renamed"));
            foundIoTab = true;
            break;
        }
    }
    QVERIFY2(foundIoTab, "expected MainWindowUi to have a tab named \"io\"");

    ElementPalette palette(&ui);
    palette.populate();

    QCOMPARE(ui.tabElements->currentIndex(), 0);
}

void TestElementPalette::testSearchFirstPassMatchesByInternalTypeKeyword()
{
    // First pass matches the internal, locale-independent type keyword via the label's
    // object name ("label_truthtable"), separately from the second pass's translated-name
    // match. "Truth Table" (the translated name) contains a space, so a contiguous query of
    // "truthtable" can only ever match through the first pass's object-name check.
    QMainWindow window;
    MainWindowUi ui;
    ui.setupUi(&window);

    ElementPalette palette(&ui);
    palette.populate();

    ElementLabel *truthTableSearchLabel = nullptr;
    ElementLabel *andSearchLabel = nullptr;
    const auto searchItems = ui.scrollAreaWidgetContents_Search->findChildren<ElementLabel *>();
    for (auto *item : searchItems) {
        if (item->objectName() == QLatin1String("label_truthtable")) {
            truthTableSearchLabel = item;
        } else if (item->objectName() == QLatin1String("label_and")) {
            andSearchLabel = item;
        }
    }
    QVERIFY(truthTableSearchLabel);
    QVERIFY(andSearchLabel);
    QVERIFY2(!truthTableSearchLabel->name().contains(QStringLiteral("truthtable"), Qt::CaseInsensitive),
              "test assumes the translated name has a space, so it can't itself match the query");

    ui.lineEditSearch->setText(QStringLiteral("truthtable"));

    QVERIFY2(!truthTableSearchLabel->isHidden(), "the first-pass object-name match must surface TruthTable");
    QVERIFY2(andSearchLabel->isHidden(), "And must not match a query that hits neither of its passes");
}

void TestElementPalette::testSearchSecondPassMatchesByTranslatedNameOnly()
{
    // Second pass adds items matched by translated name that the first pass's object-name
    // check didn't already find. InputButton's object name is "label_inputbutton" (from its
    // raw type keyword), while its translated name is "Push Button" -- a query of "push"
    // matches only the translated name, so it can only ever be appended by the second pass.
    QMainWindow window;
    MainWindowUi ui;
    ui.setupUi(&window);

    ElementPalette palette(&ui);
    palette.populate();

    ElementLabel *inputButtonSearchLabel = nullptr;
    const auto searchItems = ui.scrollAreaWidgetContents_Search->findChildren<ElementLabel *>();
    for (auto *item : searchItems) {
        if (item->objectName() == QLatin1String("label_inputbutton")) {
            inputButtonSearchLabel = item;
        }
    }
    QVERIFY(inputButtonSearchLabel);
    QCOMPARE(inputButtonSearchLabel->name(), QStringLiteral("Push Button"));
    QVERIFY2(!inputButtonSearchLabel->objectName().contains(QStringLiteral("push"), Qt::CaseInsensitive),
              "test assumes the object name can't itself match the query");

    ui.lineEditSearch->setText(QStringLiteral("push"));

    QVERIFY2(!inputButtonSearchLabel->isHidden(), "the second-pass translated-name match must surface Push Button");
}

void TestElementPalette::testSearchThirdPassMatchesByIcFullFileName()
{
    // Third pass matches the IC's full file name (including the .panda extension) via
    // objectName "label_ic", separately from the second pass's name() match (the upper-cased
    // base name only, without the extension). Querying the extension itself can only ever
    // match through the third pass.
    QMainWindow window;
    MainWindowUi ui;
    ui.setupUi(&window);

    ElementPalette palette(&ui);
    palette.populate();

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString currentProjectPath = dir.filePath(QStringLiteral("currentproject.panda"));
    const QString icPath = dir.filePath(QStringLiteral("widget.panda"));
    QFile currentProjectFile(currentProjectPath);
    QVERIFY(currentProjectFile.open(QIODevice::WriteOnly));
    currentProjectFile.close();
    QFile icFile(icPath);
    QVERIFY(icFile.open(QIODevice::WriteOnly));
    icFile.close();

    palette.updateICList(QFileInfo(currentProjectPath));

    ElementLabel *icSearchLabel = nullptr;
    const auto searchItems = ui.scrollAreaWidgetContents_Search->findChildren<ElementLabel *>();
    for (auto *item : searchItems) {
        if (item->elementType() == ElementType::IC && item->icFileName() == QLatin1String("widget.panda")) {
            icSearchLabel = item;
        }
    }
    QVERIFY(icSearchLabel);
    QCOMPARE(icSearchLabel->objectName(), QStringLiteral("label_ic"));
    QVERIFY2(!icSearchLabel->name().contains(QStringLiteral("panda"), Qt::CaseInsensitive),
              "test assumes the base name alone doesn't contain the query, so pass two can't match it");

    ui.lineEditSearch->setText(QStringLiteral("panda"));

    QVERIFY2(!icSearchLabel->isHidden(), "the third-pass file-name match must surface widget.panda");
}
