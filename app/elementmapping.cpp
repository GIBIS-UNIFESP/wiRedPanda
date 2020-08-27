#include "box.h"
#include "boxmanager.h"
#include "boxmapping.h"
#include "boxprototype.h"
#include "clock.h"
#include "elementmapping.h"
#include "qneconnection.h"

#include "logicelement/logicand.h"
#include "logicelement/logicdemux.h"
#include "logicelement/logicdflipflop.h"
#include "logicelement/logicdlatch.h"
#include "logicelement/logicjkflipflop.h"
#include "logicelement/logicmux.h"
#include "logicelement/logicnand.h"
#include "logicelement/logicnode.h"
#include "logicelement/logicnor.h"
#include "logicelement/logicnot.h"
#include "logicelement/logicor.h"
#include "logicelement/logicoutput.h"
#include "logicelement/logicsrflipflop.h"
#include "logicelement/logictflipflop.h"
#include "logicelement/logicxnor.h"
#include "logicelement/logicxor.h"

#include <QDebug>

ElementMapping::ElementMapping( const QVector< GraphicElement* > &elms, QString file ) :
  currentFile( file ),
  initialized( false ),
  elements( elms ),
  globalGND( false ),
  globalVCC( true ) {
}

ElementMapping::~ElementMapping( ) {
  clear( );
}

void ElementMapping::clear( ) {
  initialized = false;
  globalGND.clearSucessors( );
  globalVCC.clearSucessors( );
  for( LogicElement *elm: deletableElements ) {
    delete elm;
  }
  deletableElements.clear( );
  for( BoxMapping *boxMap : boxMappings ) {
    delete boxMap;
  }
  boxMappings.clear( );
  map.clear( );
  inputMap.clear( );
  clocks.clear( );
  logicElms.clear( );
}



