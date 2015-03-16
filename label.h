#ifndef LABEL_H
#define LABEL_H

#include <QLabel>

class Label : public QLabel
{
    Q_OBJECT
public:
    explicit Label(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *event);

signals:

public slots:

};

#endif // LABEL_H
