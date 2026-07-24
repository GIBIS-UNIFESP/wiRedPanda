// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementTabNavigator.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QVector>

#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/ElementEditor.h"
#include "App/UI/ElementTabNavigator.h"
#include "Tests/Common/TestUtils.h"

namespace {

// eventFilter() reads scene->visibleElements(), which is clipped to the attached
// GraphicsView's viewport rect — an unshown WorkSpace's default widget geometry
// is too small to contain elements spread out enough to have a deterministic
// reading order, so give the view a generous fixed size up front.
void makeViewLargeEnoughToShowEverything(WorkSpace &workspace)
{
    workspace.view()->resize(2000, 2000);
    QCoreApplication::processEvents();
}

// Selects \a elm and processes events so ElementEditor::setCurrentElements() (connected to
// Scene::selectionChanged) picks it up, then returns its now-current label field — the same
// field ElementEditor installs this navigator on (installEventFilter(m_tabNavigator) in
// ElementEditor's constructor). InputSwitch is used (rather than a basic gate like And, which
// has hasLabel() == false) because eventFilter() requires the returned widget to be enabled.
QLineEdit *selectAndGetLabelField(Scene *scene, ElementEditor *editor, GraphicElement *elm)
{
    scene->clearSelection();
    elm->setSelected(true);
    QCoreApplication::processEvents();
    return editor->findChild<QLineEdit *>("lineEditElementLabel");
}

} // namespace

void TestElementTabNavigator::testReadingOrderIsRowMajor()
{
    // Two rows of two elements. Reading order must be row-major (top row left-to-right, then
    // bottom row left-to-right): a, b, c, d. The pre-fix code used X as the primary sort key
    // and would return column-major a, c, b, d — this is the direct guard for that bug.
    Node a, b, c, d;
    a.setPos(0, 0);      // top-left
    b.setPos(100, 0);    // top-right
    c.setPos(0, 100);    // bottom-left
    d.setPos(100, 100);  // bottom-right

    const QVector<GraphicElement *> scrambled = {&d, &b, &c, &a};
    const QVector<GraphicElement *> expected  = {&a, &b, &c, &d};
    QCOMPARE(ElementTabNavigator::readingOrder(scrambled), expected);
}

void TestElementTabNavigator::testReadingOrderTieBreaksLeftToRight()
{
    // Elements sharing a row (equal Y) are ordered left-to-right by X.
    Node left, middle, right;
    left.setPos(0, 50);
    middle.setPos(100, 50);
    right.setPos(200, 50);

    const QVector<GraphicElement *> scrambled = {&right, &left, &middle};
    const QVector<GraphicElement *> expected  = {&left, &middle, &right};
    QCOMPARE(ElementTabNavigator::readingOrder(scrambled), expected);
}

void TestElementTabNavigator::testReadingOrderEmptyAndSingle()
{
    QVERIFY(ElementTabNavigator::readingOrder({}).isEmpty());

    Node only;
    only.setPos(42, 42);
    const QVector<GraphicElement *> one = {&only};
    QCOMPARE(ElementTabNavigator::readingOrder(one), one);
}

void TestElementTabNavigator::testTabNavigation()
{
    // Tab, while a single element is selected, advances the selection to the next element in
    // reading order (left-to-right) and moves focus to the same field on it
    // (ElementTabNavigator::eventFilter).
    WorkSpace workspace;
    makeViewLargeEnoughToShowEverything(workspace);
    auto *scene = workspace.scene();
    auto *left = new InputSwitch();
    auto *right = new InputSwitch();
    left->setPos(0, 0);
    right->setPos(200, 0);
    scene->addItem(left);
    scene->addItem(right);

    ElementEditor editor(&workspace);
    editor.setScene(scene);
    ElementTabNavigator navigator(&editor);

    auto *labelEdit = selectAndGetLabelField(scene, &editor, left);
    QVERIFY(labelEdit);
    QVERIFY(labelEdit->isEnabled());

    QKeyEvent tabEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QVERIFY(navigator.eventFilter(labelEdit, &tabEvent));

    QVERIFY(!left->isSelected());
    QVERIFY(right->isSelected());
}

void TestElementTabNavigator::testBacktabNavigation()
{
    // Shift+Tab (Qt::Key_Backtab) moves the selection to the previous element in reading order.
    WorkSpace workspace;
    makeViewLargeEnoughToShowEverything(workspace);
    auto *scene = workspace.scene();
    auto *left = new InputSwitch();
    auto *right = new InputSwitch();
    left->setPos(0, 0);
    right->setPos(200, 0);
    scene->addItem(left);
    scene->addItem(right);

    ElementEditor editor(&workspace);
    editor.setScene(scene);
    ElementTabNavigator navigator(&editor);

    auto *labelEdit = selectAndGetLabelField(scene, &editor, right);
    QVERIFY(labelEdit);
    QVERIFY(labelEdit->isEnabled());

    QKeyEvent backtabEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    QVERIFY(navigator.eventFilter(labelEdit, &backtabEvent));

    QVERIFY(!right->isSelected());
    QVERIFY(left->isSelected());
}

