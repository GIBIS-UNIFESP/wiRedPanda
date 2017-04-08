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


void SimulationController::update( ) {
  QMap< Input*, LogicElement* >::iterator iter = inputMap.begin( );
  for( ; iter != inputMap.end( ); ++iter ) {
    iter.value( )->setOutputValue( iter.key( )->getOn( ) );
  }
  for( LogicElement *elm : logicElms ) {
    elm->updateLogic( );
  }
  const QList< QGraphicsItem* > &items = scene->items( scene->views( ).first( )->sceneRect( ) );
  for( QGraphicsItem *item: items ) {
    QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
    if( conn ) {
      QNEPort *p1 = conn->port1( );
      QNEPort *p2 = conn->port2( );
      if( p1 && p1->graphicElement( ) ) {
        GraphicElement *elm1 = p1->graphicElement( );
        if( p1->isOutput( ) ) {
          p1->setValue( map[ elm1 ]->getOutputValue( p1->index( ) ) );
        }
        else if( elm1->elementGroup( ) == ElementGroup::OUTPUT ) {
          elm1->updateLogic( );
        }
      }
      if( p2 && p2->graphicElement( ) ) {
        GraphicElement *elm2 = p2->graphicElement( );
        if( p2->isOutput( ) ) {
          p2->setValue( map[ elm2 ]->getOutputValue( p2->index( ) ) );
        }
        else if( elm2->elementGroup( ) == ElementGroup::OUTPUT ) {
          elm2->updateLogic( );
        }
      }
    }
  }
}

void SimulationController::stop( ) {
  timer.stop( );
}

void SimulationController::start( ) {
  timer.start( );
  Clock::reset = true;
  reSortElms( );
}

LogicElement* buildLogicElement( GraphicElement *elm ) {
  switch( elm->elementType( ) ) {
      case ElementType::SWITCH:
      case ElementType::BUTTON:
      case ElementType::CLOCK:
      return( new LogicInput( ) );
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
      default:
      throw std::runtime_error( "Not implemented yet" );
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
  QVector< GraphicElement* > elements = scene->getElements( );
  logicElms.resize( elements.size( ) );
  for( int idx = 0; idx < elements.size( ); ++idx ) {
    GraphicElement *elm = elements[ idx ];
    LogicElement *logicElm = buildLogicElement( elm );
    logicElms[ idx ] = logicElm;
    map.insert( elm, logicElm );
    Input *in = dynamic_cast< Input* >( elm );
    if( in ) {
      inputMap[ in ] = logicElm;
    }
  }
  for( GraphicElement *elm : elements ) {
    for( QNEPort *in : elm->inputs( ) ) {
      if( in->connections( ).size( ) == 1 ) {
        QNEPort *other_out = in->connections( ).first( )->otherPort( in );
        if( other_out ) {
          GraphicElement *other_elm = other_out->graphicElement( );
          if( other_elm ) {
            map[ elm ]->connectInput( in->index( ), map[ other_elm ], other_out->index( ) );
          }
        }
      }
    }
  }
  sortLogicElements( logicElms );
  for( LogicElement *elm : logicElms ) {
    elm->validate( );
    elm->updateLogic( );
  }
}
