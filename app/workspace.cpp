// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bewaveddolphin.h"
#include "editor.h"
#include "graphicsview.h"
#include "icmanager.h"
#include "simulationcontroller.h"
#include "workspace.h"

WorkSpace::WorkSpace(QDialog *fullscreenDlg, GraphicsView *fullscreenView, Editor *editor)
    : m_fullscreenDlg(fullscreenDlg)
    , m_fullscreenView(fullscreenView)
    , m_undoStack(editor->getUndoStack())
    , m_scene(editor->getScene())
    , m_simulationController(editor->getSimulationController())
    , m_icManager(editor->getICManager())
    , m_selectionRect(editor->getSceneRect())
{
}

void WorkSpace::free()
{
    delete m_selectionRect;
    m_undoStack->deleteLater();
    m_scene->deleteLater();
    m_simulationController->deleteLater();
    m_icManager->deleteLater();
}

QDialog *WorkSpace::fullScreenDlg() const
{
    return m_fullscreenDlg;
}

GraphicsView *WorkSpace::fullscreenView() const
{
    return m_fullscreenView;
}

QUndoStack *WorkSpace::undoStack() const
{
    return m_undoStack;
}

Scene *WorkSpace::scene() const
{
    return m_scene;
}

SimulationController *WorkSpace::simulationController()
{
    return m_simulationController;
}

ICManager *WorkSpace::icManager()
{
    return m_icManager;
}

QGraphicsRectItem *WorkSpace::sceneRect()
{
    return m_selectionRect;
}

void WorkSpace::setCurrentFile(QFileInfo &finfo)
{
    m_currentFile = finfo;
}

QFileInfo WorkSpace::currentFile()
{
  return m_currentFile;
}

void WorkSpace::setDolphinFileName(const QString &fname)
{
    m_dolphinFilename = fname;
}

QString WorkSpace::dolphinFileName()
{
    return m_dolphinFilename;
}
