#ifndef LISTITEMWIDGET_H
#define LISTITEMWIDGET_H

#include "label.h"

#include <QWidget>

class ListItemWidget : public QFrame {
  Q_OBJECT
private:
  Label * label;
public:
  explicit ListItemWidget(const QPixmap &pixmap, QString name, QString pixName, QString auxData = QString(), QWidget * parent = 0);


  Label * getLabel() const;


protected:
  void mousePressEvent( QMouseEvent *event );


signals:

public slots:
};

#endif /* LISTITEMWIDGET_H */
