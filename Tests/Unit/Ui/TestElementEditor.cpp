// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementEditor.h"

#include <algorithm>
#include <memory>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPixmap>
#include <QSignalSpy>
#include <QSlider>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QTest>
#include <QToolButton>
#include <QUndoStack>
#include <QVariant>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementEditor.h"
#include "App/Wiring/Connection.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/ICTestHelpers.h"
#include "Tests/Common/StubFileDialogProvider.h"
#include "Tests/Common/TestUtils.h"

namespace {

// Finds the top-level QMenu shown by ElementEditor::contextMenu() and triggers the action with
// the given text. A plain action->trigger() only emits the signal -- it does not drive
// QMenuPrivate's internal close/return machinery, so QMenu::exec() itself would never unblock.
// Deliberately NOT a synthetic mouse click on the item's computed geometry: that only proves
// this code found the right action and clicked where it believes that action is -- it doesn't
// prove QMenu's own internal hit-testing (which independently resolves what's "under" a point at
// click time) agrees, so under any layout/timing skew a geometry-based click can silently
// activate an *adjacent* action instead while this helper still reports success (confirmed root
// cause of several outwardly-unrelated-looking flakes: a mistargeted click one item over from
// "Rename" never reaches renameAction()/setFocus() at all, and one item over from "Restore
// default appearance" instead opens the real (unstubbed) file-open dialog, which returns empty
// on a headless runner and silently no-ops). menu->setActiveAction() + Key_Return instead
// operates directly on the QAction* through Qt's own current-action state -- no pixel geometry,
// DPI scaling, or native hit-testing involved, so it cannot mistarget.
TestUtils::AutoDismisser dismisserForMenuAction(const QString &actionText)
{
    return TestUtils::AutoDismisser([actionText](QWidget *w) {
        auto *menu = qobject_cast<QMenu *>(w);
        if (!menu) return false;
        for (auto *action : menu->actions()) {
            if (action->text() == actionText) {
                menu->setActiveAction(action);
                QTest::keyClick(menu, Qt::Key_Return);
                return true;
            }
        }
        return false;
    });
}

// Shows `w` and makes its top-level window active -- required under the offscreen QPA
// platform for hasFocus() to reliably reflect setFocus() calls (activateWindow() is a no-op
// there since there is no window manager; mirrors TestMainWindowGui.cpp's createMW() helper).
// ElementEditor is normally a child widget (parented to its WorkSpace), so
// QApplication::setActiveWindow() -- which only operates on top-level widgets -- must target
// w->window(), not w itself.
void showAndActivate(QWidget *w)
{
    QWidget *top = w->window();
    top->show();
    w->show();
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
    QApplication::setActiveWindow(top);
QT_WARNING_POP
    // Under parallel ctest execution (real CPU contention across processes), activation isn't
    // always synchronous even on the offscreen platform -- wait for it to actually land before
    // any caller relies on hasFocus() working (confirmed empirically: a fixed single
    // processEvents() call after this function was insufficient under load; this widened it to
    // an actively-polling, bounded wait instead of assuming one flush is always enough).
    TestUtils::waitFor([top] { return QApplication::activeWindow() == top; });
}

// IC::loadFile()/loadFromBlob() accepts this as real IC file content.
using ICTestHelpers::minimalPandaBytes;

// What `element` holds for the property kind `type`, as the editor's golden records it: files by name, since the
// golden's files live in a temporary directory.
QString editedValue(const Element *element, const PropertyDescriptor::Type type)
{
    switch (type) {
    case PropertyDescriptor::Type::Label:
        return TestUtils::snapshotQuoted(element->label());
    case PropertyDescriptor::Type::Color:
        return element->color();
    case PropertyDescriptor::Type::Frequency:
        return TestUtils::snapshotNumber(element->frequency());
    case PropertyDescriptor::Type::Delay:
        return TestUtils::snapshotNumber(element->delay());
    case PropertyDescriptor::Type::AudioBox:
        return TestUtils::snapshotQuoted(QFileInfo(element->audio()).fileName());
    case PropertyDescriptor::Type::Trigger:
        return TestUtils::snapshotQuoted(element->trigger().toString());
    case PropertyDescriptor::Type::TruthTable:
        return "-";
    case PropertyDescriptor::Type::Appearance:
        return SnapshotAccess::pixmapIdentity(element);
    case PropertyDescriptor::Type::Volume:
        return TestUtils::snapshotNumber(static_cast<qreal>(element->volume()));
    case PropertyDescriptor::Type::WirelessModeSelector:
        return QVariant::fromValue(element->wirelessMode()).toString();
    }
    return {};
}

} // namespace

void TestElementEditor::initTestCase()
{
    // Force ElementFactory::pixmap()'s per-element-type render cache to populate for every type
    // whose icon this file's contextMenu() tests depend on (e.g. the "Change frequency" action's
    // Clock pixmap) -- a plain, side-effect-free static call, so it can't disturb anything else.
    // Confirmed via CI this cost is real and per-type (keyed by ElementType, not shared): a
    // Node-only warm-up left the first test using a Clock element to pay this cost on its own.
    ElementFactory::pixmap(ElementType::Clock);
    ElementFactory::pixmap(ElementType::InputButton);

    // Separately, force the one-time costs of the first real ElementContextMenu::exec() call in
    // this process -- Qt's lazy font-alias table population, plus the first-ever decode of the
    // menu's toolbar SVG icons (rename.svg, rotateL.svg, etc.) via the SVG image plugin -- to
    // happen here instead of inside the first contextMenu()-driven test below. A plain icon-less
    // throwaway QMenu closed immediately (an earlier version of this warm-up) did NOT reproduce
    // this: closing before a real paint pass meant nothing was actually forced, so the cost
    // landed on the first real test instead (confirmed via CI). Going through the real
    // production entry point with a real element guarantees whatever one-time costs exist get
    // paid here. Deliberately a single element/editor/workspace, constructed and torn down once:
    // an earlier attempt looped this over multiple elements to also cover the Clock icon above,
    // but two ElementEditor windows briefly taking and releasing "active window" status back to
    // back left QApplication's activation tracking in a state that broke the very first real
    // test's own showAndActivate() (confirmed via CI: re-introduced the failure this was meant to
    // fix) -- hence handling the icon-cache warm-up above as a plain function call instead.
    WorkSpace workspace;
    auto *node = new Node();
    workspace.scene()->addItem(node);
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    node->setSelected(true);
    showAndActivate(&editor);

    TestUtils::AutoDismisser dismisser([](QWidget *w) {
        auto *menu = qobject_cast<QMenu *>(w);
        if (!menu) return false;
        // isVisible() becoming true doesn't mean a paint has actually happened yet -- closing
        // immediately (an earlier version of this warm-up) tore the menu down before the
        // windowing system ever exposed/painted it, so the font-layout and icon-decode costs
        // never actually ran here. qWaitForWindowExposed() is QTest's own mechanism for waiting
        // until a widget has really been painted at least once.
        [[maybe_unused]] const bool exposed = QTest::qWaitForWindowExposed(menu);
        menu->close();
        return true;
    });
    editor.contextMenu(QPoint(0, 0), nullptr);
}

void TestElementEditor::testCreation()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);

    // The constructor builds the full property-editor widget tree.
    QVERIFY(editor.findChild<QComboBox *>("comboBoxColor"));
    QVERIFY(editor.findChild<QComboBox *>("comboBoxInputSize"));
    QVERIFY(editor.findChild<QWidget *>("widgetAppearanceStates"));
}

void TestElementEditor::testSetScene()
{
    WorkSpace workspace;
    auto *led = new Led;
    workspace.scene()->addItem(led);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // Connected: selecting an element shows the editor.
    workspace.scene()->clearSelection();
    led->setSelected(true);
    QVERIFY(!editor.isHidden());

    // Disconnected: a further scene selection change is ignored — clearing the selection
    // would hide the editor if it were still connected to the scene.
    editor.setScene(nullptr);
    workspace.scene()->clearSelection();
    QVERIFY(!editor.isHidden());
}

void TestElementEditor::testRetranslateUi()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    editor.retranslateUi();

    // retranslateUi() (re)applies the static widget labels.
    auto *labelColor = editor.findChild<QLabel *>("labelColor");
    QVERIFY(labelColor);
    QCOMPARE(labelColor->text(), QStringLiteral("Color:"));
}

void TestElementEditor::testSetCurrentElementsEmpty()
{
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // A non-empty selection shows the editor...
    andGate->setSelected(true);
    QVERIFY(!editor.isHidden());

    // ...and clearing the selection hides it (setCurrentElements({}) calls hide()).
    workspace.scene()->clearSelection();
    QVERIFY(editor.isHidden());
}

