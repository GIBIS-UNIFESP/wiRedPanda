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
    QDialog *fullScreenDlg() const;
    GraphicsView *fullscreenView() const;
    QUndoStack *undoStack() const;
    Scene *scene() const;
    QGraphicsRectItem *sceneRect();
    SimulationController *simulationController();
    ICManager *icManager();

    void setCurrentFile(const QFileInfo &finfo);
    QFileInfo currentFile();
    void setDolphinFileName(const QString &fname);
    QString dolphinFileName();

    void free();

private:
    QDialog *m_fullscreenDlg;
    GraphicsView *m_fullscreenView;
    QUndoStack *m_undoStack;
    Scene *m_scene;
    SimulationController *m_simulationController;
    ICManager *m_icManager;
    QGraphicsRectItem *m_selectionRect;
    QFileInfo m_currentFile;
    QString m_dolphinFilename;
};
