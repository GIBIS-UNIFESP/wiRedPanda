#ifndef LABEL_H
#define LABEL_H

#include <QLabel>

class Label : public QLabel {
  Q_OBJECT
public:
  Q_PROPERTY( QString elementType READ elementType WRITE setElementType DESIGNABLE true )
  explicit Label( QWidget *parent = 0 );

  QString elementType( );
  void setElementType( QString elementType );
  QString auxData( ) const;
  void setAuxData( const QString &auxData );
  void startDrag( QPoint pos = QPoint( 32, 32 ) );

protected:
  void mousePressEvent( QMouseEvent *event );

private:
  QString m_elementType;
  QString m_auxData;
};

#endif /* LABEL_H */
