#ifndef SCENE_H
#define SCENE_H

#include <QGraphicsScene>
#include <QObject>

class Scene : public QGraphicsScene {
public:
  Scene(QObject *parent = 0);
  Scene(const QRectF & sceneRect, QObject * parent = 0);
  Scene(qreal x, qreal y, qreal width, qreal height, QObject * parent = 0);

  // QGraphicsScene interface
  int gridSize() const;
  void setGridSize(int gridSize);

protected:
  void drawBackground(QPainter * painter, const QRectF & rect);
  int m_gridSize;
};

#endif // SCENE_H
