#include "graphicsview.h"

#include <QKeyEvent>
#include <QScrollBar>

GraphicsView::GraphicsView(QWidget * parent) : QGraphicsView(parent) {
  _pan = false;
}

void GraphicsView::mousePressEvent(QMouseEvent * e) {
  if(e->button() == Qt::MiddleButton) {
    _pan = true;
    _panStartX = e->x();
    _panStartY = e->y();
    setCursor(Qt::ClosedHandCursor);
    e->accept();
    return;
  }
  QGraphicsView::mousePressEvent(e);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent * e) {
  if(e->button() == Qt::MiddleButton) {
    _pan = false;
    setCursor(Qt::ArrowCursor);
    e->accept();
    return;
  }
  QGraphicsView::mouseReleaseEvent(e);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *e) {
  if(_pan) {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (e->x() - _panStartX));
    verticalScrollBar()->setValue(verticalScrollBar()->value() - (e->y() - _panStartY));
    _panStartX = e->x();
    _panStartY = e->y();
    e->accept();
    return;
  }
  QGraphicsView::mouseMoveEvent(e);
}
