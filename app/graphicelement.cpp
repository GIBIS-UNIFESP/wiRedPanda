#include "graphicelement.h"
#include "scene.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <iostream>
#include <nodes/qneconnection.h>
#include <stdexcept>
GraphicElement::GraphicElement( int minInputSz, int maxInputSz, int minOutputSz, int maxOutputSz,
                                QGraphicsItem *parent ) : QGraphicsObject( parent ), label( new QGraphicsTextItem(
                                                                                              this ) )
{
  setFlag( QGraphicsItem::ItemIsMovable );
  setFlag( QGraphicsItem::ItemIsSelectable );
/*  setFlag(QGraphicsItem::ItemSendsScenePositionChanges); */
  setFlag( QGraphicsItem::ItemSendsGeometryChanges );


  label->hide( );
  QFont font( "SansSerif" );
  font.setBold( true );
  label->setFont( font );
  label->setPos( 64, 30 );
  label->setParentItem( this );
  m_bottomPosition = 64;
  m_topPosition = 0;
  m_minInputSz = minInputSz;
  m_minOutputSz = minOutputSz;
  m_maxInputSz = maxInputSz;
  m_maxOutputSz = maxOutputSz;
  m_changed = true;
  m_visited = false;
  m_beingVisited = false;
  m_rotatable = true;
  m_hasColors = false;
  m_hasTrigger = false;
  m_hasFrequency = false;
  m_hasLabel = false;
  m_disabled = false;
  m_outputsOnTop = true;
  for( int i = 0; i < minInputSz; i++ ) {
    addInputPort( );
  }
  for( int i = 0; i < minOutputSz; i++ ) {
    addOutputPort( );
  }
}

GraphicElement::~GraphicElement( ) {

}
QPixmap GraphicElement::getPixmap( ) const {
  return( pixmap );
}


void GraphicElement::disable( ) {
  m_disabled = true;
}

void GraphicElement::enable( ) {
  m_disabled = false;
}

int GraphicElement::id( ) const {
  return( m_id );
}

void GraphicElement::setId( int value ) {
  m_id = value;
}

void GraphicElement::setPixmap( const QPixmap &pixmap ) {
  setTransformOriginPoint( pixmap.width( ) / 2, pixmap.height( ) / 2 );
  this->pixmap = pixmap;
  update( boundingRect( ) );
}

QVector< QNEPort* > GraphicElement::outputs( ) const {
  return( m_outputs );
}

void GraphicElement::setOutputs( const QVector< QNEPort* > &outputs ) {
  m_outputs = outputs;
}

void GraphicElement::save( QDataStream &ds ) {
  ds << pos( );
  ds << rotation( );

  /* <Version1.2> */
  ds << getLabel( );
  /* <\Version1.2> */

  /* <Version1.3> */
  ds << m_minInputSz;
  ds << m_maxInputSz;
  ds << m_minOutputSz;
  ds << m_maxOutputSz;
  /* <\Version1.3> */
  /* <Version1.9> */
  ds << m_trigger;
  /* <\Version1.9> */
  ds << ( quint64 ) m_inputs.size( );
  foreach( QNEPort * port, m_inputs ) {
    ds << ( quint64 ) port;
    ds << port->portName( );
    ds << port->portFlags( );
  }
  ds << ( quint64 ) m_outputs.size( );
  foreach( QNEPort * port, m_outputs ) {
    ds << ( quint64 ) port;
    ds << port->portName( );
    ds << port->portFlags( );
  }
}