void TestElementEditor::testSetCurrentElementsGate()
{
    WorkSpace workspace;
    auto *andGate = new And;
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    workspace.scene()->clearSelection();
    andGate->setSelected(true);

    // Selecting a gate shows the editor and enables the input-size selector (And is 2..8 inputs).
    QVERIFY(!editor.isHidden());
    auto *inputSizeCombo = editor.findChild<QComboBox *>("comboBoxInputSize");
    QVERIFY(inputSizeCombo);
    QVERIFY(inputSizeCombo->isEnabled());
}

void TestElementEditor::testSetCurrentElementsLed()
{
    WorkSpace workspace;
    auto *led = new Led;
    workspace.scene()->addItem(led);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // Select LED — triggers selectionChanged, populating the appearance tile grid
    workspace.scene()->clearSelection();
    led->setSelected(true);

    auto *statesWidget = editor.findChild<QWidget *>("widgetAppearanceStates");
    QVERIFY(statesWidget);
    // The editor itself is never shown in this test, so isVisible() (which also
    // depends on ancestor/window visibility) would always be false; isHidden()
    // reflects only this widget's own explicit setVisible() flag.
    QVERIFY(!statesWidget->isHidden());

    const auto tiles = statesWidget->findChildren<QToolButton *>();
    QCOMPARE(tiles.size(), 2); // 1-input LED: Off/On

    const auto checkedCount = std::count_if(tiles.begin(), tiles.end(), [](QToolButton *tile) { return tile->isChecked(); });
    QCOMPARE(checkedCount, 1);
    QVERIFY(tiles.first()->isChecked()); // first tile selected by default
}

void TestElementEditor::testAppearanceStateTileSelection()
{
    WorkSpace workspace;
    auto *led = new Led;
    led->setInputSize(2);
    workspace.scene()->addItem(led);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    workspace.scene()->clearSelection();
    led->setSelected(true);

    auto *statesWidget = editor.findChild<QWidget *>("widgetAppearanceStates");
    QVERIFY(statesWidget);

    const auto tiles = statesWidget->findChildren<QToolButton *>();
    QCOMPARE(tiles.size(), 4); // 2-input LED: 4 states

    auto *secondTile = tiles.at(1);
    secondTile->click();

    QVERIFY(secondTile->isChecked());
    QVERIFY(!tiles.first()->isChecked());
    QCOMPARE(secondTile->property("appearanceStateIndex").toInt(), led->appearanceStates().at(1).first);
}

void TestElementEditor::testFillColorComboBox()
{
    WorkSpace workspace;
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    editor.fillColorComboBox();

    // The dropdown holds the five LED colours; item data carries the untranslated English
    // name used internally by Element::setColor().
    auto *combo = editor.findChild<QComboBox *>("comboBoxColor");
    QVERIFY(combo);
    QCOMPARE(combo->count(), 5);
    QCOMPARE(combo->itemData(0).toString(), QStringLiteral("White"));
    QCOMPARE(combo->itemData(4).toString(), QStringLiteral("Purple"));
}

void TestElementEditor::testSelectionDoesNotPushPortSizeCommandB21()
{
    // Pre-fix the input/output combo-box rebuild in setCurrentElements ran
    // without QSignalBlocker, so the synthetic setCurrentIndex inside the
    // rebuild fired inputIndexChanged → ChangePortSizeCommand each time the
    // user selected an element. The QSignalBlocker added in B21 silences
    // the rebuild signal storm; a fresh selection must therefore leave the
    // undo stack untouched.
    WorkSpace workspace;
    auto *andGate = new And();
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    // Cycle selection a few times to give the combo-box rebuild several
    // chances to misfire.
    for (int i = 0; i < 3; ++i) {
        workspace.scene()->clearSelection();
        andGate->setSelected(true);
    }

    QCOMPARE(undoStack->count(), countBefore);
}

void TestElementEditor::testRenameActionFocusesAndSelectsLabelField()
{
    WorkSpace workspace;
    auto *node = new Node(); // Node has hasLabel == true.
    workspace.scene()->addItem(node);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    node->setSelected(true); // enables the editor -- setFocus() is a no-op while disabled.
    showAndActivate(&editor);

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    QVERIFY(lineEdit);
    lineEdit->setText("MyLabel");

    editor.renameAction();
    QVERIFY2(TestUtils::waitFor([&] { return lineEdit->hasFocus(); }), "lineEditElementLabel must gain focus");
    QCOMPARE(lineEdit->selectedText(), QStringLiteral("MyLabel"));
}

void TestElementEditor::testChangeTriggerActionFocusesAndSelectsTriggerField()
{
    WorkSpace workspace;
    auto *button = ElementFactory::buildElement(ElementType::InputButton); // hasTrigger == true.
    workspace.scene()->addItem(button);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    button->setSelected(true); // enables the editor -- setFocus() is a no-op while disabled.
    showAndActivate(&editor);

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditTrigger");
    QVERIFY(lineEdit);
    lineEdit->setText("A");

    editor.changeTriggerAction();
    QVERIFY2(TestUtils::waitFor([&] { return lineEdit->hasFocus(); }), "lineEditTrigger must gain focus");
    QCOMPARE(lineEdit->selectedText(), QStringLiteral("A"));
}

void TestElementEditor::testContextMenuRenameActionFocusesLabelField()
{
#ifdef Q_OS_MACOS
    QSKIP("macOS's real Cocoa windowing makes dismisserForMenuAction()'s QMenu::exec()-driven context-menu tests unreliable outside the offscreen QPA platform");
#endif
    WorkSpace workspace;
    auto *node = new Node(); // Node has hasLabel == true.
    workspace.scene()->addItem(node);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    node->setSelected(true);
    showAndActivate(&editor);

    auto dismisser = dismisserForMenuAction(QStringLiteral("Rename"));
    editor.contextMenu(QPoint(0, 0), nullptr);

    QVERIFY2(dismisser.dismissCount() >= 1, "the \"Rename\" action must have been found in the menu");
    // The now-closed popup may have taken over "active window" status without handing it back
    // (no real window manager under the offscreen platform to do that automatically) -- without
    // re-asserting it, setFocus() below can never surface via hasFocus() (see showAndActivate()'s
    // own comment; same mechanism, just needed again after the popup closes this time).
    showAndActivate(&editor);
    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    QVERIFY(lineEdit);
    QVERIFY2(TestUtils::waitFor([&] { return lineEdit->hasFocus(); }), "lineEditElementLabel must gain focus");
}

void TestElementEditor::testContextMenuTriggerActionFocusesTriggerField()
{
#ifdef Q_OS_MACOS
    QSKIP("macOS's real Cocoa windowing makes dismisserForMenuAction()'s QMenu::exec()-driven context-menu tests unreliable outside the offscreen QPA platform");
#endif
    WorkSpace workspace;
    auto *button = ElementFactory::buildElement(ElementType::InputButton);
    workspace.scene()->addItem(button);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    button->setSelected(true);
    showAndActivate(&editor);

    auto dismisser = dismisserForMenuAction(QStringLiteral("Change trigger"));
    editor.contextMenu(QPoint(0, 0), nullptr);

    QVERIFY2(dismisser.dismissCount() >= 1, "the \"Change trigger\" action must have been found in the menu");
    showAndActivate(&editor); // re-assert activation lost to the now-closed popup -- see above
    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditTrigger");
    QVERIFY(lineEdit);
    QVERIFY2(TestUtils::waitFor([&] { return lineEdit->hasFocus(); }), "lineEditTrigger must gain focus");
}

void TestElementEditor::testContextMenuFrequencyActionFocusesSpinBox()
{
#ifdef Q_OS_MACOS
    QSKIP("macOS's real Cocoa windowing makes dismisserForMenuAction()'s QMenu::exec()-driven context-menu tests unreliable outside the offscreen QPA platform");
#endif
    WorkSpace workspace;
    auto *clock = new Clock();
    workspace.scene()->addItem(clock);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    clock->setSelected(true);
    showAndActivate(&editor);

    auto dismisser = dismisserForMenuAction(QStringLiteral("Change frequency"));
    editor.contextMenu(QPoint(0, 0), nullptr);

    QVERIFY2(dismisser.dismissCount() >= 1, "the \"Change frequency\" action must have been found in the menu");
    showAndActivate(&editor); // re-assert activation lost to the now-closed popup -- see above
    auto *spinBox = editor.findChild<QDoubleSpinBox *>("doubleSpinBoxFrequency");
    QVERIFY(spinBox);
    QVERIFY2(TestUtils::waitFor([&] { return spinBox->hasFocus(); }), "doubleSpinBoxFrequency must gain focus");
}

