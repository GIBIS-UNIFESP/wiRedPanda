#include <QtWidgets>
#include "label.h"

Label::Label(QWidget *parent) :
  QLabel(parent) {
}

void Label::mousePressEvent(QMouseEvent *event) {
  QPixmap pixmap = *this->pixmap();

  QByteArray itemData;
  QDataStream dataStream(&itemData, QIODevice::WriteOnly);
  dataStream << pixmap << QPointF(event->pos());

  QMimeData *mimeData = new QMimeData;
  mimeData->setData("application/x-dnditemdata", itemData);

  QDrag *drag = new QDrag(this);
  drag->setMimeData(mimeData);
  drag->setPixmap(pixmap);
  drag->setHotSpot(event->pos());

  QPixmap tempPixmap = pixmap;
  QPainter painter;
  painter.begin(&tempPixmap);
  painter.fillRect(pixmap.rect(), QColor(127, 127, 127, 127));
  painter.end();

  setPixmap(tempPixmap);

  if(drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction) {
    close();
  } else {
    show();
    setPixmap(pixmap);
  }
}

int Label::type() {

}
