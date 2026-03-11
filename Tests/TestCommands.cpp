// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TestCommands.h"

#include "And.h"
#include "Commands.h"
#include "Common.h"
#include "RegisterTypes.h"
#include "Scene.h"
#include "Workspace.h"

#include <QApplication>
#include <QTest>

int main(int argc, char **argv)
{
#ifdef Q_OS_LINUX
    qputenv("QT_QPA_PLATFORM", "offscreen");
#endif

    registerTypes();

    Comment::setVerbosity(-1);

    QApplication app(argc, argv);
    app.setOrganizationName("GIBIS-UNIFESP");
    app.setApplicationName("wiRedPanda");
    app.setApplicationVersion(APP_VERSION);

    TestCommands testCommands;
    return QTest::qExec(&testCommands, argc, argv);
}

void TestCommands::testAddDeleteCommands()
{
    QList<QGraphicsItem *> items{new And(), new And(), new And(), new And()};

    WorkSpace workspace;
    auto *scene = workspace.scene();
    QVERIFY(scene != nullptr);
    auto *undoStack = scene->undoStack();
    QVERIFY(undoStack != nullptr);
    undoStack->setUndoLimit(1);
    scene->receiveCommand(new AddItemsCommand(items, scene));

    QCOMPARE(scene->elements().size(), items.size());

    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();
    scene->receiveCommand(new DeleteItemsCommand(scene->items(), scene));
    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();

    QCOMPARE(scene->elements().size(), 0);
    QCOMPARE(undoStack->index(), 1);
}
