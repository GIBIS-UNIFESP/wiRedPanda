#include "elementfactory.h"
#include "simulationcontroller.h"

#include <QDebug>
#include <limits>

#include <element/clock.h>

#include <nodes/qneconnection.h>

#include <QGraphicsView>
#include <QStack>

SimulationController::SimulationController( Scene *scn ) : QObject( dynamic_cast< QObject* >( scn ) ), timer( this ) {
  scene = scn;
  timer.setInterval( GLOBALCLK );
  connect( &timer, &QTimer::timeout, this, &SimulationController::update );
}

SimulationController::~SimulationController( ) {

}

int SimulationController::calculatePriority( GraphicElement *elm,
                                             QMap< GraphicElement*, bool > &beingvisited,
                                             QMap< GraphicElement*, int > &priority ) {
  if( !elm ) {
    return( 0 );
  }
  if( beingvisited.contains( elm ) && ( beingvisited[ elm ] == true ) ) {

    return( 0 );
  }
  if( priority.contains( elm ) ) {
    return( priority[ elm ] );
  }
  beingvisited[ elm ] = true;
  int max = 0;
  for( QNEPort *port : elm->outputs( ) ) {
    for( QNEConnection *conn : port->connections( ) ) {
      QNEPort *sucessor = conn->otherPort( port );
      if( sucessor ) {
        max = qMax( calculatePriority( sucessor->graphicElement( ), beingvisited, priority ), max );
      }
    }
  }
  int p = max + 1;
  priority[ elm ] = p;
  beingvisited[ elm ] = false;
  return( p );
}

QVector< GraphicElement* > SimulationController::sortElements( QVector< GraphicElement* > elms ) {
  QMap< GraphicElement*, bool > beingvisited;
  QMap< GraphicElement*, int > priority;
  for( GraphicElement *elm : elms ) {
    calculatePriority( elm, beingvisited, priority );
  }
  std::sort( elms.begin( ), elms.end( ), [ priority ]( GraphicElement *e1, GraphicElement *e2 ) {
    return( priority[ e2 ] < priority[ e1 ] );
  } );

  return( elms );
}


void SimulationController::updateScene( const QRectF &rect ) {
  const QList< QGraphicsItem* > &items = scene->items( rect );
  for( QGraphicsItem *item: items ) {
    QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
    if( conn ) {
      QNEPort *p1 = conn->port1( );
      QNEPort *p2 = conn->port2( );
      if( p1 && p1->graphicElement( ) ) {
        GraphicElement *elm1 = p1->graphicElement( );
        if( p1->isOutput( ) ) {
          LogicElement *logElm1 = m_map[ elm1 ];
          if( logElm1->isValid( ) ) {
            p1->setValue( logElm1->getOutputValue( p1->index( ) ) );
          }
          else {
            p1->setValue( -1 );
          }
        }
        else if( elm1->elementGroup( ) == ElementGroup::OUTPUT ) {
          elm1->updateLogic( );
        }
      }
      if( p2 && p2->graphicElement( ) ) {
        GraphicElement *elm2 = p2->graphicElement( );
        if( p2->isOutput( ) ) {
          LogicElement *logElm2 = m_map[ elm2 ];
          if( logElm2->isValid( ) ) {
            p2->setValue( logElm2->getOutputValue( p2->index( ) ) );
          }
          else {
            p2->setValue( -1 );
          }
        }
        else if( elm2->elementGroup( ) == ElementGroup::OUTPUT ) {
          elm2->updateLogic( );
        }
      }
    }
  }
}

void SimulationController::update( ) {
  for( Clock *clk : m_clocks ) {
    clk->updateLogic( );
  }
  for( auto iter = m_inputMap.begin( ); iter != m_inputMap.end( ); ++iter ) {
    iter.value( )->setOutputValue( iter.key( )->getOn( ) );
  }
  for( LogicElement *elm : m_logicElms ) {
    elm->updateLogic( );
  }
  updateScene( scene->views( ).first( )->sceneRect( ) );
}

void SimulationController::stop( ) {
  timer.stop( );
}

void SimulationController::start( ) {
  Clock::reset = true;
  reSortElms( );
  timer.start( );
}

LogicElement* buildLogicElement( GraphicElement *elm ) {
  switch( elm->elementType( ) ) {
      case ElementType::SWITCH:
      case ElementType::BUTTON:
      case ElementType::CLOCK:
      return( new LogicInput( ) );
      case ElementType::NODE:
      return( new LogicNode( ) );
      case ElementType::VCC:
      return( new LogicInput( true ) );
      case ElementType::GND:
      return( new LogicInput( false ) );
      case ElementType::LED:
      case ElementType::DISPLAY:
      return( new LogicOutput( elm->inputSize( ) ) );
      case ElementType::AND:
      return( new LogicAnd( elm->inputSize( ) ) );
      case ElementType::OR:
      return( new LogicOr( elm->inputSize( ) ) );
      case ElementType::NAND:
      return( new LogicNand( elm->inputSize( ) ) );
      case ElementType::NOR:
      return( new LogicNor( elm->inputSize( ) ) );
      case ElementType::XOR:
      return( new LogicXor( elm->inputSize( ) ) );
      case ElementType::XNOR:
      return( new LogicXnor( elm->inputSize( ) ) );
      case ElementType::NOT:
      return( new LogicNot( ) );
      case ElementType::JKFLIPFLOP:
      return( new LogicJKFlipFlop( ) );

      default:
      throw std::runtime_error( "Not implemented yet: " + elm->objectName( ).toStdString( ) );
      break;
  }
  return( nullptr );
}


void sortLogicElements( QVector< LogicElement* > &elms ) {
  for( LogicElement *elm : elms ) {
    elm->calculatePriority( );
  }
  std::sort( elms.begin( ), elms.end( ) );
}

void SimulationController::reSortElms( ) {
  COMMENT( "GENERATING SIMULATION LAYER", 0 );
  clear( );
  QVector< GraphicElement* > elements = scene->getElements( );
  if( elements.size( ) == 0 ) {
    return;
  }
  m_logicElms.resize( elements.size( ) );
  for( int idx = 0; idx < elements.size( ); ++idx ) {
    GraphicElement *elm = elements[ idx ];
    if( elm->elementType( ) == ElementType::CLOCK ) {
      Clock *clk = dynamic_cast< Clock* >( elm );
      Q_ASSERT( clk != nullptr );
      m_clocks.append( clk );
    }
    LogicElement *logicElm = buildLogicElement( elm );
    m_logicElms[ idx ] = logicElm;
    m_map.insert( elm, logicElm );
    Input *in = dynamic_cast< Input* >( elm );
    if( in ) {
      m_inputMap[ in ] = logicElm;
    }
  }
  for( GraphicElement *elm : elements ) {
    for( QNEPort *in : elm->inputs( ) ) {
      if( in->connections( ).size( ) == 1 ) {
        QNEPort *other_out = in->connections( ).first( )->otherPort( in );
        if( other_out ) {
          GraphicElement *other_elm = other_out->graphicElement( );
          if( other_elm ) {
            m_map[ elm ]->connectInput( in->index( ), m_map[ other_elm ], other_out->index( ) );
          }
        }
      }
    }
  }
  sortLogicElements( m_logicElms );
  for( LogicElement *elm : m_logicElms ) {
    elm->validate( );
    elm->updateLogic( );
  }
}

void SimulationController::clear( ) {
  for( LogicElement *elm: m_logicElms ) {
    delete elm;
  }
  m_logicElms.clear( );
  m_map.clear( );
  m_inputMap.clear( );
  m_clocks.clear( );
}
