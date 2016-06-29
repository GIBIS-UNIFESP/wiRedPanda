#ifndef LABEL_H
#define LABEL_H

#include <QDrag>
#include <QLabel>
#include "graphicelement.h"

class Label : public QLabel {
  Q_OBJECT
public:
  explicit Label( QWidget *parent = 0 );

  virtual ~Label( );
  ElementType elementType( );
  void setElementType(ElementType elementType );
  QString auxData( ) const;
  void setAuxData( const QString &auxData );
  void startDrag( QPoint pos = QPoint( ) );

  QString name() const;
  void setName(const QString & name);

protected:
  void mousePressEvent( QMouseEvent *event );

private:
  ElementType m_elementType;
  QString m_name;
  QString m_auxData;
};

#endif /* LABEL_H */
