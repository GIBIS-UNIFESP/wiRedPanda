/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsRectItem>
#include <QTemporaryFile>
#include <QUndoStack>

class BewavedDolphin;
class Editor;
class GraphicsView;
class ICManager;
class Scene;
class SimulationController;

class WorkSpace
{
public:
    WorkSpace() = default; // for compiling on Qt 5.7+
    WorkSpace(QDialog *fullscreenDlg, GraphicsView *fullscreenView, Editor *editor);

    GraphicsView *fullscreenView() const;
    ICManager *icManager();
    QDialog *fullScreenDlg() const;
    QFileInfo currentFile();
    QGraphicsRectItem *sceneRect();
    QString dolphinFileName();
    QUndoStack *undoStack() const;
    Scene *scene() const;
    SimulationController *simulationController();
    void free();
    void setCurrentFile(const QFileInfo &finfo);
    void setDolphinFileName(const QString &fname);

private:
    GraphicsView *m_fullscreenView;
    ICManager *m_icManager;
    QDialog *m_fullscreenDlg;
    QFileInfo m_currentFile;
    QGraphicsRectItem *m_selectionRect;
    QString m_dolphinFilename;
    QUndoStack *m_undoStack;
    Scene *m_scene;
    SimulationController *m_simulationController;
};
