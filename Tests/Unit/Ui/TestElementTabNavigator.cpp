// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestElementTabNavigator.h"

#include <QKeyEvent>
#include <QLineEdit>

#include "App/Element/GraphicElements/InputSwitch.h"
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
