#ifndef SCENE_H
#define SCENE_H

#include "graphicelement.h"

#include <QGraphicsScene>
#include <QObject>

class Scene : public QGraphicsScene {
public:
  explicit Scene( QObject *parent = 0 );
  Scene( const QRectF &sceneRect, QObject *parent = 0 );
  Scene( qreal x, qreal y, qreal width, qreal height, QObject *parent = 0 );

  /* QGraphicsScene interface */
  int gridSize( ) const;
  QVector< GraphicElement* > getElements( );

protected:
  void drawBackground( QPainter *painter, const QRectF &rect );
  int m_gridSize;
};

#endif /* SCENE_H */
