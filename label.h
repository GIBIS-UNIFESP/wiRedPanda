#ifndef LABEL_H
#define LABEL_H

#include <QLabel>

class Label : public QLabel {
  Q_OBJECT
  Q_PROPERTY(int type READ type)
public:
  explicit Label(QWidget *parent = 0);

protected:
  void mousePressEvent(QMouseEvent *event);
  int type();
signals:

public slots:

};

#endif // LABEL_H
