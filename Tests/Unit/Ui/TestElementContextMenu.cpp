// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementContextMenu.h"

#include <QClipboard>
#include <QComboBox>
#include <QMenu>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/DLatch.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/JKFlipFlop.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/IC.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementContextMenu.h"
#include "App/UI/SelectionCapabilities.h"
#include "Tests/Common/TestUtils.h"

namespace {

// Finds the QMenu shown by ElementContextMenu::exec() and clicks the action with the given
// text. A plain action->trigger() only emits the signal -- it does not drive QMenuPrivate's
// internal close/return machinery, so exec() itself would never unblock (see ElementEditor.cpp
// sweep's identical finding). A real synthetic mouse click on the item's geometry goes through
// QMenu's own event handling, which both activates the action and makes exec() return it --
// but the click must be preceded by a mouseMove to the same point first: without it, a click
// landing on a menu that appeared immediately after another one closed can silently fail to
// register on the intended action (confirmed empirically: the "Cut" action was found and
// "clicked" at its correct geometry, yet neither the scene nor the undo stack changed, only
// when run after another exec()+click in the same test; a leading mouseMove to prime QMenu's
// own hover/current-action tracking made it reliable in every ordering tried).
// Also handles one-level-nested actions (e.g. the "Change color to..."/"Morph to..."
// submenus): a submenu only becomes its own visible top-level widget once opened, which
// normally only happens on real mouse hover -- hover the parent action (not a manual popup()
// call, which opens the submenu as its own widget but leaves exec()'s internal state unaware
// of it) so a later poll (the submenu is now independently visible) can find and click the
// actual target inside it.
TestUtils::AutoDismisser dismisserForContextMenuAction(const QString &actionText)
{
    return TestUtils::AutoDismisser([actionText](QWidget *w) {
        auto *menu = qobject_cast<QMenu *>(w);
        if (!menu) return false;
        for (auto *action : menu->actions()) {
            if (action->text() == actionText) {
                const QPoint pos = menu->actionGeometry(action).center();
                QTest::mouseMove(menu, pos);
                QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier, pos);
                return true;
            }
            if (action->menu()) {
                QTest::mouseMove(menu, menu->actionGeometry(action).center());
            }
        }
        return false;
    });
}

// Mirrors ElementEditor::fillColorComboBox()'s 5-item shape, minimally, since
// ElementContextMenu::exec() only reads it (currentIndex/count/itemIcon/itemText/setCurrentText).
QComboBox *makeColorCombo(QWidget *parent)
{
    auto *combo = new QComboBox(parent);
    combo->addItem("White");
    combo->addItem("Red");
    combo->addItem("Green");
    combo->addItem("Blue");
    combo->addItem("Purple");
    return combo;
}

} // namespace

void TestElementContextMenu::testRotateRightAction()
{
    // Pins the sign convention the real "Rotate right"/"Rotate left" menu entries rely on
    // (RotateCommand(elements, +90.0/-90.0, scene)); the menu-driven dispatch itself is
    // covered by testRotateLeftAndRightActionsPushCommands() below.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{andGate}, scene));
    const int initialRotation = static_cast<int>(andGate->rotation());

    scene->receiveCommand(new RotateCommand({andGate}, 90.0, scene)); // "Rotate right"
    QCOMPARE(static_cast<int>(andGate->rotation()), (initialRotation + 90) % 360);

    scene->undoStack()->undo();
    scene->receiveCommand(new RotateCommand({andGate}, -90.0, scene)); // "Rotate left"
    // ElementOrientation::setRotation normalizes via std::fmod, which is sign-preserving
    // (unlike a positive-wrapping modulo), so a negative angle stays negative here.
    QCOMPARE(static_cast<int>(andGate->rotation()), initialRotation - 90);
}

void TestElementContextMenu::testMorphMenuAction()
{
    WorkSpace workspace;
    And andGate;
    workspace.scene()->addItem(&andGate);

    SelectionCapabilities caps = computeCapabilities({&andGate});
    QVERIFY(caps.canMorph);
}

