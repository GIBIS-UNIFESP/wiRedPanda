#include "box.h"
#include "boxmapping.h"
#include "elementfactory.h"
#include "simulationcontroller.h"
#include "simulationcontroller.h"

#include <QDebug>
#include <limits>

#include <element/clock.h>

#include <nodes/qneconnection.h>

#include <QGraphicsView>
#include <QStack>

SimulationController::SimulationController( Scene *scn ) : QObject( dynamic_cast< QObject* >( scn ) ), elMapping(
    nullptr ), timer( this ) {
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

bool SimulationController::isRunning( ) {
  return( this->timer.isActive( ) );
}

void SimulationController::update( ) {
  if( elMapping ) {
    for( Clock *clk : elMapping->clocks ) {
      if( Clock::reset ) {
        clk->resetClock( );
      }
      else {
        clk->updateLogic( );
      }
    }
    Clock::reset = false;
    for( auto iter = elMapping->inputMap.begin( ); iter != elMapping->inputMap.end( ); ++iter ) {
      iter.value( )->setOutputValue( iter.key( )->getOn( ) );
    }
    for( LogicElement *elm : elMapping->logicElms ) {
      elm->updateLogic( );
    }
    updateScene( scene->views( ).first( )->sceneRect( ) );
  }
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
  if( elMapping ) {
    delete elMapping;
  }
  elMapping = new ElementMapping( scene->getElements( ) );
  elMapping->initialize( );
  elMapping->sort( );
  update( );
}

void SimulationController::clear( ) {
  if( elMapping ) {
    delete elMapping;
  }
  elMapping = nullptr;
}

void SimulationController::updateGraphicElement( QNEPort *port ) {
  if( port && port->graphicElement( ) ) {
    GraphicElement *elm = port->graphicElement( );
    if( port->isOutput( ) ) {
      LogicElement *logElm = nullptr;
      int portIndex = 0;
      if( elm->elementType( ) == ElementType::BOX ) {
        Box *box = dynamic_cast< Box* >( elm );
        logElm = elMapping->boxMappings[ box ]->getOutput( port->index( ) );
      }
      else {
        logElm = elMapping->map[ elm ];
        portIndex = port->index( );
      }
      Q_ASSERT( logElm );
      if( logElm->isValid( ) ) {
        port->setValue( logElm->getOutputValue( portIndex ) );
      }
      else {
        port->setValue( -1 );
      }
    }
    else if( elm->elementGroup( ) == ElementGroup::OUTPUT ) {
      elm->updateLogic( );
    }
  }
}

void SimulationController::updateConnection( QNEConnection *conn ) {
  if( conn ) {
    QNEPort *p1 = dynamic_cast< QNEPort* >( conn->start( ) );
    QNEPort *p2 = dynamic_cast< QNEPort* >( conn->end( ) );
    updateGraphicElement( p1 );
    updateGraphicElement( p2 );
  }
}
