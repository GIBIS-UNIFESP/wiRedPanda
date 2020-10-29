#ifndef LABEL_H
#define LABEL_H

#include "graphicelement.h"

#include <QDrag>
#include <QLabel>

class Label : public QLabel {
  Q_OBJECT
public:
  explicit Label( QWidget *parent = nullptr );

  virtual ~Label( );
  ElementType elementType( );
  void setElementType( ElementType elementType );
  QString auxData( ) const;
  void setAuxData( const QString &auxData );
  void startDrag( QPoint pos = QPoint( ) );
  QString name( ) const;
  void setName( const QString &name );

  const QPixmap &pixmapData( ) const;
  void setPixmapData( const QPixmap &pixmapData );

protected:
  void mousePressEvent( QMouseEvent *event );

private:
  ElementType m_elementType;
  QPixmap m_pixmapData;
  QString m_name;
  QString m_auxData;
};

#endif /* LABEL_H */