void TestElementEditor::testContextMenuAppearanceRevertSetsDefaultAppearance()
{
#ifdef Q_OS_MACOS
    QSKIP("macOS's real Cocoa windowing makes dismisserForMenuAction()'s QMenu::exec()-driven context-menu tests unreliable outside the offscreen QPA platform");
#endif
    WorkSpace workspace;
    auto *andGate = new And(); // And has canChangeAppearance == true.
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    andGate->setSelected(true);

    // A custom appearance is active so the revert has a real transition to make.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString imagePath = dir.filePath("custom.png");
    QPixmap image(4, 4);
    image.fill(Qt::red);
    QVERIFY(image.save(imagePath, "PNG"));
    andGate->setAppearance(false, imagePath);
    QCOMPARE(andGate->externalFiles(), QStringList{imagePath});

    auto dismisser = dismisserForMenuAction(QStringLiteral("Restore default appearance"));
    editor.contextMenu(QPoint(0, 0), nullptr);

    QVERIFY2(dismisser.dismissCount() >= 1, "the \"Restore default appearance\" action must have been found in the menu");
    QVERIFY(andGate->externalFiles().isEmpty());
}

void TestElementEditor::testContextMenuEditSubcircuitEmitsForEmbeddedIC()
{
#ifdef Q_OS_MACOS
    QSKIP("macOS's real Cocoa windowing makes dismisserForMenuAction()'s QMenu::exec()-driven context-menu tests unreliable outside the offscreen QPA platform");
#endif
    WorkSpace workspace;
    auto *ic = new IC();
    workspace.scene()->addItem(ic);
    ICTestHelpers::embedIC(ic, minimalPandaBytes(), QStringLiteral("myBlob"),
                            workspace.scene()->contextDir(), workspace.scene()->icRegistry());

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    ic->setSelected(true);

    QSignalSpy spy(&editor, &ElementEditor::editSubcircuitRequested);

    auto dismisser = dismisserForMenuAction(QStringLiteral("Edit sub-circuit"));
    editor.contextMenu(QPoint(0, 0), nullptr);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("myBlob"));
    QCOMPARE(spy.at(0).at(1).toInt(), ic->id());
}

void TestElementEditor::testContextMenuEmbedSubcircuitEmitsForFileBackedIC()
{
#ifdef Q_OS_MACOS
    QSKIP("macOS's real Cocoa windowing makes dismisserForMenuAction()'s QMenu::exec()-driven context-menu tests unreliable outside the offscreen QPA platform");
#endif
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString icPath = dir.filePath("sub.panda");
    QFile icFile(icPath);
    QVERIFY(icFile.open(QIODevice::WriteOnly));
    icFile.write(minimalPandaBytes());
    icFile.close();

    WorkSpace workspace;
    auto *ic = new IC();
    workspace.scene()->addItem(ic);
    ic->loadFile(icPath, dir.path());
    QVERIFY(!ic->isEmbedded());

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    ic->setSelected(true);

    {
        QSignalSpy spy(&editor, &ElementEditor::embedSubcircuitRequested);
        auto dismisser = dismisserForMenuAction(QStringLiteral("Embed sub-circuit"));
        editor.contextMenu(QPoint(0, 0), nullptr);
        QCOMPARE(spy.count(), 1);
    }

    // A file-backed (non-embedded) IC's "Edit sub-circuit" action opens the backing file
    // directly (elementType() == IC branch), unlike the embedded-blob path exercised by
    // testContextMenuEditSubcircuitEmitsForEmbeddedIC().
    {
        QSignalSpy spy(&editor, &ElementEditor::openSubcircuitFileRequested);
        auto dismisser = dismisserForMenuAction(QStringLiteral("Edit sub-circuit"));
        editor.contextMenu(QPoint(0, 0), nullptr);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), ic->file());
    }
}

void TestElementEditor::testContextMenuExtractToFileEmitsForEmbeddedIC()
{
#ifdef Q_OS_MACOS
    QSKIP("macOS's real Cocoa windowing makes dismisserForMenuAction()'s QMenu::exec()-driven context-menu tests unreliable outside the offscreen QPA platform");
#endif
    WorkSpace workspace;
    auto *ic = new IC();
    workspace.scene()->addItem(ic);
    ICTestHelpers::embedIC(ic, minimalPandaBytes(), QStringLiteral("myBlob"),
                            workspace.scene()->contextDir(), workspace.scene()->icRegistry());

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    ic->setSelected(true);

    QSignalSpy spy(&editor, &ElementEditor::extractToFileRequested);

    auto dismisser = dismisserForMenuAction(QStringLiteral("Extract to file"));
    editor.contextMenu(QPoint(0, 0), nullptr);

    QCOMPARE(spy.count(), 1);
}

void TestElementEditor::testUpdateElementAppearanceCancelledDialogIsNoOp()
{
    WorkSpace workspace;
    auto *andGate = new And();
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    andGate->setSelected(true);

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = QString();
    const int commands = workspace.scene()->undoStack()->count();

    editor.updateElementAppearance();

    QCOMPARE(dialogStub.stub.openCallCount, 1);
    QCOMPARE(workspace.scene()->undoStack()->count(), commands);
    QVERIFY(andGate->externalFiles().isEmpty());
}

void TestElementEditor::testUpdateElementAppearanceViaTileGridUpdatesSelectedStateOnly()
{
    WorkSpace workspace;
    auto *led = new Led();
    led->setInputSize(2); // 4 appearance states
    workspace.scene()->addItem(led);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    led->setSelected(true);
    // updateElementAppearance()'s tile-grid branch checks widgetAppearanceStates->isVisible(),
    // which (unlike isHidden()) requires the whole ancestor chain to actually be shown.
    showAndActivate(&editor);

    auto *statesWidget = editor.findChild<QWidget *>("widgetAppearanceStates");
    QVERIFY(statesWidget);
    QVERIFY(statesWidget->isVisible());
    const auto tiles = statesWidget->findChildren<QToolButton *>();
    QCOMPARE(tiles.size(), 4);
    tiles.at(1)->click(); // select a state other than the default (index 0)
    QVERIFY(tiles.at(1)->isChecked());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString imagePath = dir.filePath("custom.png");
    QPixmap image(4, 4);
    image.fill(Qt::red);
    QVERIFY(image.save(imagePath, "PNG"));

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = imagePath;

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    editor.updateElementAppearance();

    // The tile-grid path applies via setAppearanceAt() + an UpdateCommand per element,
    // distinct from the fallback path's apply()-driven Type::Appearance property.
    QCOMPARE(undoStack->count(), countBefore + 1);
    QVERIFY(tiles.at(1)->isChecked());
}

void TestElementEditor::testUpdateElementAppearanceFallbackAppliesToAllSelected()
{
    WorkSpace workspace;
    auto *andGate = new And(); // single-state: no appearance-states tile grid.
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    andGate->setSelected(true);

    auto *statesWidget = editor.findChild<QWidget *>("widgetAppearanceStates");
    QVERIFY(statesWidget);
    QVERIFY(statesWidget->isHidden());

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString imagePath = dir.filePath("custom.png");
    QPixmap image(4, 4);
    image.fill(Qt::red);
    QVERIFY(image.save(imagePath, "PNG"));

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = imagePath;

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    editor.updateElementAppearance();

    QCOMPARE(andGate->externalFiles(), QStringList{imagePath});
    QCOMPARE(undoStack->count(), countBefore + 1); // applyAppearance() pushed an UpdateCommand
}

void TestElementEditor::testPickingAPictureThatDoesNotLoadChangesNothing()
{
    // A picked file that does not decode is reported, and neither the element nor the undo stack changes
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString imagePath = dir.filePath("broken.png");
    QFile broken(imagePath);
    QVERIFY(broken.open(QIODevice::WriteOnly));
    broken.write("this is not a valid image file at all");
    broken.close();

    WorkSpace workspace;
    auto *andGate = new And();
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    andGate->setSelected(true);

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = imagePath;
    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    editor.updateElementAppearance();

    QCOMPARE(dismisser.dismissCount(), 1);
    QVERIFY(andGate->externalFiles().isEmpty());
    QCOMPARE(undoStack->count(), countBefore);
}

void TestElementEditor::testPickingAPictureThatDoesNotLoadForAStateChangesNothing()
{
    // The same through the appearance states' grid, which sets one state's picture
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString imagePath = dir.filePath("broken.png");
    QFile broken(imagePath);
    QVERIFY(broken.open(QIODevice::WriteOnly));
    broken.write("this is not a valid image file at all");
    broken.close();

    WorkSpace workspace;
    auto *led = new Led();
    workspace.scene()->addItem(led);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    led->setSelected(true);
    showAndActivate(&editor);

    auto *statesWidget = editor.findChild<QWidget *>("widgetAppearanceStates");
    QVERIFY(statesWidget);
    QVERIFY(statesWidget->isVisible());
    const auto tiles = statesWidget->findChildren<QToolButton *>();
    QVERIFY(tiles.size() > 1);
    tiles.at(1)->click();

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = imagePath;
    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    editor.updateElementAppearance();

    QCOMPARE(dismisser.dismissCount(), 1);
    QVERIFY(led->externalFiles().isEmpty());
    QCOMPARE(undoStack->count(), countBefore);
}

