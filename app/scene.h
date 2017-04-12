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
  QVector< GraphicElement* > getElements( QRectF rect = QRectF( ) );
  QVector< QNEConnection* > getConnections( );
  QVector< GraphicElement* > selectedElements( );

  void setDots( const QPen &dots );

protected:
  void drawBackground( QPainter *painter, const QRectF &rect );
  int m_gridSize;
  QPen m_dots;
};

#endif /* SCENE_H */
