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
    nullptr ), simulationTimer( this ) {
  scene = scn;
  simulationTimer.setInterval( GLOBALCLK );
  viewTimer.setInterval( int( 1000 / 30 ) );
  viewTimer.start( );
  connect( &viewTimer, &QTimer::timeout, this, &SimulationController::updateView );
  connect( &simulationTimer, &QTimer::timeout, this, &SimulationController::update );
}

SimulationController::~SimulationController( ) {
  clear( );
}

void SimulationController::updateScene( const QRectF &rect ) {
  const QList< QGraphicsItem* > &items = scene->items( rect );
  for( QGraphicsItem *item: items ) {
    QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
    GraphicElement *elm = qgraphicsitem_cast< GraphicElement* >( item );
    if( conn ) {
      updateConnection( conn );
    }
    else if( elm && ( elm->elementGroup( ) == ElementGroup::OUTPUT ) ) {
      for( QNEInputPort *in: elm->inputs( ) ) {
        updatePort( in );
      }
    }
  }
}

void SimulationController::updateView( ) {
  updateScene( scene->views( ).first( )->sceneRect( ) );
}

bool SimulationController::isRunning( ) {
  return( this->simulationTimer.isActive( ) );
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
  }
}

void SimulationController::stop( ) {
  simulationTimer.stop( );
}

void SimulationController::start( ) {
  Clock::reset = true;
  reSortElms( );
  simulationTimer.start( );
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
  elMapping = new ElementMapping( GlobalProperties::currentFile, scene->getElements( ) );
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

void SimulationController::updatePort( QNEOutputPort *port ) {
  Q_ASSERT( port );
  GraphicElement *elm = port->graphicElement( );
  Q_ASSERT( elm );
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

void SimulationController::updatePort( QNEInputPort *port ) {
  Q_ASSERT( port );
  GraphicElement *elm = port->graphicElement( );
  Q_ASSERT( elm );
  LogicElement *logElm = elMapping->map[ elm ];
  Q_ASSERT( logElm );
  int portIndex = port->index( );
  if( logElm->isValid( ) ) {
    port->setValue( logElm->getInputValue( portIndex ) );
  }
  else {
    port->setValue( -1 );
  }
  if( elm->elementGroup( ) == ElementGroup::OUTPUT ) {
    elm->updateLogic( );
  }
}

void SimulationController::updateConnection( QNEConnection *conn ) {
  Q_ASSERT( conn );
  updatePort( conn->start( ) );
}