void TestElementEditor::testApplyCapabilitiesMixedWirelessModeShowsPlaceholder()
{
    WorkSpace workspace;
    auto *node1 = new Node();
    auto *node2 = new Node();
    workspace.scene()->addItem(node1);
    workspace.scene()->addItem(node2);
    node1->setWirelessMode(WirelessMode::Tx);
    node2->setWirelessMode(WirelessMode::Rx);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    node1->setSelected(true);
    node2->setSelected(true);

    auto *combo = editor.findChild<QComboBox *>("comboBoxWirelessMode");
    QVERIFY(combo);
    // Mixed modes across the selection: prepareCombo() must select the placeholder text
    // rather than either node's own mode.
    QCOMPARE(combo->currentText(), editor.m_manyWirelessModes);
}

void TestElementEditor::testApplyCapabilitiesMixedFrequencyShowsPlaceholder()
{
    WorkSpace workspace;
    auto *clock1 = new Clock();
    auto *clock2 = new Clock();
    clock1->setFrequency(1.0);
    clock2->setFrequency(5.0);
    workspace.scene()->addItem(clock1);
    workspace.scene()->addItem(clock2);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    clock1->setSelected(true);
    clock2->setSelected(true);

    auto *spinBox = editor.findChild<QDoubleSpinBox *>("doubleSpinBoxFrequency");
    QVERIFY(spinBox);
    QCOMPARE(spinBox->value(), 0.0);
    QCOMPARE(spinBox->specialValueText(), editor.m_manyFreq);
}

void TestElementEditor::testApplyCapabilitiesMuxShowsDataInputOptions()
{
    WorkSpace workspace;
    auto *mux = new Mux();
    workspace.scene()->addItem(mux);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    mux->setSelected(true);

    auto *combo = editor.findChild<QComboBox *>("comboBoxInputSize");
    QVERIFY(combo);
    QVERIFY(combo->count() > 0);

    // Mux's item text is the data-input count (2..8); item data is the total port count
    // (data inputs + select lines). The generic branch always keeps text == data, so
    // finding a divergent item proves the Mux-specific branch built this list.
    bool foundDivergentItem = false;
    for (int i = 0; i < combo->count(); ++i) {
        if (combo->itemText(i).toInt() != combo->itemData(i).toInt()) {
            foundDivergentItem = true;
            break;
        }
    }
    QVERIFY2(foundDivergentItem, "Mux's displayed data-input count must differ from its stored total port count");
}

void TestElementEditor::testApplyCapabilitiesDemuxShowsOutputOptions()
{
    WorkSpace workspace;
    auto *demux = new Demux();
    workspace.scene()->addItem(demux);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    demux->setSelected(true);

    auto *combo = editor.findChild<QComboBox *>("comboBoxOutputSize");
    QVERIFY(combo);
    // Demux offers every value in [minOutputSize, maxOutputSize] (2..8), including "5" —
    // a value the generic curated list ({2,3,4,6,8,10,12,16}) never offers.
    QVERIFY(combo->findText("5") >= 0);
}

void TestElementEditor::testApplyCapabilitiesGenericMultiOutputShowsCuratedOptions()
{
    WorkSpace workspace;
    auto *rotary = new InputRotary(); // multi-output (2..16), not Demux/Mux.
    workspace.scene()->addItem(rotary);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    rotary->setSelected(true);

    auto *combo = editor.findChild<QComboBox *>("comboBoxOutputSize");
    QVERIFY(combo);

    QStringList texts;
    for (int i = 0; i < combo->count(); ++i) {
        texts << combo->itemText(i);
    }
    // The curated bus-width list, not every value in [min, max] (which would include "5", "7"...).
    QCOMPARE(texts, QStringList({"2", "3", "4", "6", "8", "10", "12", "16"}));
}

void TestElementEditor::testApplyCapabilitiesMixedLockStateShowsPartiallyChecked()
{
    WorkSpace workspace;
    auto *switch1 = new InputSwitch();
    auto *switch2 = new InputSwitch();
    workspace.scene()->addItem(switch1);
    workspace.scene()->addItem(switch2);
    switch1->setLocked(true);
    switch2->setLocked(false);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    switch1->setSelected(true);
    switch2->setSelected(true);

    auto *checkbox = editor.findChild<QCheckBox *>("checkBoxLocked");
    QVERIFY(checkbox);
    QCOMPARE(checkbox->checkState(), Qt::PartiallyChecked);
}

void TestElementEditor::testApplyVolumeProperty()
{
    WorkSpace workspace;
    auto *audiobox = new AudioBox();
    workspace.scene()->addItem(audiobox);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    audiobox->setSelected(true);

    auto *slider = editor.findChild<QSlider *>("sliderVolume");
    QVERIFY(slider);
    QVERIFY(slider->value() != 80);

    slider->setValue(80);

    QCOMPARE(audiobox->volume(), 0.8f);
}

void TestElementEditor::testVolumeShownAsNearestPercent()
{
    WorkSpace workspace;
    auto *audiobox = new AudioBox();
    workspace.scene()->addItem(audiobox);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    auto *slider = editor.findChild<QSlider *>("sliderVolume");
    QVERIFY(slider);

    // A volume of k / 100 times 100 lands just below k for some k (0.53 and 0.59 in float arithmetic, 0.35 among many
    // more in x87's extended precision), which truncation would show as k - 1.
    for (int percent = 0; percent <= 100; ++percent) {
        workspace.scene()->clearSelection();
        audiobox->setVolume(static_cast<float>(percent) / 100.0f);
        audiobox->setSelected(true);
        QCOMPARE(slider->value(), percent);
    }
}

void TestElementEditor::testApplyWirelessModeChangeSeversExistingConnection()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *node = new Node();
    auto *sink = new And();
    builder.add(node, sink);
    auto *conn = builder.connect(node, 0, sink, 0);
    QVERIFY(conn);
    QVERIFY(workspace.scene()->items().contains(conn));

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    node->setSelected(true);

    auto *combo = editor.findChild<QComboBox *>("comboBoxWirelessMode");
    QVERIFY(combo);
    combo->setCurrentIndex(static_cast<int>(WirelessMode::Tx)); // fires wirelessModeChanged()

    QCOMPARE(node->wirelessMode(), WirelessMode::Tx);
    // setWirelessMode() hides the port but doesn't delete the connection itself; wirelessModeChanged()'s
    // wireless-disconnect loop + DeleteItemsCommand macro must have removed it from the scene.
    QVERIFY(!workspace.scene()->items().contains(conn));

    // The mode and the wire it severed are one undo step
    workspace.scene()->undoStack()->undo();
    QCOMPARE(node->wirelessMode(), WirelessMode::None);
    QCOMPARE(TestUtils::sceneConnections(workspace.scene()).size(), 1);
}

void TestElementEditor::testRefusesALabelAnotherTxHas()
{
    WorkSpace workspace;
    auto *first = new Node();
    first->setWirelessMode(WirelessMode::Tx);
    first->setLabel("x");
    workspace.scene()->addItem(first);
    auto *second = new Node();
    second->setWirelessMode(WirelessMode::Tx);
    second->setLabel("y");
    workspace.scene()->addItem(second);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    second->setSelected(true);
    const int commands = workspace.scene()->undoStack()->count();

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    QVERIFY(lineEdit);
    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    lineEdit->setText("x");
    QTest::keyClick(lineEdit, Qt::Key_Return);

    QCOMPARE(dismisser.dismissCount(), 1);
    QCOMPARE(second->label(), QString("y"));
    QCOMPARE(workspace.scene()->undoStack()->count(), commands);
}

void TestElementEditor::testRefusesTwoSelectedNodesBecomingOneTxChannel()
{
    // Neither node is a Tx yet, so no Tx in the scene has the label: the two would be the first to share it
    WorkSpace workspace;
    auto *first = new Node();
    auto *second = new Node();
    first->setLabel("z");
    second->setLabel("z");
    workspace.scene()->addItem(first);
    workspace.scene()->addItem(second);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    first->setSelected(true);
    second->setSelected(true);
    const int commands = workspace.scene()->undoStack()->count();

    auto *combo = editor.findChild<QComboBox *>("comboBoxWirelessMode");
    QVERIFY(combo);
    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    combo->setCurrentIndex(static_cast<int>(WirelessMode::Tx));

    QCOMPARE(dismisser.dismissCount(), 1);
    QCOMPARE(first->wirelessMode(), WirelessMode::None);
    QCOMPARE(second->wirelessMode(), WirelessMode::None);
    QCOMPARE(workspace.scene()->undoStack()->count(), commands);
}