void GraphicElement::load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) {
  QPointF p;
  QString label_text;
  ds >> p;
  qreal angle;
  ds >> angle;
  setPos( p );
  setRotation( angle );
  /* <Version1.2> */
  if( version >= 1.2 ) {
    ds >> label_text;
    setLabel( label_text );
  }
  /* <\Version1.2> */
  /* <Version1.3> */
  if( version >= 1.3 ) {
    ds >> m_minInputSz;
    ds >> m_maxInputSz;
    ds >> m_minOutputSz;
    ds >> m_maxOutputSz;
  }
  /* <\Version1.3> */
  /* <Version1.9> */
  if( version >= 1.9 ) {
    ds >> m_trigger;
    setLabel( label_text );
  }
  /* <\Version1.9> */

  quint64 inputSz, outputSz;
  ds >> inputSz;
  if( inputSz > MAXIMUMVALIDINPUTSIZE ) {
    throw std::runtime_error( "Corrupted file!" );
  }
  for( size_t port = 0; port < inputSz; ++port ) {
    QString name;
    int flags;
    quint64 ptr;
    ds >> ptr;
    ds >> name;
    ds >> flags;
    if( port < ( size_t ) m_inputs.size( ) ) {
      portMap[ ptr ] = m_inputs[ port ];
      if( elementType( ) == ElementType::BOX ) {
        m_inputs[ port ]->setName( name );
      }
      m_inputs[ port ]->setPortFlags( flags );
      m_inputs[ port ]->setPtr( ptr );
    }
    else {
      portMap[ ptr ] = addPort( name, false, flags, ptr );
    }
  }
  ds >> outputSz;
  if( outputSz > MAXIMUMVALIDINPUTSIZE ) {
    throw std::runtime_error( "Corrupted file!" );
  }
  for( size_t port = 0; port < outputSz; ++port ) {
    QString name;
    int flags;
    quint64 ptr;
    ds >> ptr;
    ds >> name;
    ds >> flags;
    if( port < ( size_t ) m_outputs.size( ) ) {
      portMap[ ptr ] = m_outputs[ port ];
      if( elementType( ) == ElementType::BOX ) {
        m_outputs[ port ]->setName( name );
      }
      m_outputs[ port ]->setPortFlags( flags );
      m_outputs[ port ]->setPtr( ptr );
    }
    else {
      portMap[ ptr ] = addPort( name, true, flags, ptr );
    }
  }
}

QVector< QNEPort* > GraphicElement::inputs( ) const {
  return( m_inputs );
}

void GraphicElement::setInputs( const QVector< QNEPort* > &inputs ) {
  m_inputs = inputs;
}


QRectF GraphicElement::boundingRect( ) const {
  return( pixmap.rect( ) );
}

void GraphicElement::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget ) {
  Q_UNUSED( widget )
  painter->setClipRect( option->exposedRect );
  if( isSelected( ) ) {
    painter->setBrush( Qt::darkGray );
    painter->setPen( QPen( Qt::black ) );
    painter->drawRoundedRect( boundingRect( ), 16, 16 );
  }
  painter->drawPixmap( QPoint( 0, 0 ), pixmap );
}

QNEPort* GraphicElement::addPort( const QString &name, bool isOutput, int flags, int ptr ) {
  if( isOutput && ( ( quint64 ) m_outputs.size( ) >= m_maxOutputSz ) ) {
    return( NULL );
  }
  else if( !isOutput && ( ( quint64 ) m_inputs.size( ) >= m_maxInputSz ) ) {
    return( NULL );
  }
  QNEPort *port = new QNEPort( this );
  port->setName( name );
  port->setIsOutput( isOutput );
  port->setGraphicElement( this );
  port->setPortFlags( flags );
  port->setPtr( ptr );
  if( isOutput ) {
    m_outputs.push_back( port );
  }
  else {
    m_inputs.push_back( port );
  }
  this->updatePorts( );
  port->show( );
  return( port );
}

void GraphicElement::addInputPort( const QString &name ) {
  addPort( name, false );
}

void GraphicElement::addOutputPort( const QString &name ) {
  addPort( name, true );
}

void GraphicElement::setPortName( QString name ) {
  setObjectName( name );
  setToolTip( name );
}

void GraphicElement::updatePorts( ) {
  int inputPos = m_topPosition;
  int outputPos = m_bottomPosition;
  if( m_outputsOnTop ) {
    inputPos = m_bottomPosition;
    outputPos = m_topPosition;
  }
  if( !m_outputs.isEmpty( ) ) {
    int step = qMax( 32 / m_outputs.size( ), 6 );
    int x = 32 - m_outputs.size( ) * step + step;
    foreach( QNEPort * port, m_outputs ) {
      port->setPos( x, outputPos );
      port->update( );
      x += step * 2;
    }
  }
  if( !m_inputs.isEmpty( ) ) {
    int step = qMax( 32 / m_inputs.size( ), 6 );
    int x = 32 - m_inputs.size( ) * step + step;
    foreach( QNEPort * port, m_inputs ) {
      port->setPos( x, inputPos );
      port->update( );
      x += step * 2;
    }
  }
}

