#include "icnotfoundexception.h"

#include <string>

ICNotFoundException::ICNotFoundException( const std::string &message, IC *ic ) : std::runtime_error( message ) {
  m_ic = ic;
}

IC* ICNotFoundException::getIC( ) const {
  return( m_ic );
}