void TestElementEditor::testRefusesALabelAnotherTxHasWithANonNodeSelected()
{
    // A switch selected with the Tx hides the wireless field, but the label, which both take, still names the Tx's
    // channel
    WorkSpace workspace;
    auto *first = new Node();
    first->setWirelessMode(WirelessMode::Tx);
    first->setLabel("x");
    workspace.scene()->addItem(first);
    auto *second = new Node();
    second->setWirelessMode(WirelessMode::Tx);
    second->setLabel("y");
    workspace.scene()->addItem(second);
    auto *inputSwitch = new InputSwitch();
    workspace.scene()->addItem(inputSwitch);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    second->setSelected(true);
    inputSwitch->setSelected(true);
    const int commands = workspace.scene()->undoStack()->count();

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    QVERIFY(lineEdit);
    QVERIFY(lineEdit->isEnabled());
    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    lineEdit->setText("x");
    QTest::keyClick(lineEdit, Qt::Key_Return);

    QCOMPARE(dismisser.dismissCount(), 1);
    QCOMPARE(second->label(), QString("y"));
    QCOMPARE(inputSwitch->label(), QString());
    QCOMPARE(workspace.scene()->undoStack()->count(), commands);
}

void TestElementEditor::testTypingPastAnotherTxsLabelDoesNotWarn()
{
    // "ab" is typed one key at a time and passes through "a", another Tx's channel, on the way
    WorkSpace workspace;
    auto *first = new Node();
    first->setWirelessMode(WirelessMode::Tx);
    first->setLabel("a");
    workspace.scene()->addItem(first);
    auto *second = new Node();
    second->setWirelessMode(WirelessMode::Tx);
    second->setLabel("y");
    workspace.scene()->addItem(second);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    second->setSelected(true);

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    QVERIFY(lineEdit);
    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    lineEdit->selectAll();
    QTest::keyClicks(lineEdit, "ab");

    QCOMPARE(dismisser.dismissCount(), 0);
    QCOMPARE(second->label(), QString("ab"));
}

void TestElementEditor::testFinishingOnAnotherTxsLabelWarnsAndRestoresTheField()
{
    // The field is left on another Tx's channel: one warning, the label stays, and the field shows it again
    WorkSpace workspace;
    auto *first = new Node();
    first->setWirelessMode(WirelessMode::Tx);
    first->setLabel("a");
    workspace.scene()->addItem(first);
    auto *second = new Node();
    second->setWirelessMode(WirelessMode::Tx);
    second->setLabel("y");
    workspace.scene()->addItem(second);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    second->setSelected(true);
    const int commands = workspace.scene()->undoStack()->count();

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    QVERIFY(lineEdit);
    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    lineEdit->selectAll();
    QTest::keyClicks(lineEdit, "a");
    QTest::keyClick(lineEdit, Qt::Key_Return);

    QCOMPARE(dismisser.dismissCount(), 1);
    QCOMPARE(second->label(), QString("y"));
    QCOMPARE(lineEdit->text(), QString("y"));
    QCOMPARE(workspace.scene()->undoStack()->count(), commands);
}

void TestElementEditor::testLeavingTheFieldOnASharedLabelDoesNotWarn()
{
    // Two Tx nodes already share a label, as a file or a paste may leave them: leaving the field unedited is no edit
    WorkSpace workspace;
    auto *first = new Node();
    first->setWirelessMode(WirelessMode::Tx);
    first->setLabel("dup");
    workspace.scene()->addItem(first);
    auto *second = new Node();
    second->setWirelessMode(WirelessMode::Tx);
    second->setLabel("dup");
    workspace.scene()->addItem(second);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    second->setSelected(true);

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    QVERIFY(lineEdit);
    QCOMPARE(lineEdit->text(), QString("dup"));
    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    QTest::keyClick(lineEdit, Qt::Key_Return);

    QCOMPARE(dismisser.dismissCount(), 0);
    QCOMPARE(second->label(), QString("dup"));
}

void TestElementEditor::testBlobNameEditingFinishedIgnoresUnchangedOrEmpty()
{
    WorkSpace workspace;
    auto *ic = new IC();
    workspace.scene()->addItem(ic);
    ICTestHelpers::embedIC(ic, minimalPandaBytes(), QStringLiteral("myBlob"),
                            workspace.scene()->contextDir(), workspace.scene()->icRegistry());

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    ic->setSelected(true);

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditBlobName");
    QVERIFY(lineEdit);
    QCOMPARE(lineEdit->text(), QStringLiteral("myBlob"));

    // Unchanged text: editingFinished fires on plain focus-out too, must stay a no-op.
    lineEdit->setText("myBlob");
    editor.blobNameEditingFinished();
    QCOMPARE(ic->blobName(), QStringLiteral("myBlob"));

    // Empty text: also a no-op, must not clear the blob name.
    lineEdit->clear();
    editor.blobNameEditingFinished();
    QCOMPARE(ic->blobName(), QStringLiteral("myBlob"));
}

void TestElementEditor::testBlobNameEditingFinishedRejectsCollision()
{
    WorkSpace workspace;
    auto *ic = new IC();
    workspace.scene()->addItem(ic);
    ICTestHelpers::embedIC(ic, minimalPandaBytes(), QStringLiteral("blobA"),
                            workspace.scene()->contextDir(), workspace.scene()->icRegistry());
    // A second, distinct blob already registered under a different name.
    workspace.scene()->icRegistry()->setBlob(QStringLiteral("blobB"), minimalPandaBytes());

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    ic->setSelected(true);

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditBlobName");
    QVERIFY(lineEdit);
    lineEdit->setText("blobB");

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    editor.blobNameEditingFinished();

    QVERIFY2(dismisser.dismissCount() >= 1, "the duplicate-name warning dialog must have appeared");
    QCOMPARE(ic->blobName(), QStringLiteral("blobA"));
    QVERIFY(workspace.scene()->icRegistry()->hasBlob(QStringLiteral("blobB")));
}

void TestElementEditor::testBlobNameEditingFinishedRenamesSuccessfully()
{
    WorkSpace workspace;
    auto *ic = new IC();
    workspace.scene()->addItem(ic);
    ICTestHelpers::embedIC(ic, minimalPandaBytes(), QStringLiteral("oldName"),
                            workspace.scene()->contextDir(), workspace.scene()->icRegistry());

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    ic->setSelected(true);

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditBlobName");
    QVERIFY(lineEdit);
    lineEdit->setText("newName");

    editor.blobNameEditingFinished();

    QCOMPARE(ic->blobName(), QStringLiteral("newName"));
    QVERIFY(workspace.scene()->icRegistry()->hasBlob(QStringLiteral("newName")));
    QVERIFY(!workspace.scene()->icRegistry()->hasBlob(QStringLiteral("oldName")));
}

void TestElementEditor::testInputIndexChangedGuardsEmptyOrDisabled()
{
    WorkSpace workspace;
    auto *andGate = new And();
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    QSignalSpy spy(&editor, &ElementEditor::sendCommand);

    // Empty selection.
    editor.inputIndexChanged(0);
    QCOMPARE(spy.count(), 0);

    // Non-empty selection but explicitly disabled.
    workspace.scene()->clearSelection();
    andGate->setSelected(true);
    editor.setEnabled(false);
    editor.inputIndexChanged(0);
    QCOMPARE(spy.count(), 0);
}

void TestElementEditor::testInputIndexChangedSendsChangePortSizeCommand()
{
    WorkSpace workspace;
    auto *andGate = new And(); // 2..8 inputs, default 2.
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    andGate->setSelected(true);

    auto *combo = editor.findChild<QComboBox *>("comboBoxInputSize");
    QVERIFY(combo);
    const int newIdx = combo->findData(4);
    QVERIFY(newIdx >= 0);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    combo->setCurrentIndex(newIdx);

    QCOMPARE(undoStack->count(), countBefore + 1);
    QCOMPARE(andGate->inputSize(), 4);
}

void TestElementEditor::testOutputIndexChangedGuardsEmptyOrDisabled()
{
    WorkSpace workspace;
    auto *rotary = new InputRotary();
    workspace.scene()->addItem(rotary);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    QSignalSpy spy(&editor, &ElementEditor::sendCommand);

    // Empty selection.
    editor.outputIndexChanged(0);
    QCOMPARE(spy.count(), 0);

    // Non-empty selection but explicitly disabled.
    workspace.scene()->clearSelection();
    rotary->setSelected(true);
    editor.setEnabled(false);
    editor.outputIndexChanged(0);
    QCOMPARE(spy.count(), 0);
}

