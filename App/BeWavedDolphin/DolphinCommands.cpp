// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/DolphinCommands.h"

#include "App/BeWavedDolphin/SignalModel.h"

SetCellsCommand::SetCellsCommand(SignalModel *model, const QList<QPair<int, int>> &cells,
                                   const QList<int> &oldValues, std::function<void()> onApplied,
                                   QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_model(model)
    , m_cells(cells)
    , m_oldValues(oldValues)
    , m_onApplied(std::move(onApplied))
{
    setText(QObject::tr("Edit waveform"));

    // The caller already applied the edit before constructing this command (same contract as
    // the main Scene's UpdateCommand) -- snapshot the resulting values as "new" here.
    m_newValues.reserve(m_cells.size());
    for (const auto &cell : m_cells) {
        m_newValues.append(m_model->value(cell.first, cell.second));
    }
}

void SetCellsCommand::redo()
{
    applyValues(m_newValues);
}

void SetCellsCommand::undo()
{
    applyValues(m_oldValues);
}

void SetCellsCommand::applyValues(const QList<int> &values)
{
    for (int i = 0; i < m_cells.size(); ++i) {
        m_model->setValue(m_cells.at(i).first, m_cells.at(i).second, values.at(i));
    }

    if (m_onApplied) {
        m_onApplied();
    }
}
