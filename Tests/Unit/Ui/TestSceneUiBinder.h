// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestSceneUiBinder : public QObject
{
    Q_OBJECT

private slots:

    // The private guard methods (updateStatusInfo/addUndoRedoMenu/removeUndoRedoMenu/
    // syncZoomActions) are only ever invoked through signal connections established by
    // bind(), which always sets m_bound first -- calling them directly (via friend access)
    // before any bind() is the only way to exercise their "not bound yet" guards.
    void testGuardMethodsAreNoOpBeforeAnyBind();

    // Regression seam: a menuEdit with fewer than 2 actions (never happens with a real
    // MainWindowUi::setupUi(), but nothing in SceneUiBinder enforces that) must skip both
    // inserting and removing the undo/redo actions rather than indexing out of range.
    void testBindWithTooFewMenuActionsSkipsUndoRedoInsertionAndRemoval();

    void testBindConnectsSimulationWarningToStatusBar();
    void testBindConnectsShowStatusMessageRequestedToStatusBar();
    void testBindConnectsOpenTruthTableRequestedToElementEditor();
    void testBindForwardsIcOpenRequestedWithBlobName();
    void testBindForwardsIcOpenRequestedWithFilePathOnly();
    void testBindForwardsFileDropRequestedToLoadFileRequested();
    void testBindShowsIcPreviewOnPreviewRequestedAndOnMoveWhenNotAlreadyActive();
    void testBindCancelsIcPreviewOnCancelRequested();
};