void TestElementEditor::testOutputIndexChangedSendsChangePortSizeCommand()
{
    WorkSpace workspace;
    auto *rotary = new InputRotary(); // 2..16 outputs, default 2.
    workspace.scene()->addItem(rotary);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    rotary->setSelected(true);

    auto *combo = editor.findChild<QComboBox *>("comboBoxOutputSize");
    QVERIFY(combo);
    const int newIdx = combo->findData(4);
    QVERIFY(newIdx >= 0);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    combo->setCurrentIndex(newIdx);

    QCOMPARE(undoStack->count(), countBefore + 1);
    QCOMPARE(rotary->outputSize(), 4);
}

void TestElementEditor::testOutputValueChangedForInputRotary()
{
    WorkSpace workspace;
    auto *rotary = new InputRotary();
    workspace.scene()->addItem(rotary);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    rotary->setSelected(true);

    auto *combo = editor.findChild<QComboBox *>("comboBoxValue");
    QVERIFY(combo);
    const int idx = combo->findData(1);
    QVERIFY(idx >= 0);

    combo->setCurrentIndex(idx);

    QCOMPARE(rotary->outputValue(), 1);
}

void TestElementEditor::testOutputValueChangedForRegularInput()
{
    WorkSpace workspace;
    auto *switchElm = new InputSwitch();
    workspace.scene()->addItem(switchElm);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    switchElm->setSelected(true);
    QVERIFY(!switchElm->isOn());

    auto *combo = editor.findChild<QComboBox *>("comboBoxValue");
    QVERIFY(combo);
    const int idx = combo->findData(1);
    QVERIFY(idx >= 0);

    combo->setCurrentIndex(idx);

    QVERIFY(switchElm->isOn());
}

void TestElementEditor::testInputLockedGuardsEmptyOrDisabled()
{
    WorkSpace workspace;
    auto *switchElm = new InputSwitch();
    workspace.scene()->addItem(switchElm);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    // Empty selection.
    editor.inputLocked(true);
    QVERIFY(!switchElm->isLocked());

    // Non-empty selection but explicitly disabled.
    workspace.scene()->clearSelection();
    switchElm->setSelected(true);
    editor.setEnabled(false);
    editor.inputLocked(true);
    QVERIFY(!switchElm->isLocked());
}

void TestElementEditor::testInputLockedSetsLockState()
{
    WorkSpace workspace;
    auto *switchElm = new InputSwitch();
    workspace.scene()->addItem(switchElm);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    switchElm->setSelected(true);

    auto *checkbox = editor.findChild<QCheckBox *>("checkBoxLocked");
    QVERIFY(checkbox);
    checkbox->click(); // real UI path: QCheckBox::clicked -> inputLocked(bool)

    QVERIFY(switchElm->isLocked());
}

void TestElementEditor::testTruthTablePopulatesTableAndShows()
{
    WorkSpace workspace;
    auto *tt = new TruthTable();
    workspace.scene()->addItem(tt);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    tt->setSelected(true);

    QVERIFY(!editor.m_tableBox->isVisible());

    editor.truthTable();

    QVERIFY(editor.m_tableBox->isVisible());
    QCOMPARE(editor.m_table->rowCount(), 1 << tt->inputSize());
    QCOMPARE(editor.m_table->columnCount(), tt->inputSize() + tt->outputSize());
    QCOMPARE(editor.m_table->horizontalHeaderItem(0)->text(), QStringLiteral("A"));
    QCOMPARE(editor.m_table->horizontalHeaderItem(tt->inputSize())->text(), QStringLiteral("S0"));
}

void TestElementEditor::testSetTruthTablePropositionTogglesOutputBit()
{
    WorkSpace workspace;
    auto *tt = new TruthTable();
    workspace.scene()->addItem(tt);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    tt->setSelected(true);

    editor.truthTable();

    const int row = 0;
    const int outputColumn = tt->inputSize();
    const bool bitBefore = tt->key().at(row);
    QCOMPARE(editor.m_table->item(row, outputColumn)->text(), bitBefore ? QStringLiteral("1") : QStringLiteral("0"));

    editor.setTruthTableProposition(row, outputColumn);

    QCOMPARE(tt->key().at(row), !bitBefore);
    QCOMPARE(editor.m_table->item(row, outputColumn)->text(), (!bitBefore) ? QStringLiteral("1") : QStringLiteral("0"));
}

void TestElementEditor::testAudioBoxGuardsNonAudioBoxSelection()
{
    WorkSpace workspace;
    auto *andGate = new And();
    workspace.scene()->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    ScopedFileDialogStub dialogStub;

    // Empty selection: m_elements.isEmpty() guard returns before the qobject_cast check.
    editor.audioBox();
    QCOMPARE(dialogStub.stub.openCallCount, 0);

    // Non-AudioBox selection: qobject_cast<AudioBox*> fails.
    workspace.scene()->clearSelection();
    andGate->setSelected(true);
    editor.audioBox();
    QCOMPARE(dialogStub.stub.openCallCount, 0);
}

void TestElementEditor::testAudioBoxCancelledDialogIsNoOp()
{
    WorkSpace workspace;
    auto *audiobox = new AudioBox();
    workspace.scene()->addItem(audiobox);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    audiobox->setSelected(true);

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = QString();

    const QString audioBefore = audiobox->audio();
    editor.audioBox();

    QCOMPARE(dialogStub.stub.openCallCount, 1);
    QCOMPARE(audiobox->audio(), audioBefore);
}

void TestElementEditor::testPickingASoundThatCannotBeReadChangesNothing()
{
    // A picked file that cannot be read is reported, and neither the box nor the undo stack changes
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    WorkSpace workspace;
    auto *audiobox = new AudioBox();
    workspace.scene()->addItem(audiobox);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    audiobox->setSelected(true);

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = dir.filePath("gone.wav");
    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();
    const QString audioBefore = audiobox->audio();

    auto dismisser = TestUtils::AutoDismisser::acceptMessageBox();
    editor.audioBox();

    QCOMPARE(dismisser.dismissCount(), 1);
    QCOMPARE(audiobox->audio(), audioBefore);
    QCOMPARE(undoStack->count(), countBefore);
}

void TestElementEditor::testAudioBoxSetsAudioAndUpdatesLabel()
{
    WorkSpace workspace;
    auto *audiobox = new AudioBox();
    workspace.scene()->addItem(audiobox);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    audiobox->setSelected(true);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString audioPath = dir.filePath("tone.wav");
    QFile audioFile(audioPath);
    QVERIFY(audioFile.open(QIODevice::WriteOnly));
    audioFile.close();

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = audioPath;

    editor.audioBox();

    QCOMPARE(audiobox->audio(), audioPath);
    auto *lineEdit = editor.findChild<QLineEdit *>("lineCurrentAudioBox");
    QVERIFY(lineEdit);
    QCOMPARE(lineEdit->text(), QFileInfo(audioPath).fileName());
}

void TestElementEditor::testAudioBoxChangeIsUndoable()
{
    // The audio file chosen in the editor is one undo step, which undo and redo restore
    WorkSpace workspace;
    auto *audiobox = new AudioBox();
    workspace.scene()->addItem(audiobox);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    audiobox->setSelected(true);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString audioPath = dir.filePath("tone.wav");
    QFile audioFile(audioPath);
    QVERIFY(audioFile.open(QIODevice::WriteOnly));
    audioFile.close();

    ScopedFileDialogStub dialogStub;
    dialogStub.stub.openResult = audioPath;

    QUndoStack *undoStack = workspace.scene()->undoStack();
    const int commandsBefore = undoStack->count();
    const QString audioBefore = audiobox->audio();
    editor.audioBox();
    QCOMPARE(audiobox->audio(), audioPath);
    QCOMPARE(undoStack->count(), commandsBefore + 1);

    undoStack->undo();
    QCOMPARE(audiobox->audio(), audioBefore);
    undoStack->redo();
    QCOMPARE(audiobox->audio(), audioPath);
}

void TestElementEditor::testLowercaseTriggerPushesOneCommand()
{
    // The field uppercases a typed letter, which changes its text again: the edit is still one undo step
    WorkSpace workspace;
    auto *inputSwitch = new InputSwitch();
    workspace.scene()->addItem(inputSwitch);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    inputSwitch->setSelected(true);

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditTrigger");
    QVERIFY(lineEdit);
    QUndoStack *undoStack = workspace.scene()->undoStack();
    const int commandsBefore = undoStack->count();
    QTest::keyClicks(lineEdit, "q");

    QCOMPARE(lineEdit->text(), QString("Q"));
    QCOMPARE(inputSwitch->trigger(), QKeySequence("Q"));
    QCOMPARE(undoStack->count(), commandsBefore + 1);

    undoStack->undo();
    QCOMPARE(inputSwitch->trigger(), QKeySequence());
}