void TestElementContextMenu::testICSubcircuitAction()
{
    // The "Edit sub-circuit"/"Embed sub-circuit"/"Extract to file" menu entries are gated by
    // caps.isEmbedded/caps.isFileBacked (App/UI/ElementContextMenu.cpp), computed from
    // IC::isEmbedded() (App/UI/SelectionCapabilities.cpp) — exercise the actual IC states
    // those flags are derived from, not just the generic "no selection" case.
    QVERIFY(!computeCapabilities({}).hasElements);

    WorkSpace workspace;
    And andGate;
    workspace.scene()->addItem(&andGate);
    const SelectionCapabilities nonIcCaps = computeCapabilities({&andGate});
    QVERIFY(!nonIcCaps.isEmbedded);
    QVERIFY(!nonIcCaps.isFileBacked);

    IC fileBackedIc;
    workspace.scene()->addItem(&fileBackedIc);
    const SelectionCapabilities fileBackedCaps = computeCapabilities({&fileBackedIc});
    QVERIFY(!fileBackedCaps.isEmbedded);
    QVERIFY(fileBackedCaps.isFileBacked);

    IC embeddedIc;
    embeddedIc.setBlobName("test_blob");
    workspace.scene()->addItem(&embeddedIc);
    const SelectionCapabilities embeddedCaps = computeCapabilities({&embeddedIc});
    QVERIFY(embeddedCaps.isEmbedded);
    QVERIFY(!embeddedCaps.isFileBacked);
}

void TestElementContextMenu::testDismissWithoutChoosingIsNoOp()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));
    andGate->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const SelectionCapabilities caps = computeCapabilities({andGate});
    auto *undoStack = scene->undoStack();
    const int countBefore = undoStack->count();

    TestUtils::AutoDismisser dismisser([](QWidget *w) {
        auto *menu = qobject_cast<QMenu *>(w);
        if (!menu) return false;
        menu->close(); // dismiss without choosing -- exec() returns nullptr.
        return true;
    });

    ElementContextMenu::exec(QPoint(0, 0), nullptr, caps, {andGate}, colorCombo, scene,
        [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
        [] {}, [] {}, [] {}, [] {}, [] {});

    QCOMPARE(undoStack->count(), countBefore);
}