QVector< GraphicElement* > ElementMapping::sortGraphicElements( QVector< GraphicElement* > elms ) {
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

void ElementMapping::insertElement( GraphicElement *elm ) {
  LogicElement *logicElm = buildLogicElement( elm );
  deletableElements.append( logicElm );
  logicElms.append( logicElm );
  map.insert( elm, logicElm );
  Input *in = dynamic_cast< Input* >( elm );
  if( in ) {
    inputMap[ in ] = logicElm;
  }
}

void ElementMapping::insertBox( Box *box ) {
  Q_ASSERT( box );
  Q_ASSERT( !boxMappings.contains( box ) );
  BoxPrototype *proto = box->getPrototype( );
  if( proto ) {
    BoxMapping *boxMap = proto->generateMapping( );
    Q_ASSERT( boxMap );
    boxMap->initialize( );
    boxMappings.insert( box, boxMap );
    logicElms.append( boxMap->logicElms );
  }
}

void ElementMapping::generateMap( ) {
  for( GraphicElement *elm: elements ) {
    if( elm->elementType( ) == ElementType::CLOCK ) {
      Clock *clk = dynamic_cast< Clock* >( elm );
      Q_ASSERT( clk != nullptr );
      clocks.append( clk );
    }
    if( elm->elementType( ) == ElementType::BOX ) {
      Box *box = dynamic_cast< Box* >( elm );
      insertBox( box );
    }
    else {
      insertElement( elm );
    }
  }
}

LogicElement* ElementMapping::buildLogicElement( GraphicElement *elm ) {
  switch( elm->elementType( ) ) {
      case ElementType::SWITCH:
      case ElementType::BUTTON:
      case ElementType::CLOCK:
      return( new LogicInput( ) );
      case ElementType::LED:
      case ElementType::BUZZER:
      case ElementType::DISPLAY:
      case ElementType::DISPLAY14:
      return( new LogicOutput( elm->inputSize( ) ) );
      case ElementType::NODE:
      return( new LogicNode( ) );
      case ElementType::VCC:
      return( new LogicInput( true ) );
      case ElementType::GND:
      return( new LogicInput( false ) );
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
      case ElementType::SRFLIPFLOP:
      return( new LogicSRFlipFlop( ) );
      case ElementType::TFLIPFLOP:
      return( new LogicTFlipFlop( ) );
      case ElementType::DFLIPFLOP:
      return( new LogicDFlipFlop( ) );
      case ElementType::DLATCH:
      return( new LogicDLatch( ) );
      case ElementType::MUX:
      return( new LogicMux( ) );
      case ElementType::DEMUX:
      return( new LogicDemux( ) );
      default:
      throw std::runtime_error( "Not implemented yet: " + elm->objectName( ).toStdString( ) );
  }
}

void ElementMapping::initialize( ) {
  clear( );
  generateMap( );
  connectElements( );
  initialized = true;
}

void ElementMapping::sort( ) {
  sortLogicElements( );
  validateElements( );
}

void ElementMapping::update( ) {
  if( canRun( ) ) {
    for( Clock *clk : clocks ) {
      if( Clock::reset ) {
        clk->resetClock( );
      }
      else {
        clk->updateClock( );
      }
    }
    Clock::reset = false;
    for( auto iter = inputMap.begin( ); iter != inputMap.end( ); ++iter ) {
      iter.value( )->setOutputValue( iter.key( )->getOn( ) );
    }
    for( LogicElement *elm : logicElms ) {
      elm->updateLogic( );
    }
  }
}

BoxMapping* ElementMapping::getBoxMapping( Box *box ) const {
  Q_ASSERT( box );
  return( boxMappings[ box ] );
}

LogicElement* ElementMapping::getLogicElement( GraphicElement *elm ) const {
  Q_ASSERT( elm );
  return( map[ elm ] );
}

bool ElementMapping::canRun( ) const {
  return( initialized );
}

bool ElementMapping::canInitialize( ) const {
  for( GraphicElement *elm: elements ) {
    if( elm->elementType( ) == ElementType::BOX ) {
      Box *box = dynamic_cast< Box* >( elm );
      BoxPrototype *prototype = BoxManager::instance( )->getPrototype( box->getFile( ) );
      if( !box || !prototype ) {
        return( false );
      }
    }
  }
  return( true );
}

void ElementMapping::applyConnection( GraphicElement *elm, QNEPort *in ) {
  LogicElement *currentLogElm;
  int inputIndex = 0;
  if( elm->elementType( ) == ElementType::BOX ) {
    Box *box = dynamic_cast< Box* >( elm );
    Q_ASSERT( box );
    currentLogElm = boxMappings[ box ]->getInput( in->index( ) );
  }
  else {
    currentLogElm = map[ elm ];
    inputIndex = in->index( );
  }
  Q_ASSERT( currentLogElm );
  int connections = in->connections( ).size( );
  bool connection_required = in->isRequired( );
  if( connections == 1 ) {
    QNEPort *other_out = in->connections( ).first( )->otherPort( in );
    if( other_out ) {
      GraphicElement *predecessor = other_out->graphicElement( );
      if( predecessor ) {
        int predOutIndex = 0;
        LogicElement *predOutElm;
        if( predecessor->elementType( ) == ElementType::BOX ) {
          Box *box = dynamic_cast< Box* >( predecessor );
          Q_ASSERT( box );
          Q_ASSERT( boxMappings.contains( box ) );
          predOutElm = boxMappings[ box ]->getOutput( other_out->index( ) );
        }
        else {
          predOutElm = map[ predecessor ];
          predOutIndex = other_out->index( );
        }
        currentLogElm->connectPredecessor( inputIndex, predOutElm, predOutIndex );
      }
    }
  }
  else if( ( connections == 0 ) && ( !connection_required ) ) {
    LogicElement *pred = in->defaultValue( ) ? &globalVCC : &globalGND;
    currentLogElm->connectPredecessor( inputIndex, pred, 0 );
  }
}

void ElementMapping::connectElements( ) {
  for( GraphicElement *elm : elements ) {
    for( QNEPort *in : elm->inputs( ) ) {
      applyConnection( elm, in );
    }
  }
}

void ElementMapping::validateElements( ) {
  for( LogicElement *elm : logicElms ) {
    elm->validate( );
  }
}

void ElementMapping::sortLogicElements( ) {
  for( LogicElement *elm : logicElms ) {
    elm->calculatePriority( );
  }
  std::sort( logicElms.begin( ),
             logicElms.end( ),
             [ ]( LogicElement *e1, LogicElement *e2 ) {
    return( *e2 < *e1 );
  } );

}

int ElementMapping::calculatePriority( GraphicElement *elm, QMap< GraphicElement*, bool > &beingvisited,
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
