// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DolphinCommands: undo/redo commands for the beWavedDolphin waveform editor.
 */

#pragma once

#include <functional>

#include <QList>
#include <QPair>
#include <QUndoCommand>

class SignalModel;

/**
 * \class SetCellsCommand
 * \brief Generic undo command for any DolphinEdits mutation (Set 0/1, Invert, Clear,
 * clock/combinational fill, and the value-filling half of a length increase).
 *
 * \details Mirrors the main Scene's UpdateCommand: the caller applies the edit *before*
 * constructing this command, so the constructor captures the post-edit ("new") values
 * directly from the model, while the pre-edit ("old") values must be snapshotted by the
 * caller beforehand and passed in. \a cells and both value lists must be the same size and
 * in the same (row, col) order. \a onApplied is invoked after both redo() and undo() so the
 * caller can re-run the simulation and refresh its edited-flag -- this command only owns
 * cell values, not BewavedDolphin's wider state.
 */
class SetCellsCommand : public QUndoCommand
{
public:
    SetCellsCommand(SignalModel *model, const QList<QPair<int, int>> &cells,
                     const QList<int> &oldValues, std::function<void()> onApplied,
                     QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void applyValues(const QList<int> &values);

    SignalModel *m_model;
    QList<QPair<int, int>> m_cells;
    QList<int> m_oldValues;
    QList<int> m_newValues;
    std::function<void()> m_onApplied;
};

/**
 * \class SetLengthCommand
 * \brief Undo step for a change of the simulation length, with the edits made under it as its children.
 *
 * \details The caller changes the length, and then makes the edits, before pushing this command: its children are the
 * edits' commands. Redo sets the new length and then redoes the children; undo undoes the children and then sets the
 * old length, so an edit that grew the grid is undone as one step with the columns it added. \a setLength applies a
 * length and runs the simulation.
 */
class SetLengthCommand : public QUndoCommand
{
public:
    SetLengthCommand(std::function<void(int)> setLength, int oldLength, int newLength, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    std::function<void(int)> m_setLength;
    int m_oldLength;
    int m_newLength;
};