void TestElementContextMenu::testRotateLeftAndRightActionsPushCommands()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));
    andGate->setSelected(true);
    const int initialRotation = static_cast<int>(andGate->rotation());

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const SelectionCapabilities caps = computeCapabilities({andGate});

    {
        auto dismisser = dismisserForContextMenuAction(QStringLiteral("Rotate right"));
        ElementContextMenu::exec(QPoint(0, 0), nullptr, caps, {andGate}, colorCombo, scene,
            [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
            [] {}, [] {}, [] {}, [] {}, [] {});
    }
    QCOMPARE(static_cast<int>(andGate->rotation()), (initialRotation + 90) % 360);

    {
        auto dismisser = dismisserForContextMenuAction(QStringLiteral("Rotate left"));
        ElementContextMenu::exec(QPoint(0, 0), nullptr, caps, {andGate}, colorCombo, scene,
            [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
            [] {}, [] {}, [] {}, [] {}, [] {});
    }
    QCOMPARE(static_cast<int>(andGate->rotation()), initialRotation);
}

void TestElementContextMenu::testFlipHorizontalAndVerticalActionsPushCommands()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));
    andGate->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const SelectionCapabilities caps = computeCapabilities({andGate});
    auto *undoStack = scene->undoStack();

    {
        const int countBefore = undoStack->count();
        auto dismisser = dismisserForContextMenuAction(QStringLiteral("Flip horizontally"));
        ElementContextMenu::exec(QPoint(0, 0), nullptr, caps, {andGate}, colorCombo, scene,
            [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
            [] {}, [] {}, [] {}, [] {}, [] {});
        QCOMPARE(undoStack->count(), countBefore + 1);
    }

    {
        const int countBefore = undoStack->count();
        auto dismisser = dismisserForContextMenuAction(QStringLiteral("Flip vertically"));
        ElementContextMenu::exec(QPoint(0, 0), nullptr, caps, {andGate}, colorCombo, scene,
            [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
            [] {}, [] {}, [] {}, [] {}, [] {});
        QCOMPARE(undoStack->count(), countBefore + 1);
    }
}

void TestElementContextMenu::testColorSubmenuPopulatesAndDispatches()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *led = new Led();
    scene->receiveCommand(new AddItemsCommand({led}, scene));
    led->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    colorCombo->setCurrentIndex(0); // White
    const SelectionCapabilities caps = computeCapabilities({led});
    QVERIFY(caps.hasColors);

    auto dismisser = dismisserForContextMenuAction(QStringLiteral("Red"));
    ElementContextMenu::exec(QPoint(0, 0), nullptr, caps, {led}, colorCombo, scene,
        [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
        [] {}, [] {}, [] {}, [] {}, [] {});

    QCOMPARE(colorCombo->currentText(), QStringLiteral("Red"));
}

void TestElementContextMenu::testCopyCutDeleteActionsDelegateToScene()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);

    // Copy: the scene's selection is serialized onto the real clipboard.
    auto *copySubject = new And();
    scene->receiveCommand(new AddItemsCommand({copySubject}, scene));
    copySubject->setSelected(true);
    {
        const SelectionCapabilities caps = computeCapabilities({copySubject});
        QVERIFY(caps.hasElements);
        auto dismisser = dismisserForContextMenuAction(QStringLiteral("Copy"));
        ElementContextMenu::exec(QPoint(0, 0), nullptr, caps, {copySubject}, colorCombo, scene,
            [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
            [] {}, [] {}, [] {}, [] {}, [] {});
    }
    QVERIFY(QApplication::clipboard()->mimeData());

    // Cut: the element leaves the scene (undoable removal).
    scene->clearSelection();
    auto *cutSubject = new And();
    scene->receiveCommand(new AddItemsCommand({cutSubject}, scene));
    scene->clearSelection();
    cutSubject->setSelected(true);
    QCOMPARE(scene->selectedElements().size(), 1);
    const int countBeforeCut = static_cast<int>(scene->elements().size());
    {
        const SelectionCapabilities caps = computeCapabilities({cutSubject});
        auto dismisser = dismisserForContextMenuAction(QStringLiteral("Cut"));
        ElementContextMenu::exec(QPoint(0, 0), nullptr, caps, {cutSubject}, colorCombo, scene,
            [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
            [] {}, [] {}, [] {}, [] {}, [] {});
    }
    QCOMPARE(static_cast<int>(scene->elements().size()), countBeforeCut - 1);

    // Delete: same real removal, via the dedicated Delete action.
    scene->clearSelection();
    auto *deleteSubject = new And();
    scene->receiveCommand(new AddItemsCommand({deleteSubject}, scene));
    scene->clearSelection();
    deleteSubject->setSelected(true);
    const int countBeforeDelete = static_cast<int>(scene->elements().size());
    {
        const SelectionCapabilities caps = computeCapabilities({deleteSubject});
        auto dismisser = dismisserForContextMenuAction(QStringLiteral("Delete"));
        ElementContextMenu::exec(QPoint(0, 0), nullptr, caps, {deleteSubject}, colorCombo, scene,
            [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
            [] {}, [] {}, [] {}, [] {}, [] {});
    }
    QCOMPARE(static_cast<int>(scene->elements().size()), countBeforeDelete - 1);
}

namespace {

// Drives ElementContextMenu::exec() far enough to capture the "Morph to..." submenu's action
// texts (if any), then dismisses without choosing. `itemAtMouse` doubles as the sole selected
// element (matches ElementEditor::contextMenu()'s real single-element-under-cursor usage).
QStringList captureMorphSubmenuTexts(GraphicElement *itemAtMouse, Scene *scene, QComboBox *colorCombo)
{
    const SelectionCapabilities caps = computeCapabilities({itemAtMouse});

    QStringList texts;
    TestUtils::AutoDismisser dismisser([&](QWidget *w) {
        auto *menu = qobject_cast<QMenu *>(w);
        if (!menu) return false;
        for (auto *topAction : menu->actions()) {
            if (topAction->menu() && topAction->text() == QStringLiteral("Morph to...")) {
                for (auto *subAction : topAction->menu()->actions()) {
                    texts << subAction->text();
                }
            }
        }
        menu->close();
        return true;
    });

    ElementContextMenu::exec(QPoint(0, 0), itemAtMouse, caps, {itemAtMouse}, colorCombo, scene,
        [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
        [] {}, [] {}, [] {}, [] {}, [] {});

    return texts;
}

} // namespace

void TestElementContextMenu::testMorphSubmenuGateSingleInput()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *notGate = new Not();
    scene->receiveCommand(new AddItemsCommand({notGate}, scene));
    notGate->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const QStringList texts = captureMorphSubmenuTexts(notGate, scene, colorCombo);

    QVERIFY2(texts.contains(ElementFactory::translatedName(ElementType::Node)), qPrintable(texts.join(", ")));
    QVERIFY2(!texts.contains(ElementFactory::translatedName(ElementType::Not)), "same-type target must be excluded");
}

void TestElementContextMenu::testMorphSubmenuGateMultiInputExcludesSameType()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));
    andGate->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const QStringList texts = captureMorphSubmenuTexts(andGate, scene, colorCombo);

    QVERIFY2(texts.contains(ElementFactory::translatedName(ElementType::Or)), qPrintable(texts.join(", ")));
    QVERIFY2(!texts.contains(ElementFactory::translatedName(ElementType::And)), "same-type target (And) must be excluded");
}