void TestElementEditor::testManyTriggersPlaceholderKeepsItsCase()
{
    // The field uppercases a typed key; the placeholder it shows for a selection of different triggers is not one
    WorkSpace workspace;
    auto *switchA = new InputSwitch();
    auto *switchB = new InputSwitch();
    switchA->setTrigger(QKeySequence("A"));
    switchB->setTrigger(QKeySequence("B"));
    workspace.scene()->addItem(switchA);
    workspace.scene()->addItem(switchB);

    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    QUndoStack *undoStack = workspace.scene()->undoStack();
    const int commandsBefore = undoStack->count();
    switchA->setSelected(true);
    switchB->setSelected(true);

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditTrigger");
    QVERIFY(lineEdit);
    QCOMPARE(lineEdit->text(), editor.m_manyTriggers);
    QCOMPARE(undoStack->count(), commandsBefore);
    QCOMPARE(switchA->trigger(), QKeySequence("A"));
    QCOMPARE(switchB->trigger(), QKeySequence("B"));
}

void TestElementEditor::testWritesMatchGolden()
{
    // Each property kind every creatable type offers, written once through the editor's own widget or dialog: the
    // value written, what the element held before and after, and the commands the write pushed. A truth table is
    // edited in a dialog of its own, which is not driven here.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString imagePath = dir.filePath("custom.png");
    QPixmap image(4, 4);
    image.fill(Qt::red);
    QVERIFY(image.save(imagePath, "PNG"));
    const QString audioPath = dir.filePath("tone.wav");
    QFile audioFile(audioPath);
    QVERIFY(audioFile.open(QIODevice::WriteOnly));
    audioFile.close();

    QStringList lines;
    for (const ElementType type : TestUtils::dumpedTypes()) {
        if (!ElementFactory::hasCreator(type)) {
            lines << QString("%1 %2").arg(QLatin1String(TestUtils::kNoCreator), TestUtils::snapshotType(type));
            continue;
        }
        const auto properties = std::unique_ptr<Element>(ElementFactory::buildElement(type))->editableProperties();
        for (const auto &property : properties) {
            WorkSpace workspace;
            auto *element = ElementFactory::buildElement(type);
            workspace.scene()->addItem(element);
            ElementEditor editor(&workspace);
            editor.setScene(workspace.scene());
            workspace.scene()->clearSelection();
            element->setSelected(true);

            const QString before = editedValue(element, property.type);
            auto *undoStack = workspace.scene()->undoStack();
            const int commandsBefore = undoStack->count();
            QString written;
            switch (property.type) {
            case PropertyDescriptor::Type::Label:
                written = "Edited";
                editor.findChild<QLineEdit *>("lineEditElementLabel")->setText(written);
                break;
            case PropertyDescriptor::Type::Color: {
                auto *combo = editor.findChild<QComboBox *>("comboBoxColor");
                const int index = (combo->currentIndex() + 1) % combo->count();
                written = combo->itemText(index);
                combo->setCurrentIndex(index);
                break;
            }
            case PropertyDescriptor::Type::Frequency: {
                auto *spinBox = editor.findChild<QDoubleSpinBox *>("doubleSpinBoxFrequency");
                const double value = (spinBox->value() < spinBox->maximum()) ? spinBox->value() + spinBox->singleStep()
                                                                             : spinBox->value() - spinBox->singleStep();
                written = TestUtils::snapshotNumber(value);
                spinBox->setValue(value);
                break;
            }
            case PropertyDescriptor::Type::Delay:
            case PropertyDescriptor::Type::Volume: {
                const bool delay = (property.type == PropertyDescriptor::Type::Delay);
                auto *slider = editor.findChild<QSlider *>(delay ? "sliderDelay" : "sliderVolume");
                const int value = (slider->value() < slider->maximum()) ? slider->value() + 1 : slider->value() - 1;
                written = QString::number(value);
                slider->setValue(value);
                break;
            }
            case PropertyDescriptor::Type::AudioBox: {
                ScopedFileDialogStub dialogStub;
                dialogStub.stub.openResult = audioPath;
                written = QFileInfo(audioPath).fileName();
                editor.audioBox();
                break;
            }
            case PropertyDescriptor::Type::Trigger:
                written = "q";
                editor.findChild<QLineEdit *>("lineEditTrigger")->setText(written);
                break;
            case PropertyDescriptor::Type::TruthTable:
                written = "-";
                break;
            case PropertyDescriptor::Type::Appearance: {
                // A multi-state element's appearance goes through its state tiles, which only a shown editor offers.
                showAndActivate(&editor);
                ScopedFileDialogStub dialogStub;
                dialogStub.stub.openResult = imagePath;
                written = QFileInfo(imagePath).fileName();
                editor.updateElementAppearance();
                break;
            }
            case PropertyDescriptor::Type::WirelessModeSelector: {
                auto *combo = editor.findChild<QComboBox *>("comboBoxWirelessMode");
                written = combo->itemText(static_cast<int>(WirelessMode::Tx));
                combo->setCurrentIndex(static_cast<int>(WirelessMode::Tx));
                break;
            }
            }

            QStringList pushed;
            for (int command = commandsBefore; command < undoStack->count(); ++command) {
                pushed << TestUtils::snapshotQuoted(undoStack->command(command)->text());
            }
            lines << QString("T %1 %2 wrote=%3 before=%4 after=%5 pushed=[%6]")
                         .arg(TestUtils::snapshotType(type), TestUtils::snapshotPropertyKind(property.type),
                              TestUtils::snapshotQuoted(written), before, editedValue(element, property.type),
                              pushed.join(','));
        }
    }

    const QString golden = QString(QUOTE(CURRENTDIR)) + "/Integration/Properties/writes.txt";
    const QString mismatch = TestUtils::compareWithGolden(golden, lines.join('\n') + '\n');
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestElementEditor::testTypingALabelIsOneUndoStep()
{
    WorkSpace workspace;
    auto *input = new InputSwitch();
    workspace.scene()->addItem(input);
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    input->setSelected(true);
    auto *undoStack = workspace.scene()->undoStack();
    const int commands = undoStack->count();

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    lineEdit->setText("a");
    lineEdit->setText("ab");
    lineEdit->setText("abc");
    QCOMPARE(input->label(), QString("abc"));
    QCOMPARE(undoStack->count(), commands + 1);

    // Leaving the field ends the edit: what is typed next is a step of its own
    emit lineEdit->editingFinished();
    lineEdit->setText("abcd");
    QCOMPARE(undoStack->count(), commands + 2);

    undoStack->undo();
    QCOMPARE(input->label(), QString("abc"));
    undoStack->undo();
    QCOMPARE(input->label(), QString());
}

void TestElementEditor::testDraggingASliderIsOneUndoStep()
{
    WorkSpace workspace;
    auto *clock = new Clock();
    workspace.scene()->addItem(clock);
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    clock->setSelected(true);
    auto *undoStack = workspace.scene()->undoStack();
    const int commands = undoStack->count();

    // A drag holds the handle down
    auto *slider = editor.findChild<QSlider *>("sliderDelay");
    slider->setSliderDown(true);
    slider->setValue(1);
    slider->setValue(2);
    slider->setValue(3);
    QCOMPARE(clock->delay(), 3 / 8.0);
    QCOMPARE(undoStack->count(), commands + 1);

    // Releasing the slider ends the drag
    slider->setSliderDown(false);
    slider->setSliderDown(true);
    slider->setValue(4);
    slider->setSliderDown(false);
    QCOMPARE(undoStack->count(), commands + 2);

    undoStack->undo();
    QCOMPARE(clock->delay(), 3 / 8.0);
    undoStack->undo();
    QCOMPARE(clock->delay(), 0.0);
}

void TestElementEditor::testSliderKeyStepsAreStepsOfTheirOwn()
{
    // A slider moved with the handle not held (a key, the wheel, a click on the groove) takes an undo step per move,
    // and a later drag merges with none of them
    WorkSpace workspace;
    auto *buzzer = new Buzzer();
    workspace.scene()->addItem(buzzer);
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    buzzer->setSelected(true);
    auto *undoStack = workspace.scene()->undoStack();
    auto *slider = editor.findChild<QSlider *>("sliderVolume");
    QVERIFY(slider);
    const int start = slider->value();
    const int step = slider->singleStep();
    const int commands = undoStack->count();

    QTest::keyClick(slider, Qt::Key_Left);
    QTest::keyClick(slider, Qt::Key_Left);
    QCOMPARE(slider->value(), start - 2 * step);
    QCOMPARE(undoStack->count(), commands + 2);

    slider->setSliderDown(true);
    slider->setValue(start - 3 * step);
    slider->setValue(start - 4 * step);
    slider->setSliderDown(false);
    QCOMPARE(undoStack->count(), commands + 3);

    undoStack->undo();
    QCOMPARE(buzzer->volume(), static_cast<float>(start - 2 * step) / 100.0f);
    undoStack->undo();
    QCOMPARE(buzzer->volume(), static_cast<float>(start - step) / 100.0f);
}

void TestElementEditor::testTypingAFrequencyIsOneUndoStep()
{
    WorkSpace workspace;
    auto *clock = new Clock();
    workspace.scene()->addItem(clock);
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    clock->setSelected(true);
    const double frequency = clock->frequency();
    auto *undoStack = workspace.scene()->undoStack();
    const int commands = undoStack->count();

    auto *spinBox = editor.findChild<QDoubleSpinBox *>("doubleSpinBoxFrequency");
    spinBox->setValue(2.0);
    spinBox->setValue(25.0);
    QCOMPARE(undoStack->count(), commands + 1);
    emit spinBox->editingFinished();
    spinBox->setValue(3.0);
    QCOMPARE(undoStack->count(), commands + 2);

    undoStack->undo();
    QCOMPARE(clock->frequency(), 25.0);
    undoStack->undo();
    QCOMPARE(clock->frequency(), frequency);
}

void TestElementEditor::testEachComboPickIsAnUndoStep()
{
    WorkSpace workspace;
    auto *led = new Led();
    workspace.scene()->addItem(led);
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    led->setSelected(true);
    const QString colour = led->color();
    auto *undoStack = workspace.scene()->undoStack();
    const int commands = undoStack->count();

    auto *combo = editor.findChild<QComboBox *>("comboBoxColor");
    combo->setCurrentIndex(combo->findData("Blue"));
    combo->setCurrentIndex(combo->findData("Green"));
    QCOMPARE(led->color(), QString("Green"));
    QCOMPARE(undoStack->count(), commands + 2);

    undoStack->undo();
    QCOMPARE(led->color(), QString("Blue"));
    undoStack->undo();
    QCOMPARE(led->color(), colour);
}

void TestElementEditor::testANewSelectionStartsAnUndoStep()
{
    WorkSpace workspace;
    auto *first = new InputSwitch();
    auto *second = new InputSwitch();
    workspace.scene()->addItem(first);
    workspace.scene()->addItem(second);
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    first->setSelected(true);
    auto *undoStack = workspace.scene()->undoStack();
    const int commands = undoStack->count();

    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    lineEdit->setText("a");
    first->setSelected(false);
    second->setSelected(true);
    second->setSelected(false);
    first->setSelected(true);
    lineEdit->setText("ab");
    QCOMPARE(undoStack->count(), commands + 2);
}

void TestElementEditor::testUndoRestoresTheLock()
{
    WorkSpace workspace;
    auto *input = new InputSwitch();
    workspace.scene()->addItem(input);
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    input->setSelected(true);

    editor.findChild<QCheckBox *>("checkBoxLocked")->click();
    QVERIFY(input->isLocked());
    workspace.scene()->undoStack()->undo();
    QVERIFY(!input->isLocked());
    workspace.scene()->undoStack()->redo();
    QVERIFY(input->isLocked());
}

void TestElementEditor::testUndoRestoresTheValueOfAnInput()
{
    WorkSpace workspace;
    auto *input = new InputSwitch();
    auto *rotary = new InputRotary();
    workspace.scene()->addItem(input);
    workspace.scene()->addItem(rotary);
    rotary->setOutputSize(4);
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    input->setSelected(true);
    rotary->setSelected(true);

    auto *combo = editor.findChild<QComboBox *>("comboBoxValue");
    combo->setCurrentText("1");
    QVERIFY(input->isOn());
    QCOMPARE(rotary->outputValue(), 1);

    workspace.scene()->undoStack()->undo();
    QVERIFY(!input->isOn());
    QCOMPARE(rotary->outputValue(), 0);
    workspace.scene()->undoStack()->redo();
    QVERIFY(input->isOn());
    QCOMPARE(rotary->outputValue(), 1);
}

void TestElementEditor::testEditingAFieldLeavesTheOthers()
{
    // The selection's delays and volumes differ, so their fields show no value; a label typed in leaves them
    WorkSpace workspace;
    auto *early = new Clock();
    auto *late = new Clock();
    early->setDelay(-0.25);
    late->setDelay(0.25);
    auto *quiet = new Buzzer();
    auto *loud = new Buzzer();
    quiet->setVolume(0.2f);
    loud->setVolume(0.8f);
    for (auto *element : QList<Element *>{early, late, quiet, loud}) {
        workspace.scene()->addItem(element);
    }
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());

    workspace.scene()->clearSelection();
    early->setSelected(true);
    late->setSelected(true);
    editor.findChild<QLineEdit *>("lineEditElementLabel")->setText("clocks");
    QCOMPARE(early->label(), QString("clocks"));
    QCOMPARE(early->delay(), -0.25);
    QCOMPARE(late->delay(), 0.25);

    workspace.scene()->clearSelection();
    quiet->setSelected(true);
    loud->setSelected(true);
    editor.findChild<QLineEdit *>("lineEditElementLabel")->setText("buzzers");
    QCOMPARE(quiet->label(), QString("buzzers"));
    QCOMPARE(quiet->volume(), 0.2f);
    QCOMPARE(loud->volume(), 0.8f);
}

