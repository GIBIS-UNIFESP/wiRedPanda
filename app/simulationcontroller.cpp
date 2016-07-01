#include "simulationcontroller.h"

#include <QDebug>
#include <limits>

#include <element/clock.h>

#include <nodes/qneconnection.h>

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
    elm->setChanged( true );
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
  QVector< GraphicElement* > elements = scene->getElements( );
  if( Clock::reset ) {
    for( GraphicElement *elm : elements ) {
      if( elm->elementType( ) == ElementType::CLOCK ) {
        Clock *clk = dynamic_cast< Clock* >( elm );
        if( clk ) {
          clk->resetClock( );
        }
      }
    }
    Clock::reset = false;
  }
  if( elements.isEmpty( ) ) {
    return;
  }
  for( GraphicElement *elm : sortedElements ) {
    elm->updateLogic( );
    elm->setChanged( false );
  }
}

void SimulationController::stop( ) {
  timer.stop( );
}

void SimulationController::start( ) {
  timer.start( );
  Clock::reset = true;
  reSortElms();
}

void SimulationController::reSortElms( ) {
  qDebug() << "Re-sorting elements";
  QVector< GraphicElement* > elements = scene->getElements( );
  sortedElements = sortElements( elements );
}
