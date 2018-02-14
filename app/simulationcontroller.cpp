#include "elementfactory.h"
#include "simulationcontroller.h"
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
  clear( );
}




void SimulationController::updateScene( const QRectF &rect ) {
  const QList< QGraphicsItem* > &items = scene->items( rect );
  for( QGraphicsItem *item: items ) {
    QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
    updateConnection( conn );
  }
}

void SimulationController::update( ) {
  for( Clock *clk : elMapping.clocks ) {
    clk->updateLogic( );
  }
  for( auto iter = elMapping.inputMap.begin( ); iter != elMapping.inputMap.end( ); ++iter ) {
    iter.value( )->setOutputValue( iter.key( )->getOn( ) );
  }
  for( LogicElement *elm : elMapping.logicElms ) {
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


void SimulationController::reSortElms( ) {
  COMMENT( "GENERATING SIMULATION LAYER", 0 );
  QVector< GraphicElement* > elements = scene->getElements( );
  if( elements.size( ) == 0 ) {
    return;
  }
  elMapping.resortElements( scene->getElements( ) );
  update( );
}

void SimulationController::clear( ) {
}

void SimulationController::updateGraphicElement( QNEPort *port ) {
  if( port && port->graphicElement( ) ) {
    GraphicElement *elm = port->graphicElement( );
    if( port->isOutput( ) ) {
      if( elMapping.map.contains( elm ) ) {
        LogicElement *logElm = elMapping.map[ elm ];
        if( logElm->isValid( ) ) {
          port->setValue( logElm->getOutputValue( port->index( ) ) );
        }
        else {
          port->setValue( -1 );
        }
      }
    }
    else if( elm->elementGroup( ) == ElementGroup::OUTPUT ) {
      elm->updateLogic( );
    }
  }
}

void SimulationController::updateConnection( QNEConnection *conn ) {
  if( conn ) {
    QNEPort *p1 = conn->port1( );
    QNEPort *p2 = conn->port2( );
    updateGraphicElement( p1 );
    updateGraphicElement( p2 );
  }
}
