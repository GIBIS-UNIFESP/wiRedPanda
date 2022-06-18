/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"
#include "graphicsview.h"
#include "mainwindow.h"
#include "scene.h"

#include <QTemporaryFile>
#include <QUndoStack>

class GraphicsView;
class SimulationController;

class WorkSpace : public QWidget
{
    Q_OBJECT

public:
    explicit WorkSpace(QWidget *parent = nullptr);

    static void selectWorkspace();

    GraphicsView *view();
    QFileInfo fileInfo();
    QString dolphinFileName();
    Scene *scene();
    SimulationController *simulationController();
    void load(QDataStream &stream);
    void load(const QString &fileName);
    void save(QDataStream &stream);
    void save(const QString &fileName);
    void setDolphinFileName(const QString &fileName);
    void setIsAutosave();

signals:
    void fileChanged(const QFileInfo &fileInfo);

private:
    void autosave();
    void setAutosaveFileName();

    GraphicsView m_view;
    QFileInfo m_fileInfo;
    QString m_dolphinFileName;
    QTemporaryFile m_autosaveFile;
    Scene m_scene;
};
