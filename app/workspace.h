/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"
#include "graphicsview.h"
#include "mainwindow.h"
#include "scene.h"

#include <QUndoStack>

class GraphicsView;
class SimulationController;

class WorkSpace : public QWidget
{
    Q_OBJECT

public:
    explicit WorkSpace(QWidget *parent = nullptr);

    GraphicsView *view();
    QFileInfo currentFile();
    QString dolphinFileName();
    Scene *scene();
    SimulationController *simulationController();
    void load(QDataStream &stream);
    void save(QDataStream &stream, const QString &dolphinFileName);
    void selectWorkspace();
    void setCurrentFile(const QFileInfo &fileInfo);
    void setDolphinFileName(const QString &fileName);

private:
    GraphicsView m_view;
    QFileInfo m_currentFile;
    QString m_dolphinFileName;
    Scene m_scene;
};
