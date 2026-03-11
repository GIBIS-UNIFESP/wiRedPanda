// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFileInfo>
#include <QTemporaryFile>
#include <QUndoStack>

#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"

class GraphicsView;
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
    // --- Lifecycle ---

    explicit WorkSpace(QWidget *parent = nullptr);

    // --- Component Access ---

    GraphicsView *view();
    Scene *scene();
    Simulation *simulation();

    // --- File Operations ---

    QFileInfo fileInfo();
    void load(const QString &fileName);
    void load(QDataStream &stream, QVersionNumber number);
    void save(const QString &fileName);
    void save(QDataStream &stream);
    void setAutosaveFile();

    // --- Waveform Integration ---

    QString dolphinFileName();
    void setDolphinFileName(const QString &fileName);

    // --- ID Management ---

    int lastId() const;
    void setLastId(int newLastId);

signals:
    // --- File signals ---

    void fileChanged(const QFileInfo &fileInfo);

private:
    // --- Internal methods ---

    void autosave();
    void setAutosaveFileName();

    // --- Members ---

    GraphicsView m_view;
    Scene m_scene;
    QFileInfo m_fileInfo;
    QString m_dolphinFileName;
    QTemporaryFile m_autosaveFile;
    int m_lastId = 0;
};
