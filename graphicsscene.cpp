#include <QDebug>
#include "graphicsscene.h"

GraphicsScene::GraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{
}


void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "scene press";
    qDebug() << "scene event pos:" << event->pos();
}

void GraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "scene move";
}

void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "scene release";
}
