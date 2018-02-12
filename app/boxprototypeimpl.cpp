#include "boxprototypeimpl.h"
#include "editor.h"
#include "elementfactory.h"
#include "serializationfunctions.h"

#include <QFile>

void BoxPrototypeImpl::loadFile( QString fileName ) {
  QFile file( fileName );
  if( file.open( QFile::ReadOnly ) ) {
    QDataStream ds( &file );
    QList< QGraphicsItem* > items = SerializationFunctions::load( Editor::globalEditor, ds, fileName );
    for( QGraphicsItem *item : items ) {
      loadItem( item );
    }
  }
  setInputSize( inputMap.size( ) );
  setOutputSize( outputMap.size( ) );

  sortMap( inputMap );
  sortMap( outputMap );

  loadInputs( );
  loadOutputs( );
//  elements = SimulationController::sortElements( elements );

}

void BoxPrototypeImpl::loadInputs( ) {
  for( int portIndex = 0; portIndex < getInputSize( ); ++portIndex ) {
    GraphicElement *elm = inputMap.at( portIndex )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      lb = elm->objectName( );
    }
    if( !inputMap.at( portIndex )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += inputMap.at( portIndex )->portName( );
    }
    if( !elm->genericProperties( ).isEmpty( ) ) {
      lb += " [" + elm->genericProperties( ) + "]";
    }
    inputLabels[ portIndex ] = lb;
    if( elm->elementType( ) != ElementType::CLOCK ) {
      requiredInputs[ portIndex ] = false;
      defaultInputValues[ portIndex ] = inputMap.at( portIndex )->value( );
    }
  }
}

void BoxPrototypeImpl::loadOutputs( ) {
  for( int portIndex = 0; portIndex < getOutputSize( ); ++portIndex ) {
    GraphicElement *elm = outputMap.at( portIndex )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      lb = elm->objectName( );
    }
    if( !outputMap.at( portIndex )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += outputMap.at( portIndex )->portName( );
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

void BoxPrototypeImpl::loadItem( QGraphicsItem *item ) {
  if( item->type( ) == GraphicElement::Type ) {
    GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
    elements.append( elm );
    if( elm ) {
      switch( elm->elementType( ) ) {
          case ElementType::BUTTON:
          case ElementType::SWITCH:
          case ElementType::CLOCK: {
          for( QNEPort *port : elm->outputs( ) ) {
            inputMap.append( port );
          }
          elm->disable( );
          break;
        }
          case ElementType::DISPLAY:
          case ElementType::LED: {
          for( QNEPort *port : elm->inputs( ) ) {
            outputMap.append( port );
          }
          break;
        }
          default: {
          break;
        }
      }
    }
  }
}

void BoxPrototypeImpl::clear( ) {
  setInputSize( 0 );
  setOutputSize( 0 );
  qDeleteAll( elements );
  elements.clear( );
}

int BoxPrototypeImpl::getInputSize( ) const {
  return( inputMap.size( ) );
}

int BoxPrototypeImpl::getOutputSize( ) const {
  return( outputMap.size( ) );
}

void BoxPrototypeImpl::setOutputSize( int outputSize ) {
  outputLabels = QVector< QString >( outputSize );
}

void BoxPrototypeImpl::setInputSize( int inputSize ) {
  inputLabels = QVector< QString >( inputSize );
  defaultInputValues = QVector< bool >( inputSize, false );
  requiredInputs = QVector< bool >( inputSize, true );
}