void TestElementEditor::testMixedDelaysDragIsOneStep()
{
    // A drag over two clocks with different delays is one undo step: every step of it names both clocks
    WorkSpace workspace;
    auto *early = new Clock();
    auto *late = new Clock();
    late->setDelay(0.125);
    workspace.scene()->addItem(early);
    workspace.scene()->addItem(late);
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    early->setSelected(true);
    late->setSelected(true);
    auto *undoStack = workspace.scene()->undoStack();
    auto *slider = editor.findChild<QSlider *>("sliderDelay");
    QVERIFY(slider);
    QCOMPARE(slider->value(), 0);
    const int before = undoStack->count();

    slider->setSliderDown(true);
    slider->setValue(1);
    slider->setValue(2);
    slider->setSliderDown(false);
    QCOMPARE(early->delay(), 0.25);
    QCOMPARE(late->delay(), 0.25);
    QCOMPARE(undoStack->count() - before, 1);

    undoStack->undo();
    QCOMPARE(early->delay(), 0.0);
    QCOMPARE(late->delay(), 0.125);
}

void TestElementEditor::testMixedLabelsTypingIsOneStep()
{
    // Typing into the label field of two switches whose labels differ is one undo step
    WorkSpace workspace;
    auto *first = new InputSwitch();
    auto *second = new InputSwitch();
    first->setLabel("a");
    second->setLabel("b");
    workspace.scene()->addItem(first);
    workspace.scene()->addItem(second);
    ElementEditor editor(&workspace);
    editor.setScene(workspace.scene());
    workspace.scene()->clearSelection();
    first->setSelected(true);
    second->setSelected(true);
    auto *undoStack = workspace.scene()->undoStack();
    auto *lineEdit = editor.findChild<QLineEdit *>("lineEditElementLabel");
    QVERIFY(lineEdit);
    const int before = undoStack->count();

    lineEdit->setText("a");
    lineEdit->setText("ab");
    QCOMPARE(first->label(), QString("ab"));
    QCOMPARE(second->label(), QString("ab"));
    QCOMPARE(undoStack->count() - before, 1);

    undoStack->undo();
    QCOMPARE(first->label(), QString("a"));
    QCOMPARE(second->label(), QString("b"));
}

void TestElementEditor::testDestroyingTheEditorWithAFocusedField()
{
    // A visible editor window closes as it is destroyed, after its own part is gone, and its focused field reports
    // that it was left: a slot of the editor called then asserts in a debug build (Qt's check that the receiver is
    // still of its type)
    WorkSpace workspace;
    auto *input = new InputSwitch();
    workspace.scene()->addItem(input);
    auto *editor = new ElementEditor();
    editor->setScene(workspace.scene());
    workspace.scene()->clearSelection();
    input->setSelected(true);
    QVERIFY(editor->isWindow());
    // Active as Qt sees it: macOS's windowing does not activate a test's window
    showAndActivate(editor);
    QCOMPARE(QApplication::activeWindow(), editor);

    auto *lineEdit = editor->findChild<QLineEdit *>("lineEditElementLabel");
    lineEdit->setFocus();
    QVERIFY(lineEdit->hasFocus());
    QTest::keyClicks(lineEdit, "x");
    QSignalSpy finished(lineEdit, &QLineEdit::editingFinished);

    delete editor;
    QCOMPARE(finished.count(), 1);
    QCOMPARE(input->label(), QString("x"));
}
