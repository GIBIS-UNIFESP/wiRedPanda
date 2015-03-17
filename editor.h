#ifndef EDITOR_H
#define EDITOR_H

#include <QGraphicsScene>
#include <QObject>

#include <nodes/qneconnection.h>

class Editor : public QObject {
  Q_OBJECT
public:
  explicit Editor(QObject *parent = 0);
  ~Editor();
  void install(QGraphicsScene * s);
signals:

public slots:

private:
  QGraphicsScene * scene;
  QGraphicsItem * itemAt( const QPointF & pos );
  QNEConnection * conn;
  // QObject interface
public:
  bool eventFilter(QObject *o, QEvent *e);
};

#endif // EDITOR_H
