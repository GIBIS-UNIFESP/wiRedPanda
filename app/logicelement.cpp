#include "logicelement.h"

#include <QDebug>

bool LogicElement::isValid( ) const {
  return( m_isValid );
}

void LogicElement::clearPredecessors( ) {
  for( auto input: m_inputs ) {
    input.first = nullptr;
    input.second = 0;
  }
}

void LogicElement::clearSucessors( ) {
  for( LogicElement *elm : m_sucessors ) {
    for( auto input: elm->m_inputs ) {
      if( input.first == this ) {
        input.first = nullptr;
        input.second = 0;
      }
    }
  }
  m_sucessors.clear( );
}

LogicElement::LogicElement( size_t inputSize, size_t outputSize ) :
  m_isValid( true ),
  beingVisited( false ),
  priority( -1 ),
  m_inputs( inputSize, std::make_pair( nullptr, 0 ) ),
  m_inputvalues( inputSize, false ),
  m_outputs( outputSize, false ) {

}

LogicElement::~LogicElement( ) {

}

void LogicElement::updateLogic( ) {
  if( m_isValid ) {
    for( size_t idx = 0; idx < m_inputs.size( ); ++idx ) {
      m_inputvalues[ idx ] = getInputValue( idx );
    }
    _updateLogic( m_inputvalues );
  }
}

void LogicElement::connectPredecessor( int index, LogicElement *elm, int port ) {
  m_inputs.at( index ) = std::make_pair( elm, port );
  elm->m_sucessors.insert( this );
}

void LogicElement::setOutputValue( size_t index, bool value ) {
  m_outputs.at( index ) = value;
}

void LogicElement::setOutputValue( bool value ) {
  m_outputs.at( 0 ) = value;
}

void LogicElement::validate( ) {
  m_isValid = true;
  for( size_t in = 0; in < m_inputs.size( ) && m_isValid; ++in ) {
    if( m_inputs[ in ].first == nullptr ) {
      m_isValid = false;
    }
  }
  if( !m_isValid ) {
    for( LogicElement *elm : m_sucessors ) {
      elm->m_isValid = false;
    }
  }
}

bool LogicElement::operator<( const LogicElement &other ) {
  return( priority < other.priority );
}

int LogicElement::calculatePriority( ) {
  if( beingVisited ) {
    return( 0 );
  }
  if( priority != -1 ) {
    return( priority );
  }
  beingVisited = true;
  int max = 0;
  for( LogicElement *s : m_sucessors ) {
    max = qMax( s->calculatePriority( ), max );
  }
  int p = max + 1;
  priority = p;
  beingVisited = false;
  return( p );
}

bool LogicElement::getOutputValue( size_t index ) const {
  return( m_outputs.at( index ) );
}

bool LogicElement::getInputValue( size_t index ) const {
  Q_ASSERT( m_isValid );
  LogicElement *pred = m_inputs[ index ].first;
  Q_ASSERT( pred );
  int port = m_inputs[ index ].second;
  return( pred->getOutputValue( port ) );
}

LogicNode::LogicNode( ) : LogicElement( 1, 1 ) {

}

void LogicNode::_updateLogic( const std::vector< bool > &inputs ) {
  setOutputValue( inputs[ 0 ] );
}

//LogicInput::LogicInput( Input *in ) : LogicElement( 0, 1 ), input( in ) {
//  setOutputValue( 0, input->getOn( ) );
//}


LogicOutput::LogicOutput( size_t inputSz ) : LogicElement( inputSz, inputSz ) {

}

void LogicOutput::_updateLogic( const std::vector< bool > &inputs ) {
  for( size_t idx = 0; idx < inputs.size( ); ++idx ) {
    setOutputValue( idx, inputs[ idx ] );
  }
}

LogicAnd::LogicAnd( size_t inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicAnd::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = true;
  for( bool in : inputs ) {
    result &= in;
  }
  setOutputValue( result );
}

LogicOr::LogicOr( size_t inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicOr::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = false;
  for( bool in : inputs ) {
    result |= in;
  }
  setOutputValue( result );
}

LogicNand::LogicNand( size_t inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicNand::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = true;
  for( bool in : inputs ) {
    result &= in;
  }
  setOutputValue( !result );
}

LogicNor::LogicNor( size_t inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicNor::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = false;
  for( bool in : inputs ) {
    result |= in;
  }
  setOutputValue( !result );
}

LogicXor::LogicXor( size_t inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicXor::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = false;
  for( bool in : inputs ) {
    result ^= in;
  }
  setOutputValue( result );
}

LogicXnor::LogicXnor( size_t inputSize )  : LogicElement( inputSize, 1 ) {

}

void LogicXnor::_updateLogic( const std::vector< bool > &inputs ) {
  bool result = false;
  for( bool in : inputs ) {
    result ^= in;
  }
  setOutputValue( !result );
}

LogicNot::LogicNot( ) : LogicElement( 1, 1 ) {

}

void LogicNot::_updateLogic( const std::vector< bool > &inputs ) {
  setOutputValue( !inputs[ 0 ] );
}


LogicInput::LogicInput( bool defaultValue ) : LogicElement( 0, 1 ) {
  setOutputValue( 0, defaultValue );
}

void LogicInput::_updateLogic( const std::vector< bool > & ) {
  // Does nothing on update
}

LogicJKFlipFlop::LogicJKFlipFlop( ) :
  LogicElement( 5, 2 ),
  lastClk( false ) {
  setOutputValue( 0, false );
  setOutputValue( 1, true );
}

