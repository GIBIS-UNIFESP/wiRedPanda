// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bewaveddolphin.h"
#include "graphicsview.h"
#include "scene.h"
#include "workspace.h"

WorkSpace::WorkSpace(QDialog *fullscreenDlg, GraphicsView *fullscreenView, QUndoStack *undoStack, Scene *scene)
  : m_fullscreenDlg(fullscreenDlg)
  , m_fullscreenView(fullscreenView)
  , m_undoStack(undoStack)
  , m_scene(scene)
{
}

WorkSpace::~WorkSpace()
{
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

void WorkSpace::setCurrentFile(QFileInfo &finfo)
{
    m_currentFile = finfo;
}

QFileInfo WorkSpace::currentFile()
{
  return m_currentFile;
}

void WorkSpace::setAutoSaveFileName(QString &fname)
{
    m_autosaveFileName = fname;
}

QString WorkSpace::autoSaveFileName()
{
    return m_autosaveFileName;
}

void WorkSpace::setDolphinFileName(QString &fname)
{
    m_dolphinFilename = fname;
}

QString WorkSpace::dolphinFileName()
{
    return m_dolphinFilename;
}