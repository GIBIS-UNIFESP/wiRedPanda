#ifndef LISTITEMWIDGET_H
#define LISTITEMWIDGET_H

#include "label.h"

#include <QWidget>

class ListItemWidget : public QWidget {
  Q_OBJECT
private:
  Label * label;
public:
  explicit ListItemWidget(const QPixmap &pixmap, QString name, QString pixName, QString auxData = QString(), QWidget * parent = 0);


  Label * getLabel() const;

signals:

public slots:
};

#endif /* LISTITEMWIDGET_H */