void LogicJKFlipFlop::_updateLogic( const std::vector< bool > &inputs ) {
  bool q0 = getOutputValue( 0 );
  bool q1 = getOutputValue( 1 );
  bool j = inputs[ 0 ];
  bool clk = inputs[ 1 ];
  bool k = inputs[ 2 ];
  bool prst = inputs[ 3 ];
  bool clr = inputs[ 4 ];
  if( clk && !lastClk ) {
    if( j && k ) {
      std::swap( q0, q1 );
    }
    else if( j ) {
      q0 = true;
      q1 = false;
    }
    else if( k ) {
      q0 = false;
      q1 = true;
    }
  }
  if( ( !prst ) || ( !clr ) ) {
    q0 = !prst;
    q1 = !clr;
  }
  lastClk = clk;

  setOutputValue( 0, q0 );
  setOutputValue( 1, q1 );
}

LogicSRFlipFlop::LogicSRFlipFlop( ) :
  LogicElement( 5, 2 ),
  lastClk( false ) {
  setOutputValue( 0, false );
  setOutputValue( 1, true );
}

void LogicSRFlipFlop::_updateLogic( const std::vector< bool > &inputs ) {
  bool q0 = getOutputValue( 0 );
  bool q1 = getOutputValue( 1 );
  bool s = inputs[ 0 ];
  bool clk = inputs[ 1 ];
  bool r = inputs[ 2 ];
  bool prst = inputs[ 3 ];
  bool clr = inputs[ 4 ];
  if( clk && !lastClk ) {
    if( s && r ) {
      q0 = true;
      q1 = true;
    }
    else if( s != r ) {
      q0 = s;
      q1 = r;
    }
  }
  if( ( !prst ) || ( !clr ) ) {
    q0 = !prst;
    q1 = !clr;
  }
  lastClk = clk;

  setOutputValue( 0, q0 );
  setOutputValue( 1, q1 );
  /* Reference: https://pt.wikipedia.org/wiki/Flip-flop#Flip-flop_SR_Sincrono */
}

LogicTFlipFlop::LogicTFlipFlop( ) :
  LogicElement( 4, 2 ),
  lastClk( false ) {
  setOutputValue( 0, false );
  setOutputValue( 1, true );
}

void LogicTFlipFlop::_updateLogic( const std::vector< bool > &inputs ) {
  bool q0 = getOutputValue( 0 );
  bool q1 = getOutputValue( 1 );
  bool T = inputs[ 0 ];
  bool clk = inputs[ 1 ];
  bool prst = inputs[ 2 ];
  bool clr = inputs[ 3 ];
  if( clk && !lastClk ) {
    if( T ) {
      q0 = !q0;
      q1 = !q0;
    }
  }
  if( ( !prst ) || ( !clr ) ) {
    q0 = !prst;
    q1 = !clr;
  }
  setOutputValue( 0, q0 );
  setOutputValue( 1, q1 );
  lastClk = clk;
  /* Reference: https://en.wikipedia.org/wiki/Flip-flop_(electronics)#T_flip-flop */
}

LogicDFlipFlop::LogicDFlipFlop( ) :
  LogicElement( 4, 2 ),
  lastClk( false ) {
  setOutputValue( 0, false );
  setOutputValue( 1, true );

}

void LogicDFlipFlop::_updateLogic( const std::vector< bool > &inputs ) {
  bool q0 = getOutputValue( 0 );
  bool q1 = getOutputValue( 1 );
  bool D = inputs[ 0 ];
  bool clk = inputs[ 1 ];
  bool prst = inputs[ 2 ];
  bool clr = inputs[ 3 ];
  if( clk && !lastClk ) {
    q0 = lastValue;
    q1 = !lastValue;
  }
  if( ( !prst ) || ( !clr ) ) {
    q0 = !prst;
    q1 = !clr;
  }
  setOutputValue( 0, q0 );
  setOutputValue( 1, q1 );
  lastClk = clk;
  lastValue = D;
  /* Reference: https://en.wikipedia.org/wiki/Flip-flop_(electronics)#T_flip-flop */
}

LogicDLatch::LogicDLatch( ) :
  LogicElement( 2, 2 ) {
  setOutputValue( 0, false );
  setOutputValue( 1, true );
}

void LogicDLatch::_updateLogic( const std::vector< bool > &inputs ) {
  bool q0 = getOutputValue( 0 );
  bool q1 = getOutputValue( 1 );
  bool D = inputs[ 0 ];
  bool enable = inputs[ 1 ];
  if( enable ) {
    q0 = D;
    q1 = !D;
  }
  setOutputValue( 0, q0 );
  setOutputValue( 1, q1 );
}

LogicMux::LogicMux( ) : LogicElement( 3, 1 ) {

}

void LogicMux::_updateLogic( const std::vector< bool > &inputs ) {
  bool data1 = inputs[ 0 ];
  bool data2 = inputs[ 1 ];
  bool choice = inputs[ 2 ];
  if( choice == false ) {
    setOutputValue( data1 );
  }
  else {
    setOutputValue( data2 );
  }
}

LogicDemux::LogicDemux( ) : LogicElement( 2, 2 ) {

}

void LogicDemux::_updateLogic( const std::vector< bool > &inputs ) {
  bool data = inputs[ 0 ];
  bool choice = inputs[ 1 ];

  bool out0 = false;
  bool out1 = false;
  if( choice == false ) {
    out0 = data;
  }
  else {
    out1 = data;
  }
  setOutputValue( 0, out0 );
  setOutputValue( 1, out1 );
}
