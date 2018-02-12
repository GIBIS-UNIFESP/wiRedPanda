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
  sortMap( m_inputMap );
  sortMap( m_outputMap );

  setInputSize( m_inputMap.size( ) );
  setOutputSize( m_inputMap.size( ) );

  loadInputs( );
  loadOutputs( );
//  elements = SimulationController::sortElements( elements );

}

void BoxPrototypeImpl::loadInputs( ) {
  for( int portIndex = 0; portIndex < inputSize( ); ++portIndex ) {
    GraphicElement *elm = m_inputMap.at( portIndex )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      lb = elm->objectName( );
    }
    if( !m_inputMap.at( portIndex )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += m_inputMap.at( portIndex )->portName( );
    }
    if( !elm->genericProperties( ).isEmpty( ) ) {
      lb += " [" + elm->genericProperties( ) + "]";
    }
    m_inputLabels[ portIndex ] = lb;
    if( elm->elementType( ) != ElementType::CLOCK ) {
      m_requiredInputs[ portIndex ] = false;
      m_defaultInputValues[ portIndex ] = m_inputMap.at( portIndex )->value( );
    }
  }
}

void BoxPrototypeImpl::loadOutputs( ) {
  for( int portIndex = 0; portIndex < outputSize( ); ++portIndex ) {
    GraphicElement *elm = m_outputMap.at( portIndex )->graphicElement( );
    QString lb = elm->getLabel( );
    if( lb.isEmpty( ) ) {
      lb = elm->objectName( );
    }
    if( !m_outputMap.at( portIndex )->portName( ).isEmpty( ) ) {
      lb += " ";
      lb += m_outputMap.at( portIndex )->portName( );
    }
    if( !elm->genericProperties( ).isEmpty( ) ) {
      lb += " [" + elm->genericProperties( ) + "]";
    }
    m_outputLabels[ portIndex ] = lb;
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
    m_elements.append( elm );
    if( elm ) {
      switch( elm->elementType( ) ) {
          case ElementType::BUTTON:
          case ElementType::SWITCH:
          case ElementType::CLOCK: {
          for( QNEPort *port : elm->outputs( ) ) {
            m_inputMap.append( port );
          }
          elm->disable( );
          break;
        }
          case ElementType::DISPLAY:
          case ElementType::LED: {
          for( QNEPort *port : elm->inputs( ) ) {
            m_outputMap.append( port );
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
  qDeleteAll( m_elements );
  m_elements.clear( );
}

int BoxPrototypeImpl::inputSize( ) const {
  return( m_inputSize );
}

int BoxPrototypeImpl::outputSize( ) const {
  return( m_outputSize );
}

void BoxPrototypeImpl::setOutputSize( int outputSize ) {
  m_outputSize = outputSize;
}

void BoxPrototypeImpl::setInputSize( int inputSize ) {
  Q_ASSERT( inputSize > 0 );
  m_inputSize = inputSize;
  m_defaultInputValues = QVector< bool >( inputSize, false );
  m_requiredInputs = QVector< bool >( inputSize, false );
}
