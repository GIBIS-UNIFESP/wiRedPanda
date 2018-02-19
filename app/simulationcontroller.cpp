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
  if( canRun( ) ) {
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
}

void SimulationController::updateView( ) {
  updateScene( scene->views( ).first( )->sceneRect( ) );
}

bool SimulationController::canRun( ) {
  if( !elMapping ) {
    return( false );
  }
  return( elMapping->canRun( ) );
}

bool SimulationController::isRunning( ) {
  return( this->simulationTimer.isActive( ) );
}

void SimulationController::update( ) {
  if( elMapping ) {
    elMapping->update( );
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
  COMMENT( "GENERATING SIMULATION LAYER", 1 );
  QVector< GraphicElement* > elements = scene->getElements( );
  if( elements.size( ) == 0 ) {
    return;
  }
  if( elMapping ) {
    delete elMapping;
  }
  elMapping = new ElementMapping( GlobalProperties::currentFile, scene->getElements( ) );
  if( elMapping->canInitialize( ) ) {
    elMapping->initialize( );
    elMapping->sort( );
    update( );
  }
  else {
    qDebug( ) << "Cannot initialize simulation!";
  }
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
    logElm = elMapping->getBoxMapping( box )->getOutput( port->index( ) );
  }
  else {
    logElm = elMapping->getLogicElement( elm );
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
  LogicElement *logElm = elMapping->getLogicElement( elm );
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
