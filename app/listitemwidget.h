#ifndef LISTITEMWIDGET_H
#define LISTITEMWIDGET_H

#include <QWidget>

class ListItemWidget : public QWidget {
  Q_OBJECT
public:
  explicit ListItemWidget(const QPixmap &pixmap, QString name, QString pixName, QString auxData = QString(), QWidget * parent = 0);


signals:

public slots:
};

#endif /* LISTITEMWIDGET_H */
