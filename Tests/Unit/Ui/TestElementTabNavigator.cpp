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

struct NavigatorFixture {
    WorkSpace workspace;
    ElementEditor editor{&workspace};
    ElementTabNavigator navigator{&editor};
    QLineEdit widget; // enabled stand-in for the editor field the filter is installed on
    InputSwitch *left = nullptr;
    InputSwitch *right = nullptr;

    NavigatorFixture()
    {
        editor.setScene(workspace.scene());

        left = new InputSwitch();
        right = new InputSwitch();
        workspace.scene()->addItem(left);
        workspace.scene()->addItem(right);
        left->setPos(0, 0);
        right->setPos(100, 0);
    }

    bool sendKey(Qt::Key key)
    {
        QKeyEvent event(QEvent::KeyPress, key, Qt::NoModifier);
        return navigator.eventFilter(&widget, &event);
    }
};

} // namespace

void TestElementTabNavigator::testTabNavigation()
{
    NavigatorFixture f;

    f.left->setSelected(true);

    // Tab advances the selection to the next element in reading order
    QVERIFY(f.sendKey(Qt::Key_Tab));
    QVERIFY(f.right->isSelected());
    QVERIFY(!f.left->isSelected());
}

void TestElementTabNavigator::testBacktabNavigation()
{
    NavigatorFixture f;

    f.right->setSelected(true);

    // Backtab moves the selection to the previous element in reading order
    QVERIFY(f.sendKey(Qt::Key_Backtab));
    QVERIFY(f.left->isSelected());
    QVERIFY(!f.right->isSelected());
}

void TestElementTabNavigator::testTabWrapAround()
{
    NavigatorFixture f;

    f.right->setSelected(true);

    // Tab from the last element wraps around to the first
    QVERIFY(f.sendKey(Qt::Key_Tab));
    QVERIFY(f.left->isSelected());
    QVERIFY(!f.right->isSelected());
}
