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

LogicElement::LogicElement(int inputSize, int outputSize ) :
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

void LogicElement::setOutputValue(int index, bool value ) {
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

bool LogicElement::getOutputValue(int index ) const {
  return( m_outputs.at( index ) );
}

bool LogicElement::getInputValue(int index ) const {
  Q_ASSERT( m_isValid );
  LogicElement *pred = m_inputs[ index ].first;
  Q_ASSERT( pred );
  int port = m_inputs[ index ].second;
  return( pred->getOutputValue( port ) );
}
