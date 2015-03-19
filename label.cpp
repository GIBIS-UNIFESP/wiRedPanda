#include <QtWidgets>
#include "label.h"
#include "graphicelement.h"
Label::Label(QWidget *parent) :
  QLabel(parent) {
}

QString Label::elementType() {
  return m_elementType;
}

void Label::setElementType(QString elementType) {
  m_elementType = elementType;
}

void Label::mousePressEvent(QMouseEvent *event) {
  QPixmap pixmap = *this->pixmap();

  QByteArray itemData;
  QDataStream dataStream(&itemData, QIODevice::WriteOnly);
  ElementType type = ElementType::UNKNOWN;
  qDebug() << objectName();
  if(objectName().endsWith("_button")){
    type = ElementType::BUTTON;
  }else if(objectName().endsWith("_led")){
    type = ElementType::LED;
  }
  dataStream << pixmap << QPointF(event->pos()) << (qint32) type;

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

