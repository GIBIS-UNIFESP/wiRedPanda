// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspace.h"

#include "common.h"
#include "globalproperties.h"
#include "serializationfunctions.h"

#include <QHBoxLayout>
#include <QMessageBox>

WorkSpace::WorkSpace(QWidget *parent)
    : QWidget(parent)
{
    m_view.setCacheMode(QGraphicsView::CacheBackground);
    m_view.setScene(&m_scene);
    m_scene.setSceneRect(m_scene.sceneRect().united(m_view.rect()));
    setLayout(new QHBoxLayout);
    layout()->addWidget(&m_view);
}

Scene *WorkSpace::scene()
{
    return &m_scene;
}

GraphicsView *WorkSpace::view()
{
    return &m_view;
}

SimulationController *WorkSpace::simulationController()
{
    return m_scene.simulationController();
}

void WorkSpace::setCurrentFile(const QFileInfo &fileInfo)
{
    m_currentFile = fileInfo;
}

QFileInfo WorkSpace::currentFile()
{
    return m_currentFile;
}

void WorkSpace::save(QDataStream &stream, const QString &dolphinFileName)
{
    SerializationFunctions::saveHeader(stream, dolphinFileName, m_scene.sceneRect());
    SerializationFunctions::serialize(m_scene.items(), stream);
}

void WorkSpace::load(QDataStream &stream)
{
    qCDebug(zero) << "Loading file.";
    m_scene.simulationController()->stop();
    qCDebug(zero) << "Stopped simulation.";
    const double version = SerializationFunctions::loadVersion(stream);
    if (version > GlobalProperties::version && GlobalProperties::verbose) {
        QMessageBox::warning(this, tr("Newer version file."), tr("Warning! Your WiRedPanda is possibly outdated.\n The file you are opening has been saved in a newer version.\n Please check for updates."));
    } else if (version < 4.0 && GlobalProperties::verbose) {
        QMessageBox::warning(this, tr("Old version file."), tr("Warning! This is an old version WiRedPanda project file (version < 4.0). To open it correctly, save all ICs and skins the main project directory."));
    }
    qCDebug(zero) << "Version:" << version;
    m_dolphinFileName = SerializationFunctions::loadDolphinFileName(stream, version);
    qCDebug(zero) << "Dolphin name:" << m_dolphinFileName;
    QRectF rect(SerializationFunctions::loadRect(stream, version));
    qCDebug(zero) << "Header Ok. Version:" << version;
    QList<QGraphicsItem *> items = SerializationFunctions::deserialize(stream, version);
    qCDebug(zero) << "Finished loading items.";
    for (auto *item : qAsConst(items)) {
        m_scene.addItem(item);
    }
    qCDebug(three) << "This code tries to centralize the elements in scene using the rectangle. But it is not working well.";
    m_scene.setSceneRect(m_scene.itemsBoundingRect());
    if (!m_scene.views().empty()) {
        const auto scene_views = m_scene.views();
        auto *view = static_cast<GraphicsView *>(scene_views.first());
        rect = rect.united(view->rect());
        rect.moveCenter(QPointF(0, 0));
        m_scene.setSceneRect(m_scene.sceneRect().united(rect));
        view->centerOn(m_scene.itemsBoundingRect().center());
    }
    m_scene.clearSelection();
    m_scene.simulationController()->start();
    qCDebug(zero) << "Finished loading file.";
}

void WorkSpace::setDolphinFileName(const QString &fileName)
{
    m_dolphinFileName = fileName;
}

QString WorkSpace::dolphinFileName()
{
    return m_dolphinFileName;
}

void WorkSpace::selectWorkspace()
{
    ICManager::wakeUp();
}