void TestElementTabNavigator::testTabWrapAround()
{
    // With only two elements, Tab from the last one in reading order wraps back to the first
    // ((total + elmPos + step) % total in ElementTabNavigator::eventFilter).
    WorkSpace workspace;
    makeViewLargeEnoughToShowEverything(workspace);
    auto *scene = workspace.scene();
    auto *left = new InputSwitch();
    auto *right = new InputSwitch();
    left->setPos(0, 0);
    right->setPos(200, 0);
    scene->addItem(left);
    scene->addItem(right);

    ElementEditor editor(&workspace);
    editor.setScene(scene);
    ElementTabNavigator navigator(&editor);

    auto *labelEdit = selectAndGetLabelField(scene, &editor, right);
    QVERIFY(labelEdit);
    QVERIFY(labelEdit->isEnabled());

    QKeyEvent tabEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QVERIFY(navigator.eventFilter(labelEdit, &tabEvent));

    QVERIFY(!right->isSelected());
    QVERIFY(left->isSelected());
}

void TestElementTabNavigator::testEventFilterIgnoresNonSingleSelection()
{
    // Tab navigation only makes sense while exactly one element is being edited; with zero or
    // two+ elements selected, eventFilter() must decline the event so it falls through to
    // default handling instead.
    WorkSpace workspace;
    makeViewLargeEnoughToShowEverything(workspace);
    auto *scene = workspace.scene();
    auto *left = new InputSwitch();
    auto *right = new InputSwitch();
    left->setPos(0, 0);
    right->setPos(200, 0);
    scene->addItem(left);
    scene->addItem(right);

    ElementEditor editor(&workspace);
    editor.setScene(scene);
    ElementTabNavigator navigator(&editor);

    left->setSelected(true);
    right->setSelected(true);
    QCoreApplication::processEvents();

    QKeyEvent tabEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QVERIFY(!navigator.eventFilter(&editor, &tabEvent));

    // Selection is untouched -- the event was declined, not acted on.
    QVERIFY(left->isSelected());
    QVERIFY(right->isSelected());
}

void TestElementTabNavigator::testEventFilterIgnoresKeyPressWithNoScene()
{
    // A stale single-element selection can outlive its scene (e.g. ElementEditor::setScene()
    // is called with nullptr when the active tab changes, per its own documented contract, and
    // m_elements is not cleared as part of that). eventFilter() must guard against a null scene
    // rather than dereferencing it.
    WorkSpace workspace;
    makeViewLargeEnoughToShowEverything(workspace);
    auto *scene = workspace.scene();
    auto *elm = new InputSwitch();
    elm->setPos(0, 0);
    scene->addItem(elm);

    ElementEditor editor(&workspace);
    editor.setScene(scene);
    ElementTabNavigator navigator(&editor);

    auto *labelEdit = selectAndGetLabelField(scene, &editor, elm);
    QVERIFY(labelEdit);

    editor.setScene(nullptr);

    QKeyEvent tabEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QVERIFY(!navigator.eventFilter(labelEdit, &tabEvent));
}

void TestElementTabNavigator::testEventFilterIgnoresFocusedElementNotInVisibleElements()
{
    // scene->visibleElements() is clipped to the view's viewport rect, independent of
    // ElementEditor's own tracked selection. If the currently-edited element falls outside
    // that rect, it can't appear in readingOrder()'s list -- eventFilter() must fall through
    // to default handling rather than looping forever or indexing at -1.
    WorkSpace workspace;
    workspace.view()->resize(200, 200);
    QCoreApplication::processEvents();
    auto *scene = workspace.scene();
    auto *elm = new InputSwitch();
    elm->setPos(1e6, 1e6); // far outside the small view's visible rect
    scene->addItem(elm);

    ElementEditor editor(&workspace);
    editor.setScene(scene);
    ElementTabNavigator navigator(&editor);

    auto *labelEdit = selectAndGetLabelField(scene, &editor, elm);
    QVERIFY(labelEdit);
    QVERIFY(scene->visibleElements().isEmpty());

    QKeyEvent tabEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QVERIFY(!navigator.eventFilter(labelEdit, &tabEvent));

    // The out-of-view element remains selected -- the event was declined, not acted on.
    QVERIFY(elm->isSelected());
}

void TestElementTabNavigator::testTabRevertsToOriginalElementWhenNoOtherFieldIsEnabled()
{
    // With only two elements, if the other one doesn't support the currently-focused field at
    // all (And has no label, unlike InputSwitch), the "advance until enabled" loop cycles all
    // the way back around without ever breaking -- eventFilter() must then revert to the
    // originally-focused element rather than leaving the disabled field's element selected.
    WorkSpace workspace;
    makeViewLargeEnoughToShowEverything(workspace);
    auto *scene = workspace.scene();
    auto *inputSwitch = new InputSwitch();
    auto *andGate = new And();
    inputSwitch->setPos(0, 0);
    andGate->setPos(200, 0);
    scene->addItem(inputSwitch);
    scene->addItem(andGate);

    ElementEditor editor(&workspace);
    editor.setScene(scene);
    ElementTabNavigator navigator(&editor);

    auto *labelEdit = selectAndGetLabelField(scene, &editor, inputSwitch);
    QVERIFY(labelEdit);
    QVERIFY(labelEdit->isEnabled());
    QVERIFY(!andGate->hasLabel());

    QKeyEvent tabEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QVERIFY(navigator.eventFilter(labelEdit, &tabEvent));

    QVERIFY(inputSwitch->isSelected());
    QVERIFY(!andGate->isSelected());
}
