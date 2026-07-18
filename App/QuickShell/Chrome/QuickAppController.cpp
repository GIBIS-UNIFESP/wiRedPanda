// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickAppController.h"

#include <QUndoStack>

#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"

QuickAppController::QuickAppController(QObject *parent)
    : QObject(parent)
    , m_exportController(*this)
{
    connect(&m_workspaceManager, &QuickWorkspaceManager::currentTabChanged, this, [this] {
        bindCurrentTab();
        emit currentTabChanged();
        emit windowTitleChanged();
        emit undoRedoStateChanged();
    });
    connect(&m_workspaceManager, &QuickWorkspaceManager::tabsChanged, this, &QuickAppController::tabsChanged);
    connect(&m_workspaceManager, &QuickWorkspaceManager::titleChanged, this, &QuickAppController::windowTitleChanged);
}

QuickWorkSpace *QuickAppController::currentTab() const
{
    return m_workspaceManager.currentTab();
}

QFileInfo QuickAppController::currentFile() const
{
    return m_workspaceManager.currentFile();
}

QDir QuickAppController::currentDir() const
{
    return m_workspaceManager.currentDir();
}

void QuickAppController::showStatusMessage(const QString & /*message*/, int /*timeout*/)
{
    // No-op until a status bar exists (sub-step 3's remaining chrome / sub-step 7).
}

CanvasItem *QuickAppController::activeCanvas() const
{
    auto *tab = currentTab();
    return tab ? tab->canvas() : nullptr;
}

void QuickAppController::bindCurrentTab()
{
    for (const auto &connection : std::as_const(m_tabConnections)) {
        disconnect(connection);
    }
    m_tabConnections.clear();

    auto *canvas = activeCanvas();
    if (!canvas) {
        return;
    }

    auto *undoStack = canvas->undoStack();
    const auto reemit = [this] {
        emit undoRedoStateChanged();
        emit windowTitleChanged();
    };
    m_tabConnections.append(connect(undoStack, &QUndoStack::canUndoChanged, this, reemit));
    m_tabConnections.append(connect(undoStack, &QUndoStack::canRedoChanged, this, reemit));
    m_tabConnections.append(connect(undoStack, &QUndoStack::undoTextChanged, this, reemit));
    m_tabConnections.append(connect(undoStack, &QUndoStack::redoTextChanged, this, reemit));
    m_tabConnections.append(connect(undoStack, &QUndoStack::cleanChanged, this, reemit));
}

QString QuickAppController::windowTitle() const
{
    auto *tab = currentTab();
    if (!tab) {
        return QStringLiteral("wiRedPanda " APP_VERSION);
    }
    QString title = m_workspaceManager.tabTitle(tab);
    if (!tab->canvas()->undoStack()->isClean()) {
        title += QStringLiteral("*");
    }
    return title + QStringLiteral(" — wiRedPanda " APP_VERSION);
}

bool QuickAppController::canUndo() const
{
    auto *c = activeCanvas();
    return c && c->undoStack()->canUndo();
}

bool QuickAppController::canRedo() const
{
    auto *c = activeCanvas();
    return c && c->undoStack()->canRedo();
}

QString QuickAppController::undoText() const
{
    auto *c = activeCanvas();
    return c ? c->undoStack()->undoText() : QString();
}

QString QuickAppController::redoText() const
{
    auto *c = activeCanvas();
    return c ? c->undoStack()->redoText() : QString();
}

QString QuickAppController::shortcutsHelpHtml() const
{
    // QString::arg()'s multi-argument overload maxes out at 9 placeholders per call -- 11
    // needed here, so chained across two calls.
    return QStringLiteral(
        "<table>"
        "<tr><td>Ctrl+N</td><td>%1</td></tr>"
        "<tr><td>Ctrl+O</td><td>%2</td></tr>"
        "<tr><td>Ctrl+S</td><td>%3</td></tr>"
        "<tr><td>Ctrl+Z</td><td>%4</td></tr>"
        "<tr><td>Ctrl+Shift+Z</td><td>%5</td></tr>"
        "<tr><td>Ctrl+X / C / V / D</td><td>%6</td></tr>"
        "<tr><td>Del</td><td>%7</td></tr>"
        "<tr><td>Ctrl+A</td><td>%8</td></tr>"
        "<tr><td>Ctrl+R / Ctrl+Shift+R</td><td>%9</td></tr>"
        "<tr><td>Ctrl+H</td><td>%10</td></tr>"
        "<tr><td>[ ] { } &lt; &gt;</td><td>%11</td></tr>"
        "</table>")
        .arg(tr("New project"), tr("Open file"), tr("Save file"), tr("Undo"), tr("Redo"),
             tr("Cut / Copy / Paste / Duplicate"), tr("Delete selection"), tr("Select all"),
             tr("Rotate right / left"))
        .arg(tr("Flip horizontally"), tr("Cycle element type / properties (selected element)"));
}

void QuickAppController::undo()
{
    if (auto *c = activeCanvas()) {
        c->undoStack()->undo();
    }
}

void QuickAppController::redo()
{
    if (auto *c = activeCanvas()) {
        c->undoStack()->redo();
    }
}

void QuickAppController::cut()
{
    if (auto *c = activeCanvas()) c->cutAction();
}

void QuickAppController::copy()
{
    if (auto *c = activeCanvas()) c->copyAction();
}

void QuickAppController::paste()
{
    if (auto *c = activeCanvas()) c->pasteAction();
}

void QuickAppController::duplicateSelection()
{
    if (auto *c = activeCanvas()) c->duplicateAction();
}

void QuickAppController::deleteSelection()
{
    if (auto *c = activeCanvas()) c->deleteSelected();
}

void QuickAppController::selectAll()
{
    if (auto *c = activeCanvas()) c->selectAll();
}

void QuickAppController::rotateRight()
{
    if (auto *c = activeCanvas()) c->rotateRight();
}

void QuickAppController::rotateLeft()
{
    if (auto *c = activeCanvas()) c->rotateLeft();
}

void QuickAppController::flipHorizontal()
{
    if (auto *c = activeCanvas()) c->flipHorizontally();
}

void QuickAppController::flipVertical()
{
    if (auto *c = activeCanvas()) c->flipVertically();
}

void QuickAppController::mute(bool muted)
{
    if (auto *c = activeCanvas()) c->mute(muted);
}

void QuickAppController::restartSimulation()
{
    if (auto *c = activeCanvas()) c->restartSimulation();
}

void QuickAppController::alignLeft()
{
    if (auto *c = activeCanvas()) c->alignLeft();
}

void QuickAppController::alignRight()
{
    if (auto *c = activeCanvas()) c->alignRight();
}

void QuickAppController::alignTop()
{
    if (auto *c = activeCanvas()) c->alignTop();
}

void QuickAppController::alignBottom()
{
    if (auto *c = activeCanvas()) c->alignBottom();
}

void QuickAppController::alignHorizontalCenter()
{
    if (auto *c = activeCanvas()) c->alignHorizontalCenter();
}

void QuickAppController::alignVerticalCenter()
{
    if (auto *c = activeCanvas()) c->alignVerticalCenter();
}

void QuickAppController::distributeHorizontally()
{
    if (auto *c = activeCanvas()) c->distributeHorizontally();
}

void QuickAppController::distributeVertically()
{
    if (auto *c = activeCanvas()) c->distributeVertically();
}
