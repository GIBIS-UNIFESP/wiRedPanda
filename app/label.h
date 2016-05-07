#ifndef LABEL_H
#define LABEL_H

#include <QLabel>

class Label : public QLabel {
  Q_OBJECT
public:
  explicit Label( QWidget *parent = 0 );

  Q_PROPERTY( QString elementType READ elementType WRITE setElementType DESIGNABLE true )


  virtual ~Label( );
  QString elementType( );
  void setElementType( QString elementType );
  QString auxData( ) const;
  void setAuxData( const QString &auxData );
  void startDrag( QPoint pos = QPoint( 32, 32 ) );

  bool busy( ) const;
  void setBusy( bool busy );

protected:
  void mousePressEvent( QMouseEvent *event );

private:
  bool m_busy;
  QString m_elementType;
  QString m_auxData;
};

#endif /* LABEL_H */
