// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testcommands.h"

#include "and.h"
#include "commands.h"
#include "editor.h"

#include <QTest>

void TestCommands::testAddDeleteCommands()
{
    /* FIXME: Implementar gerenciamento de memória inteligente. */
    QList<QGraphicsItem *> items;
    items << new And() << new And() << new And() << new And();

    auto *editor = new Editor();
    editor->setupWorkspace();
    editor->getUndoStack()->setUndoLimit(1);
    editor->receiveCommand(new AddItemsCommand(items, editor));

    QCOMPARE(editor->getScene()->getElements().size(), items.size());

    editor->getUndoStack()->undo();
    editor->getUndoStack()->redo();
    editor->getUndoStack()->undo();
    editor->getUndoStack()->redo();
    editor->getUndoStack()->undo();
    editor->getUndoStack()->redo();
    editor->receiveCommand(new DeleteItemsCommand(editor->getScene()->items(), editor));
    editor->getUndoStack()->undo();
    editor->getUndoStack()->redo();
    editor->getUndoStack()->undo();
    editor->getUndoStack()->redo();
    editor->getUndoStack()->undo();
    editor->getUndoStack()->redo();

    QCOMPARE(editor->getScene()->getElements().size(), 0);
    QCOMPARE(editor->getUndoStack()->index(), 1);
}