/*
 * void GraphicElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * e) {
 *  if(e->button() == Qt::LeftButton) {
 *    addOutputPort();
 *  } else {
 *    addInputPort();
 *  }
 * }
 */

QVariant GraphicElement::itemChange( QGraphicsItem::GraphicsItemChange change, const QVariant &value ) {
  /* Align to grid */
  if( ( change == ItemPositionChange ) && scene( ) ) {
    QPointF newPos = value.toPointF( );
    Scene *customScene = dynamic_cast< Scene* >( scene( ) );
    if( customScene ) {
      int gridSize = customScene->gridSize( );
      qreal xV = qRound( newPos.x( ) / gridSize ) * gridSize;
      qreal yV = qRound( newPos.y( ) / gridSize ) * gridSize;
      return( QPointF( xV, yV ) );
    }
    else {
      return( newPos );
    }
  }
  /* Moves wires */
  if( ( change == ItemScenePositionHasChanged ) || ( change == ItemRotationHasChanged ) ||
      ( change == ItemTransformHasChanged ) ) {
    foreach( QNEPort * port, m_outputs ) {
      port->updateConnections( );
    }
    foreach( QNEPort * port, m_inputs ) {
      port->updateConnections( );
    }
  }
  update( );
  return( QGraphicsItem::itemChange( change, value ) );
}

QKeySequence GraphicElement::getTrigger( ) const {
  return( m_trigger );
}

void GraphicElement::setTrigger( const QKeySequence &trigger ) {
  m_trigger = trigger;
}

QString GraphicElement::genericProperties( ) {
  return( QString( ) );
}

/*
 * void GraphicElement::mouseReleaseEvent( QGraphicsSceneMouseEvent *event ) {
 *  / * Align to grid * /
 *  QPointF newPos = this->pos();
 *  Scene *customScene = dynamic_cast< Scene* >( scene( ) );
 *  if( customScene ) {
 *    int gridSize = customScene->gridSize( );
 *    qreal xV = qRound( newPos.x( ) / gridSize ) * gridSize;
 *    qreal yV = qRound( newPos.y( ) / gridSize ) * gridSize;
 *    setPos(QPointF( xV, yV ) );
 *  }
 *  QGraphicsObject::mouseReleaseEvent( event );
 * }
 */

void GraphicElement::setLabel( QString label ) {
  m_labelText = label;
  if( !hasTrigger( ) || getTrigger( ).toString( ).isEmpty( ) ) {
    this->label->setPlainText( label );
  }
  else {
    if( !label.isEmpty( ) ) {
      label += " ";
    }
    this->label->setPlainText( label + QString( "(%1)" ).arg( getTrigger( ).toString( ) ) );
  }
}

QString GraphicElement::getLabel( ) {
  return( m_labelText );
}

bool GraphicElement::visited( ) const {
  return( m_visited );
}

void GraphicElement::setVisited( bool visited ) {
  m_visited = visited;
}

bool GraphicElement::isValid( ) {
  bool valid = true;
  for( QNEPort *input  : m_inputs ) {
    /* Required inputs must have exactly one connection. */
    if( !input->isValid( ) ) {
      valid = false;
      break;
    }
  }
  if( valid == false ) {
    foreach( QNEPort * output, outputs( ) ) {
      foreach( QNEConnection * conn, output->connections( ) ) {
        conn->setStatus( QNEConnection::Invalid );
        QNEPort *port = conn->otherPort( output );
        if( port ) {
          port->setValue( -1 );
        }
      }
    }
  }
  return( valid );
}

bool GraphicElement::beingVisited( ) const {
  return( m_beingVisited );
}

