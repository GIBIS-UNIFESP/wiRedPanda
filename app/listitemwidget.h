#ifndef LISTITEMWIDGET_H
#define LISTITEMWIDGET_H

#include "label.h"

#include <QWidget>

class ListItemWidget : public QFrame {
  Q_OBJECT
private:
  Label *label;
  QLabel *nameLabel;
public:
  explicit ListItemWidget( const QPixmap &pixmap, ElementType elementType, QString icFileName, QWidget *parent = nullptr );

  Label* getLabel( ) const;

  void updateName( );

protected:
  void mousePressEvent( QMouseEvent *event );


signals:

public slots:
};

#endif /* LISTITEMWIDGET_H */
