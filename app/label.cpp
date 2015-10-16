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
  ElementType type = ElementType::UNKNOWN;
/*  qDebug() << objectName(); */
  if( objectName( ).endsWith( "_button" ) ) {
    type = ElementType::BUTTON;
  }
  else if( objectName( ).endsWith( "_led" ) ) {
    type = ElementType::LED;
  }
  else if( objectName( ).endsWith( "_and" ) ) {
    type = ElementType::AND;
  }
  else if( objectName( ).endsWith( "_or" ) ) {
    type = ElementType::OR;
  }
  else if( objectName( ).endsWith( "_clock" ) ) {
    type = ElementType::CLOCK;
  }
  else if( objectName( ).endsWith( "_switch" ) ) {
    type = ElementType::SWITCH;
  }
  else if( objectName( ).endsWith( "_not" ) ) {
    type = ElementType::NOT;
  }
  else if( objectName( ).endsWith( "_nand" ) ) {
    type = ElementType::NAND;
  }
  else if( objectName( ).endsWith( "_nor" ) ) {
    type = ElementType::NOR;
  }
  else if( objectName( ).endsWith( "_xor" ) ) {
    type = ElementType::XOR;
  }
  else if( objectName( ).endsWith( "_xnor" ) ) {
    type = ElementType::XNOR;
  }
  else if( objectName( ).endsWith( "_vcc" ) ) {
    type = ElementType::VCC;
  }
  else if( objectName( ).endsWith( "_gnd" ) ) {
    type = ElementType::GND;
  }
  else if( objectName( ).endsWith( "_dflipflop" ) ) {
    type = ElementType::DFLIPFLOP;
  }
  else if( objectName( ).endsWith( "_dlatch" ) ) {
    type = ElementType::DLATCH;
  }
  else if( objectName( ).endsWith( "_jkflipflop" ) ) {
    type = ElementType::JKFLIPFLOP;
  }
  else if( objectName( ).endsWith( "_jklatch" ) ) {
    type = ElementType::JKLATCH;
  }
  else if( objectName( ).endsWith( "_srflipflop" ) ) {
    type = ElementType::SRFLIPFLOP;
  }
  else if( objectName( ).endsWith( "_tlatch" ) ) {
    type = ElementType::TLATCH;
  }
  else if( objectName( ).endsWith( "_tflipflop" ) ) {
    type = ElementType::TFLIPFLOP;
  }
  else if( objectName( ).endsWith( "_display" ) ) {
    type = ElementType::DISPLAY;
  }
  else if( objectName( ).endsWith( "_box" ) ) {
    type = ElementType::BOX;
  }
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
