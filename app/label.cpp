#include "elementfactory.h"
#include "label.h"

#include <QtWidgets>
#include <thread>

Label::Label( QWidget *parent ) : QLabel( parent ) {
}

Label::~Label( ) {

}

ElementType Label::elementType( ) {
  return( m_elementType );
}

void Label::setElementType( ElementType elementType ) {
  m_elementType = elementType;
}

void Label::mousePressEvent( QMouseEvent *event ) {
  startDrag( event->pos( ) );
}

QString Label::name( ) const {
  return( m_name );
}

void Label::setName( const QString &name ) {
  m_name = name;
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
  if( pos.isNull( ) ) {
    pos = this->pixmap( )->rect( ).center( );
  }
  QByteArray itemData;
  QDataStream dataStream( &itemData, QIODevice::WriteOnly );
  QString text = ElementFactory::typeToText( m_elementType );
  if( text.contains( "_" ) ) {
    text = text.split( "_" ).last( );
  }
  ElementType type = ElementFactory::textToType( text );
/*  qDebug() << objectName(); */

  dataStream << QPointF( pos ) << static_cast< qint32 >( type ) << m_auxData;

  QMimeData *mimeData = new QMimeData;
  mimeData->setData( "application/x-dnditemdata", itemData );
  QDrag *drag = new QDrag( parent( ) );
  drag->setMimeData( mimeData );
  drag->setPixmap( pixmap );
  drag->setHotSpot( pos );
  drag->exec( Qt::CopyAction, Qt::CopyAction );
}
