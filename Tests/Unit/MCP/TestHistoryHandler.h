// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestHistoryHandler : public QObject
{
    Q_OBJECT

private slots:
    void testHandleUndoRejectsNoScene();
    void testHandleRedoRejectsNoScene();
    void testHandleGetUndoStackRejectsNoScene();

    void testHandleUndoReportsNothingToUndo();
    void testHandleUndoPerformsRealUndo();
    void testHandleRedoReportsNothingToRedo();
    void testHandleRedoPerformsRealRedo();

    void testHandleGetUndoStackReportsRealState();

    void testHandleCommandRejectsUnknownCommand();
};
