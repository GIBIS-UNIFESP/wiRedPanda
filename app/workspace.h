/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <qdialog.h>
#include <qdir.h>
#include <qfileinfo.h>
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
    ~WorkSpace();
    QDialog *fullScreenDlg() const;
    GraphicsView *fullscreenView() const;
    QUndoStack *undoStack() const;
    Scene *scene() const;
    SimulationController *simulationController();
    ICManager *icManager();

    void setCurrentFile(QFileInfo &finfo);
    QFileInfo currentFile();
    void setAutoSaveFileName(QString &fname);
    QString autoSaveFileName();
    void setDolphinFileName(QString &fname);
    QString dolphinFileName();

private:
    QDialog *m_fullscreenDlg;
    GraphicsView *m_fullscreenView;
    QUndoStack *m_undoStack;
    Scene *m_scene;
    SimulationController *m_simulationController;
    ICManager *m_icManager;
    QFileInfo m_currentFile;
    QString m_autosaveFileName;
    QString m_dolphinFilename;

public:
    //QTemporaryFile m_autoSaveFile;

};

#endif /* WORKSPACE_H */
