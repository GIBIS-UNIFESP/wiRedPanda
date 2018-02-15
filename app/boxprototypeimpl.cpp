#include "boxprototypeimpl.h"
#include "editor.h"
#include "elementfactory.h"
#include "serializationfunctions.h"

#include <QFile>

BoxPrototypeImpl::~BoxPrototypeImpl( ) {
  for( GraphicElement *elm: elements ) {
    delete elm;
  }
}

void BoxPrototypeImpl::loadFile( QString fileName ) {
  clear( );

  QFile file( fileName );
  if( file.open( QFile::ReadOnly ) ) {
    QDataStream ds( &file );
    QList< QGraphicsItem* > items = SerializationFunctions::load( Editor::globalEditor, ds, fileName );
    for( QGraphicsItem *item : items ) {
      loadItem( item );
    }
  }
  setInputSize( referenceInputs.size( ) );
  setOutputSize( referenceOutputs.size( ) );

  sortMap( referenceInputs );
  sortMap( referenceOutputs );

  loadInputs( );
  loadOutputs( );
//  elements = SimulationController::sortElements( elements );

}

void BoxPrototypeImpl::loadInputs( ) {
  for( int portIndex = 0; portIndex < getInputSize( ); ++portIndex ) {
    GraphicElement *elm = referenceInputs.at( portIndex )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      lb = elm->objectName( );
    }
    if( !referenceInputs.at( portIndex )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += referenceInputs.at( portIndex )->portName( );
    }
    if( !elm->genericProperties( ).isEmpty( ) ) {
      lb += " [" + elm->genericProperties( ) + "]";
    }
    inputLabels[ portIndex ] = lb;
    if( elm->elementType( ) != ElementType::CLOCK ) {
      requiredInputs[ portIndex ] = false;
      defaultInputValues[ portIndex ] = referenceInputs.at( portIndex )->value( );
    }
  }
}

void BoxPrototypeImpl::loadOutputs( ) {
  for( int portIndex = 0; portIndex < getOutputSize( ); ++portIndex ) {
    GraphicElement *elm = referenceOutputs.at( portIndex )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      lb = elm->objectName( );
    }
    if( !referenceOutputs.at( portIndex )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += referenceOutputs.at( portIndex )->portName( );
    }
    if( !elm->genericProperties( ).isEmpty( ) ) {
      lb += " [" + elm->genericProperties( ) + "]";
    }
    outputLabels[ portIndex ] = lb;
  }
}

void BoxPrototypeImpl::sortMap( QVector< QNEPort* > &map ) {
  // BubbleSort
  for( int i = map.size( ) - 1; i >= 1; i-- ) {
    for( int j = 0; j < i; j++ ) {
      QPointF p1 = map[ j ]->graphicElement( )->pos( );
      QPointF p2 = map[ j + 1 ]->graphicElement( )->pos( );
      if( p1 != p2 ) {
        if( p1.y( ) > p2.y( ) ) {
          std::swap( map[ j ], map[ j + 1 ] );
        }
        else if( ( p1.y( ) == p2.y( ) ) && ( p1.x( ) > p2.x( ) ) ) {
          std::swap( map[ j ], map[ j + 1 ] );
        }
      }
      else {
        p1 = map[ j ]->pos( );
        p2 = map[ j + 1 ]->pos( );
        if( p1.x( ) > p2.x( ) ) {
          std::swap( map[ j ], map[ j + 1 ] );
        }
        else if( ( p1.x( ) == p2.x( ) ) && ( p1.y( ) > p2.y( ) ) ) {
          std::swap( map[ j ], map[ j + 1 ] );
        }
      }
    }
  }
}

void BoxPrototypeImpl::loadInput( GraphicElement *elm ) {
  for( QNEPort *port : elm->outputs( ) ) {
    referenceInputs.append( port );


    Q_ASSERT( Editor::globalEditor );
    GraphicElement *nodeElm = ElementFactory::buildElement( ElementType::NODE, Editor::globalEditor );
    elements.append( nodeElm );
    for( QNEConnection *conn: port->connections( ) ) {
      if( conn->port1( ) == port ) {
        conn->setPort1( nodeElm->output( ) );
      }
      else {
        conn->setPort2( nodeElm->output( ) );
      }
    }
    inputs.append( nodeElm );
  }
  elm->disable( );
}

void BoxPrototypeImpl::loadOutput( GraphicElement *elm ) {
  for( QNEPort *port : elm->inputs( ) ) {
    referenceOutputs.append( port );

    Q_ASSERT( Editor::globalEditor );
    GraphicElement *nodeElm = ElementFactory::buildElement( ElementType::NODE, Editor::globalEditor );
    elements.append( nodeElm );
    for( QNEConnection *conn: port->connections( ) ) {
      if( conn->port1( ) == port ) {
        conn->setPort1( nodeElm->input( ) );
      }
      else {
        conn->setPort2( nodeElm->input( ) );
      }
    }
    outputs.append( nodeElm );
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
          loadInput( elm );
          break;
        }
          case ElementType::DISPLAY:
          case ElementType::LED: {
          loadOutput( elm );
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
  referenceInputs.clear( );
  referenceOutputs.clear( );
  setInputSize( 0 );
  setOutputSize( 0 );
  qDeleteAll( elements );
  elements.clear( );
}

int BoxPrototypeImpl::getInputSize( ) const {
  return( referenceInputs.size( ) );
}

int BoxPrototypeImpl::getOutputSize( ) const {
  return( referenceOutputs.size( ) );
}

void BoxPrototypeImpl::setOutputSize( int outputSize ) {
  outputLabels = QVector< QString >( outputSize );
}

void BoxPrototypeImpl::setInputSize( int inputSize ) {
  inputLabels = QVector< QString >( inputSize );
  defaultInputValues = QVector< bool >( inputSize, false );
  requiredInputs = QVector< bool >( inputSize, true );
}
