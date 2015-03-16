#include <QtWidgets>
#include <QDebug>
#include "graphicsview.h"

GraphicsView::GraphicsView(QWidget *parent) :
  QGraphicsView(parent) {
  setAcceptDrops(true);
}

void GraphicsView::dragEnterEvent(QDragEnterEvent *event) {
  if(event->mimeData()->hasFormat("application/x-dnditemdata")) {
    if(event->source() == this) {
      event->setDropAction(Qt::MoveAction);
      event->accept();
    } else {
      event->acceptProposedAction();
    }
  } else {
    event->ignore();
  }
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *event) {
  if(event->mimeData()->hasFormat("application/x-dnditemdata")) {
    if(event->source() == this) {
      event->setDropAction(Qt::MoveAction);
      event->accept();
    } else {
      event->acceptProposedAction();
    }
  } else {
    event->ignore();
  }
}

void GraphicsView::dropEvent(QDropEvent *event) {
  qDebug() << "Drop";
  if(event->mimeData()->hasFormat("application/x-dnditemdata")) {
    qDebug() << "Drop Accepted";
    QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);

    QPixmap pixmap;
    QPoint offset;
    dataStream >> pixmap >> offset;

    QPoint pos = event->pos();
    pos = QPoint( 64 * (pos.x()/64), 64 * (pos.y()/64));

    QLabel *newIcon = new QLabel(this);
    newIcon->setPixmap(pixmap);
    newIcon->move(pos);
    newIcon->show();
    newIcon->setAttribute(Qt::WA_DeleteOnClose);

//        QGraphicsProxyWidget *proxy = scene()->addWidget(newIcon);
//        proxy->setFlag(QGraphicsItem::ItemIsMovable);
//        proxy->setFlag(QGraphicsItem::ItemIsSelectable);

//        QGraphicsPixmapItem *item = scene()->addPixmap(pixmap);
//        item->setPos(event->pos() - offset);
//        item->setPos(event->pos());
//        qDebug() << "event pos:" << event->pos();
//        qDebug() << "offset:" << offset;
//        item->show();
//        item->setFlag(QGraphicsPixmapItem::ItemIsMovable);
//        item->setFlag(QGraphicsPixmapItem::ItemIsSelectable);

    if(event->source() == this) {
      event->setDropAction(Qt::MoveAction);
      event->accept();
    } else {
      event->acceptProposedAction();
    }
  } else {
    event->ignore();
  }
}

void GraphicsView::mousePressEvent(QMouseEvent *event) {

  QGraphicsView::mousePressEvent(event);
  //    if(itemAt(event->pos())){
  //        QPixmap pixmap = itemAt(event->pos())
  //        QPixmap pixmap = *this->pixmap();

  //        QByteArray itemData;
  //        QDataStream dataStream(&itemData, QIODevice::WriteOnly);
  //        dataStream << pixmap << QPoint(event->pos());

  //        QMimeData *mimeData = new QMimeData;
  //        mimeData->setData("application/x-dnditemdata", itemData);

  //        QDrag *drag = new QDrag(this);
  //        drag->setMimeData(mimeData);
  //        drag->setPixmap(pixmap);
  //        drag->setHotSpot(event->pos());

  //        QPixmap tempPixmap = pixmap;
  //        QPainter painter;
  //        painter.begin(&tempPixmap);
  //        painter.fillRect(pixmap.rect(), QColor(127, 127, 127, 127));
  //        painter.end();

  //        setPixmap(tempPixmap);

  //        if(drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction){
  //            close();
  //        } else{
  //            show();
  //            setPixmap(pixmap);
  //        }
  //    }
}
