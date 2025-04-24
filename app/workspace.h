// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicsview.h"
#include "scene.h"

#include <QFileInfo>
#include <QTemporaryFile>
#include <QUndoStack>

class GraphicsView;
class Simulation;

class WorkSpace : public QWidget
{
    Q_OBJECT

public:
    explicit WorkSpace(QWidget *parent = nullptr);

    GraphicsView *view();
    QFileInfo fileInfo();
    QString dolphinFileName();
    Scene *scene();
    Simulation *simulation();
    void load(QDataStream &stream, QVersionNumber number);
    void load(const QString &fileName);
    void save(QDataStream &stream);
    void save(const QString &fileName);
    void setAutosaveFile();
    void setDolphinFileName(const QString &fileName);

    int lastId() const;
    void setLastId(int newLastId);

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
    int m_lastId = 0;
};
