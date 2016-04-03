#include "elementfactory.h"
#include "graphicelement.h"
#include "label.h"
#include <QtWidgets>

Label::Label( QWidget *parent ) : QLabel( parent ) {
  setBusy( false );
}

Label::~Label( ) {

}

QString Label::elementType( ) {
  return( m_elementType );
}

void Label::setElementType( QString elementType ) {
  m_elementType = elementType;
}

void Label::mousePressEvent( QMouseEvent *event ) {
  startDrag( event->pos( ) );
}

bool Label::busy( ) const {
  return( m_busy );
}

void Label::setBusy( bool busy ) {
  m_busy = busy;
}

QString Label::auxData( ) const {
  return( m_auxData );
}

void Label::setAuxData( const QString &auxData ) {
  m_auxData = auxData;
  setProperty( "Name", auxData );
}

void Label::startDrag( QPoint pos ) {
  QPixmap pixmap = *this->pixmap( );

  QByteArray itemData;
  QDataStream dataStream( &itemData, QIODevice::WriteOnly );
  QString text = objectName( );
  if( text.contains( "_" ) ) {
    text = text.split( "_" ).last( );
  }
  ElementType type = ElementFactory::textToType( text );
/*  qDebug() << objectName(); */

  dataStream << QPointF( pos ) << ( qint32 ) type << m_auxData;

  QMimeData *mimeData = new QMimeData;
  mimeData->setData( "application/x-dnditemdata", itemData );

  QDrag *drag = new QDrag( this );
  drag->setMimeData( mimeData );
  drag->setPixmap( pixmap );
  drag->setHotSpot( pos );
  drag->exec( Qt::CopyAction, Qt::CopyAction );
  setBusy( true );
}
