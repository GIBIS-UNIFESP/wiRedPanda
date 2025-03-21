// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicsview.h"
#include "scene.h"

#include <QFileInfo>
#include <QTemporaryFile>
#include <QUndoStack>

class GraphicsView;
class IC;
class Simulation;

/*!
 * @class WorkSpace
 * @brief A widget containing a complete circuit editing environment
 *
 * WorkSpace serves as a container for all components needed for circuit editing,
 * including the GraphicsView for visualization and interaction, Scene for element
 * management, and Simulation for circuit simulation. It also handles file operations
 * like loading, saving, and autosaving.
 */
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
    void save(const QString &fileName, const bool embeddedIcSave = false);
    void saveEmbeddedIc(const bool reload = true);
    void setAutosaveFile();
    void setDolphinFileName(const QString &fileName);

    IC *m_EmbeddedIc = nullptr;
    QFileInfo *fatherFileInfo = nullptr;
    WorkSpace *fatherTab = nullptr;

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
