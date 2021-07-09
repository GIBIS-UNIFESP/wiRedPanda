/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <qdialog.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <QGraphicsRectItem>
#include <qtemporaryfile.h>
#include <qundostack.h>

#ifndef WORKSPACE_H
#define WORKSPACE_H

class BewavedDolphin;
class Editor;
class GraphicsView;
class Scene;
class SimulationController;
class ICManager;

class WorkSpace
{
public:
    WorkSpace(QDialog *fullscreenDlg, GraphicsView *fullscreenView, Editor *editor);
    QDialog *fullScreenDlg() const;
    GraphicsView *fullscreenView() const;
    QUndoStack *undoStack() const;
    Scene *scene() const;
    QGraphicsRectItem *sceneRect();
    SimulationController *simulationController();
    ICManager *icManager();

    void setCurrentFile(QFileInfo &finfo);
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

#endif /* WORKSPACE_H */