void TestElementContextMenu::testMorphSubmenuInputGroup()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *clock = new Clock();
    scene->receiveCommand(new AddItemsCommand({clock}, scene));
    clock->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const QStringList texts = captureMorphSubmenuTexts(clock, scene, colorCombo);

    QVERIFY2(texts.contains(ElementFactory::translatedName(ElementType::InputSwitch)), qPrintable(texts.join(", ")));
    QVERIFY2(!texts.contains(ElementFactory::translatedName(ElementType::Clock)), "same-type target (Clock) must be excluded");
}

void TestElementContextMenu::testMorphSubmenuMemoryTwoInputEmptiesSubmenu()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *dLatch = new DLatch();
    scene->receiveCommand(new AddItemsCommand({dLatch}, scene));
    dLatch->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    // DLatch (2 inputs) offers only itself as a morph target, which addElementAction()
    // then excludes for being the same type -- the submenu ends up empty.
    const QStringList texts = captureMorphSubmenuTexts(dLatch, scene, colorCombo);

    QVERIFY(texts.isEmpty());
}

void TestElementContextMenu::testMorphSubmenuMemoryFourInput()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *dff = new DFlipFlop();
    scene->receiveCommand(new AddItemsCommand({dff}, scene));
    dff->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const QStringList texts = captureMorphSubmenuTexts(dff, scene, colorCombo);

    QVERIFY2(texts.contains(ElementFactory::translatedName(ElementType::TFlipFlop)), qPrintable(texts.join(", ")));
    QVERIFY2(!texts.contains(ElementFactory::translatedName(ElementType::DFlipFlop)), "same-type target (DFlipFlop) must be excluded");
}

void TestElementContextMenu::testMorphSubmenuMemoryFiveInput()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *jkff = new JKFlipFlop();
    scene->receiveCommand(new AddItemsCommand({jkff}, scene));
    jkff->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const QStringList texts = captureMorphSubmenuTexts(jkff, scene, colorCombo);

    QVERIFY2(texts.contains(ElementFactory::translatedName(ElementType::SRFlipFlop)), qPrintable(texts.join(", ")));
    QVERIFY2(!texts.contains(ElementFactory::translatedName(ElementType::JKFlipFlop)), "same-type target (JKFlipFlop) must be excluded");
}

