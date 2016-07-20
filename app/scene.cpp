#include "scene.h"

#include <QColor>
#include <QPainter>
Scene::Scene( QObject *parent ) : QGraphicsScene( parent ) {
  m_gridSize = 16;
}

Scene::Scene( const QRectF &sceneRect, QObject *parent ) : QGraphicsScene( sceneRect, parent ) {
  m_gridSize = 16;
}

Scene::Scene( qreal x, qreal y, qreal width, qreal height, QObject *parent ) : QGraphicsScene( x,
                                                                                               y,
                                                                                               width,
                                                                                               height,
                                                                                               parent ) {
  m_gridSize = 16;
}

int Scene::gridSize( ) const {
  return( m_gridSize );
}

void Scene::drawBackground( QPainter *painter, const QRectF &rect ) {
  painter->setRenderHint( QPainter::Antialiasing, true );
  QGraphicsScene::drawBackground( painter, rect );
  QColor blackColor( Qt::darkGray );
  QPen pen( blackColor );
  painter->setPen( pen );
  qreal left = int( rect.left( ) ) - ( int( rect.left( ) ) % m_gridSize );
  qreal top = int( rect.top( ) ) - ( int( rect.top( ) ) % m_gridSize );
  QVector< QPointF > points;
  for( qreal x = left; x < rect.right( ); x += m_gridSize ) {
    for( qreal y = top; y < rect.bottom( ); y += m_gridSize ) {
      points.append( QPointF( x, y ) );
    }
  }
  painter->drawPoints( points.data( ), points.size( ) );
}

QVector< GraphicElement* > Scene::getElements( ) {
  QVector< GraphicElement* > elements;
  QList< QGraphicsItem* > myItems = items( );
  for( QGraphicsItem *item : myItems ) {
    GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
    if( elm ) {
      elements.append( elm );
    }
  }
  return( elements );
}

QVector< GraphicElement* > Scene::selectedElements( ) {
  QVector< GraphicElement* > elements;
  QList< QGraphicsItem* > myItems = selectedItems( );
  for( QGraphicsItem *item : myItems ) {
    if( item->type( ) == GraphicElement::Type ) {
      GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
      if( elm ) {
        elements.append( elm );
      }
    }
  }
  return( elements );
}