void GraphicElement::setBeingVisited( bool beingVisited ) {
  m_beingVisited = beingVisited;
}

bool GraphicElement::changed( ) const {
  return( m_changed );
}

void GraphicElement::setChanged( bool changed ) {
  m_changed = changed;
}

bool GraphicElement::hasColors( ) const {
  return( m_hasColors );
}

bool GraphicElement::hasTrigger( ) const {
  return( m_hasTrigger );
}

void GraphicElement::setColor( QString ) {

}

QString GraphicElement::color( ) {
  return( QString( ) );
}

void GraphicElement::setHasColors( bool hasColors ) {
  m_hasColors = hasColors;
}

void GraphicElement::setHasTrigger( bool hasTrigger ) {
  m_hasTrigger = hasTrigger;
}

bool GraphicElement::hasFrequency( ) const {
  return( m_hasFrequency );
}

void GraphicElement::setHasFrequency( bool hasFrequency ) {
  m_hasFrequency = hasFrequency;
}

bool GraphicElement::hasLabel( ) const {
  return( m_hasLabel );
}

void GraphicElement::setHasLabel( bool hasLabel ) {
  m_hasLabel = hasLabel;
  label->setVisible( hasLabel );
}

bool GraphicElement::rotatable( ) const {
  return( m_rotatable );
}

void GraphicElement::setRotatable( bool rotatable ) {
  m_rotatable = rotatable;
}

int GraphicElement::minOutputSz( ) const {
  return( m_minOutputSz );
}

int GraphicElement::inputSize( ) {
  return( m_inputs.size( ) );
}

void GraphicElement::setInputSize( int size ) {
  if( ( size >= minInputSz( ) ) && ( size <= maxInputSz( ) ) ) {
    if( inputSize( ) < size ) {
      while( inputSize( ) < size ) {
        addInputPort( );
      }
    }
    else {
      while( inputSize( ) > size ) {
        delete m_inputs.back( );
        m_inputs.pop_back( );
      }
      updatePorts( );
    }
  }
}

int GraphicElement::outputSize( ) {
  return( m_outputs.size( ) );
}

void GraphicElement::setOutputSize( int size ) {
  if( ( size >= minOutputSz( ) ) && ( size <= maxOutputSz( ) ) ) {
    if( outputSize( ) < size ) {
      for( int port = outputSize( ); port < size; ++port ) {
        addOutputPort( );
      }
    }
    else {
      while( outputSize( ) > size ) {
        m_outputs.pop_back( );
      }
    }
  }
}

float GraphicElement::frequency( ) {
  return( 0.0 );
}

void GraphicElement::setFrequency( float ) {

}

void GraphicElement::setMinOutputSz( int minOutputSz ) {
  m_minOutputSz = minOutputSz;
}

int GraphicElement::minInputSz( ) const {
  return( m_minInputSz );
}

void GraphicElement::setMinInputSz( int minInputSz ) {
  m_minInputSz = minInputSz;
}


int GraphicElement::maxOutputSz( ) const {
  return( m_maxOutputSz );
}

void GraphicElement::setMaxOutputSz( int maxOutputSz ) {
  m_maxOutputSz = maxOutputSz;
}

int GraphicElement::maxInputSz( ) const {
  return( m_maxInputSz );
}

void GraphicElement::setMaxInputSz( int maxInputSz ) {
  m_maxInputSz = maxInputSz;
}

int GraphicElement::bottomPosition( ) const {
  return( m_bottomPosition );
}

void GraphicElement::setBottomPosition( int bottomPosition ) {
  m_bottomPosition = bottomPosition;
  updatePorts( );
}

int GraphicElement::topPosition( ) const {
  return( m_topPosition );
}

void GraphicElement::setTopPosition( int topPosition ) {
  m_topPosition = topPosition;
  updatePorts( );
}

bool GraphicElement::outputsOnTop( ) const {
  return( m_outputsOnTop );
}

void GraphicElement::setOutputsOnTop( bool outputsOnTop ) {
  m_outputsOnTop = outputsOnTop;
  updatePorts( );
}

bool GraphicElement::disabled( ) {
  return( m_disabled );
}
