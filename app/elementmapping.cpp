#include "elementmapping.h"

#include <QDebug>
#include <clock.h>
#include <qneconnection.h>



ElementMapping::ElementMapping( ) : m_globalGND( nullptr ), m_globalVCC( nullptr ) {
}

void ElementMapping::clear( ) {
  for( LogicElement *elm: logicElms ) {
    delete elm;
  }
  if( m_globalVCC ) {
    delete m_globalVCC;
    m_globalVCC = nullptr;
  }
  if( m_globalGND ) {
    delete m_globalGND;
    m_globalGND = nullptr;
  }
  delete m_globalGND;
  logicElms.clear( );
  map.clear( );
  inputMap.clear( );
  clocks.clear( );
}

void ElementMapping::resortElements( const QVector< GraphicElement* > &elms ) {
  initialize( );
  elements = elms;
  generateMap( );
  generateConnections( );
  sortLogicElements( );
  validateElements( );
}

QVector< GraphicElement* > ElementMapping::sortElements( QVector< GraphicElement* > elms ) {
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

void ElementMapping::generateMap( ) {
  for( GraphicElement *elm: elements ) {
    if( elm->elementType( ) == ElementType::CLOCK ) {
      Clock *clk = dynamic_cast< Clock* >( elm );
      Q_ASSERT( clk != nullptr );
      clocks.append( clk );
    }
    if( elm->elementType( ) == ElementType::BOX ) {
      qDebug( ) << "Cannot handle boxes yet";
      // FIXME Cannot handle boxes yet
    }
    else {
      LogicElement *logicElm = buildLogicElement( elm );
      logicElms.append( logicElm );
      map.insert( elm, logicElm );
      Input *in = dynamic_cast< Input* >( elm );
      if( in ) {
        inputMap[ in ] = logicElm;
      }
    }
  }
}

LogicElement* ElementMapping::buildLogicElement( GraphicElement *elm ) {
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
      break;
  }
  return( nullptr );
}

void ElementMapping::initialize( ) {
  clear( );
  if( m_globalGND == nullptr ) {
    m_globalGND = new LogicInput( false );
  }
  if( m_globalVCC == nullptr ) {
    m_globalVCC = new LogicInput( true );
  }
}





void ElementMapping::setDefaultValue( GraphicElement *elm, QNEPort *in ) {
  if( in->defaultValue( ) == false ) {
    map[ elm ]->connectInput( in->index( ), m_globalGND, 0 );
  }
  else {
    map[ elm ]->connectInput( in->index( ), m_globalVCC, 0 );
  }
}

void ElementMapping::applyConnection( GraphicElement *elm, QNEPort *in ) {
  QNEPort *other_out = in->connections( ).first( )->otherPort( in );
  if( other_out ) {
    GraphicElement *other_elm = other_out->graphicElement( );
    if( other_elm ) {
      map[ elm ]->connectInput( in->index( ), map[ other_elm ], other_out->index( ) );
    }
  }
}

void ElementMapping::generateConnections( ) {
  for( GraphicElement *elm : elements ) {
    if( elm->elementType( ) == ElementType::BOX ) {
      qDebug( ) << "Cannot handle box element yet";
      //FIXME Cannot handle box yet
      continue;
    }
    for( QNEPort *in : elm->inputs( ) ) {
      if( in->connections( ).size( ) == 1 ) {
        applyConnection( elm, in );
      }
      else if( ( in->connections( ).size( ) == 0 ) && !in->isRequired( ) ) {
        setDefaultValue( elm, in );
      }
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
  std::sort( logicElms.begin( ), logicElms.end( ) );
}

int ElementMapping::calculatePriority( GraphicElement *elm,
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
