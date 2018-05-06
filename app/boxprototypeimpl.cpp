#include "boxprototypeimpl.h"
#include "editor.h"
#include "elementfactory.h"
#include "serializationfunctions.h"

#include <QDebug>
#include <QFile>

BoxPrototypeImpl::~BoxPrototypeImpl( ) {
  for( GraphicElement *elm: elements ) {
    delete elm;
  }
}


//void BoxPrototypeImpl::sortMap( QVector< QNEPort* > &map ) {
//  // BubbleSort
//  for( int i = map.size( ) - 1; i >= 1; i-- ) {
//    for( int j = 0; j < i; j++ ) {
//      QPointF p1 = map[ j ]->graphicElement( )->pos( );
//      QPointF p2 = map[ j + 1 ]->graphicElement( )->pos( );
//      if( p1 != p2 ) {
//        if( p1.y( ) > p2.y( ) ) {
//          std::swap( map[ j ], map[ j + 1 ] );
//        }
//        else if( ( p1.y( ) == p2.y( ) ) && ( p1.x( ) > p2.x( ) ) ) {
//          std::swap( map[ j ], map[ j + 1 ] );
//        }
//      }
//      else {
//        p1 = map[ j ]->pos( );
//        p2 = map[ j + 1 ]->pos( );
//        if( p1.x( ) > p2.x( ) ) {
//          std::swap( map[ j ], map[ j + 1 ] );
//        }
//        else if( ( p1.x( ) == p2.x( ) ) && ( p1.y( ) > p2.y( ) ) ) {
//          std::swap( map[ j ], map[ j + 1 ] );
//        }
//      }
//    }
//  }
//}

bool comparePorts( QNEPort *port1, QNEPort *port2 ) {
  QPointF p1 = port1->graphicElement( )->pos( );
  QPointF p2 = port2->graphicElement( )->pos( );
  if( p1 != p2 ) {
    return( p1.y( ) < p2.y( ) || ( qFuzzyCompare(p1.y( ), p2.y( )) && p1.x( ) < p2.x( ) ) );
  }
  else {
    p1 = port1->pos( );
    p2 = port2->pos( );
    return( p1.x( ) < p2.x( ) || ( qFuzzyCompare(p1.x( ), p2.x( )) && p1.y( ) < p2.y( ) ) );
  }
}

void BoxPrototypeImpl::sortPorts( QVector< QNEPort* > &map ) {
  std::stable_sort( map.begin( ), map.end( ), comparePorts );
}

void BoxPrototypeImpl::loadFile( QString fileName ) {
  clear( );

  QFile file( fileName );
  if( file.open( QFile::ReadOnly ) ) {
    QDataStream ds( &file );
    QList< QGraphicsItem* > items = SerializationFunctions::load( ds, fileName );
    for( QGraphicsItem *item : items ) {
      loadItem( item );
    }
  }
  setInputSize( inputs.size( ) );
  setOutputSize( outputs.size( ) );

  sortPorts( inputs );
  sortPorts( outputs );

  loadInputs( );
  loadOutputs( );
}

void BoxPrototypeImpl::loadInputs( ) {
  for( int portIndex = 0; portIndex < getInputSize( ); ++portIndex ) {
    GraphicElement *elm = inputs.at( portIndex )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      lb = elm->objectName( );
    }
    if( !inputs.at( portIndex )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += inputs.at( portIndex )->portName( );
    }
    if( !elm->genericProperties( ).isEmpty( ) ) {
      lb += " [" + elm->genericProperties( ) + "]";
    }
    inputLabels[ portIndex ] = lb;
  }
}

void BoxPrototypeImpl::loadOutputs( ) {
  for( int portIndex = 0; portIndex < getOutputSize( ); ++portIndex ) {
    GraphicElement *elm = outputs.at( portIndex )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      lb = elm->objectName( );
    }
    if( !outputs.at( portIndex )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += outputs.at( portIndex )->portName( );
    }
    if( !elm->genericProperties( ).isEmpty( ) ) {
      lb += " [" + elm->genericProperties( ) + "]";
    }
    outputLabels[ portIndex ] = lb;
  }
}

void BoxPrototypeImpl::loadInputElement( GraphicElement *elm ) {
  for( QNEOutputPort *port : elm->outputs( ) ) {
    GraphicElement *nodeElm = ElementFactory::buildElement( ElementType::NODE );
    nodeElm->setPos( elm->pos( ) );
    nodeElm->setLabel( elm->getLabel( ) );
    QNEInputPort *nodeInput = nodeElm->input( );
    nodeInput->setPos( port->pos( ) );
    nodeInput->setName( port->getName( ) );
    nodeInput->setRequired( false );
    nodeInput->setDefaultValue( port->value( ) );
    nodeInput->setValue( port->value( ) );
    if( elm->elementType( ) == ElementType::CLOCK ) {
      nodeInput->setRequired( true );
    }
    inputs.append( nodeInput );
    elements.append( nodeElm );
    QList< QNEConnection* > conns = port->connections( );
    for( QNEConnection *conn: conns ) {
      if( port == conn->start( ) ) {
        conn->setStart( nodeElm->output( ) );
      }
    }
  }
  elm->disable( );
}

void BoxPrototypeImpl::loadOutputElement( GraphicElement *elm ) {
  for( QNEInputPort *port : elm->inputs( ) ) {
    GraphicElement *nodeElm = ElementFactory::buildElement( ElementType::NODE );
    nodeElm->setPos( elm->pos( ) );
    nodeElm->setLabel( elm->getLabel( ) );
    QNEOutputPort *nodeOutput = nodeElm->output( );
    nodeOutput->setPos( port->pos( ) );
    nodeOutput->setName( port->getName( ) );
    outputs.append( nodeOutput );
    elements.append( nodeElm );
    QList< QNEConnection* > conns = port->connections( );
    for( QNEConnection *conn: conns ) {
      if( port == conn->end( ) ) {
        conn->setEnd( nodeElm->input( ) );
      }
    }
  }
}

void BoxPrototypeImpl::loadItem( QGraphicsItem *item ) {
  if( item->type( ) == GraphicElement::Type ) {
    GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
    if( elm ) {
      switch( elm->elementType( ) ) {
          case ElementType::BUTTON:
          case ElementType::SWITCH:
          case ElementType::CLOCK: {
          loadInputElement( elm );
          break;
        }
          case ElementType::DISPLAY:
          case ElementType::LED:
          case ElementType::BUZZER: {
          loadOutputElement( elm );
          break;
        }
          default: {
          elements.append( elm );
          break;
        }
      }
    }
  }
}

void BoxPrototypeImpl::clear( ) {
  inputs.clear( );
  outputs.clear( );
  setInputSize( 0 );
  setOutputSize( 0 );
  qDeleteAll( elements );
  elements.clear( );
}

int BoxPrototypeImpl::getInputSize( ) const {
  return( inputs.size( ) );
}

int BoxPrototypeImpl::getOutputSize( ) const {
  return( outputs.size( ) );
}

void BoxPrototypeImpl::setOutputSize( int outputSize ) {
  outputLabels = QVector< QString >( outputSize );
}

void BoxPrototypeImpl::setInputSize( int inputSize ) {
  inputLabels = QVector< QString >( inputSize );
}
