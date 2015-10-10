#include "scene.h"

#include <QPainter>

Scene::Scene(QObject * parent) : QGraphicsScene(parent) {

}

Scene::Scene(const QRectF &sceneRect, QObject *parent) : QGraphicsScene( sceneRect, parent) {

}

Scene::Scene(qreal x, qreal y, qreal width, qreal height, QObject *parent ) : QGraphicsScene( x, y, width, height, parent) {

}

int Scene::gridSize() const {
  return m_gridSize;
}

void Scene::setGridSize(int gridSize) {
  m_gridSize = gridSize;
}

void Scene::drawBackground(QPainter * painter, const QRectF & rect) {
  QGraphicsScene::drawBackground(painter,rect);
  QPen pen(QColor(Qt::black));
  painter->setPen(pen);
  qreal left = int(rect.left()) - (int(rect.left()) % m_gridSize);
  qreal top = int(rect.top()) - (int(rect.top()) % m_gridSize);
  QVector<QPointF> points;
  for (qreal x = left; x < rect.right(); x += m_gridSize) {
    for (qreal y = top; y < rect.bottom(); y += m_gridSize) {
      points.append(QPointF(x,y));
    }
  }
  painter->drawPoints(points.data(), points.size());
}