void TestElementContextMenu::testMorphSubmenuOutputDisplay()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *display = new Display7();
    scene->receiveCommand(new AddItemsCommand({display}, scene));
    display->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const QStringList texts = captureMorphSubmenuTexts(display, scene, colorCombo);

    QVERIFY2(texts.contains(ElementFactory::translatedName(ElementType::Display14)), qPrintable(texts.join(", ")));
    QVERIFY2(texts.contains(ElementFactory::translatedName(ElementType::Display16)), qPrintable(texts.join(", ")));
    QVERIFY2(!texts.contains(ElementFactory::translatedName(ElementType::Display7)), "same-type target (Display7) must be excluded");
}

void TestElementContextMenu::testMorphSubmenuOutputNonDisplay()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *led = new Led();
    scene->receiveCommand(new AddItemsCommand({led}, scene));
    led->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const QStringList texts = captureMorphSubmenuTexts(led, scene, colorCombo);

    QVERIFY2(texts.contains(ElementFactory::translatedName(ElementType::Buzzer)), qPrintable(texts.join(", ")));
    QVERIFY2(!texts.contains(ElementFactory::translatedName(ElementType::Led)), "same-type target (Led) must be excluded");
}

void TestElementContextMenu::testMorphSubmenuGroupWithNoMorphTargetsRemovesSubmenu()
{
    // Mux (ElementGroup::Mux) falls through every case in the morph switch to `break;` with
    // nothing added -- the "Morph to..." top-level action itself must be removed, not just
    // left as an empty submenu.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *mux = new Mux();
    scene->receiveCommand(new AddItemsCommand({mux}, scene));
    mux->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const SelectionCapabilities caps = computeCapabilities({mux});

    bool morphActionSeen = false;
    TestUtils::AutoDismisser dismisser([&](QWidget *w) {
        auto *menu = qobject_cast<QMenu *>(w);
        if (!menu) return false;
        for (auto *topAction : menu->actions()) {
            if (topAction->text() == QStringLiteral("Morph to...")) {
                morphActionSeen = true;
            }
        }
        menu->close();
        return true;
    });

    ElementContextMenu::exec(QPoint(0, 0), mux, caps, {mux}, colorCombo, scene,
        [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
        [] {}, [] {}, [] {}, [] {}, [] {});

    QVERIFY(!morphActionSeen);
}

void TestElementContextMenu::testMorphActionDispatchesMorphCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));
    andGate->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const SelectionCapabilities caps = computeCapabilities({andGate});

    auto dismisser = dismisserForContextMenuAction(ElementFactory::translatedName(ElementType::Or));
    ElementContextMenu::exec(QPoint(0, 0), andGate, caps, {andGate}, colorCombo, scene,
        [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
        [] {}, [] {}, [] {}, [] {}, [] {});

    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(scene->elements().constFirst()->elementType(), ElementType::Or);
}

void TestElementContextMenu::testUnknownMenuOptionThrows()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));
    andGate->setSelected(true);

    QWidget parent;
    auto *colorCombo = makeColorCombo(&parent);
    const SelectionCapabilities caps = computeCapabilities({andGate});

    // Inject a rogue action with unrecognized text/data into the real menu, then click it --
    // none of exec()'s dispatch checks match, so it falls through to the final throw.
    TestUtils::AutoDismisser dismisser([](QWidget *w) {
        auto *menu = qobject_cast<QMenu *>(w);
        if (!menu) return false;
        auto *rogue = menu->addAction(QStringLiteral("Definitely not a real option"));
        QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(rogue).center());
        return true;
    });

    QVERIFY_THROWS_EXCEPTION(std::exception, ElementContextMenu::exec(QPoint(0, 0), nullptr, caps, {andGate}, colorCombo, scene,
        [scene](QUndoCommand *cmd) { scene->receiveCommand(cmd); },
        [] {}, [] {}, [] {}, [] {}, [] {}));
}
