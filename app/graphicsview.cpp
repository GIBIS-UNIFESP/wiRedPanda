#include "graphicsview.h"

#include <QApplication>
#include <QKeyEvent>
#include <QScrollBar>

GraphicsView::GraphicsView( QWidget *parent ) : QGraphicsView( parent ) {
  _pan = false;
  _panStartX = 0;
  _panStartY = 0;
}

void GraphicsView::mousePressEvent( QMouseEvent *e ) {
  if( e->button( ) == Qt::MiddleButton ) {
    _pan = true;
    _panStartX = e->x( );
    _panStartY = e->y( );
    QApplication::setOverrideCursor( Qt::ClosedHandCursor );
    e->accept( );
    return;
  }
  QGraphicsView::mousePressEvent( e );
}

void GraphicsView::mouseReleaseEvent( QMouseEvent *e ) {
  if( e->button( ) == Qt::MiddleButton ) {
    _pan = false;
    QApplication::restoreOverrideCursor( );
    e->accept( );
    return;
  }
  QGraphicsView::mouseReleaseEvent( e );
}

void GraphicsView::mouseMoveEvent( QMouseEvent *e ) {
  if( _pan ) {
    horizontalScrollBar( )->setValue( horizontalScrollBar( )->value( ) - ( e->x( ) - _panStartX ) );
    verticalScrollBar( )->setValue( verticalScrollBar( )->value( ) - ( e->y( ) - _panStartY ) );
    _panStartX = e->x( );
    _panStartY = e->y( );
    e->accept( );
    return;
  }
  QGraphicsView::mouseMoveEvent( e );
}
