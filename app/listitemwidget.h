#ifndef LISTITEMWIDGET_H
#define LISTITEMWIDGET_H

#include "label.h"

#include <QWidget>

class ListItemWidget : public QFrame {
  Q_OBJECT
private:
  Label * label;
public:
  explicit ListItemWidget(const QPixmap & pixmap, QString name, ElementType elementType, QString auxData, QWidget * parent  = NULL);


  Label * getLabel() const;


protected:
  void mousePressEvent( QMouseEvent *event );


signals:

public slots:
};

#endif /* LISTITEMWIDGET_H */
