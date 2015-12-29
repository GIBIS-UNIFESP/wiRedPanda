#include "boxnotfoundexception.h"
#include <string>
BoxNotFoundException::BoxNotFoundException( std::string message, Box *box ) : std::runtime_error( message ) {
  m_box = box;
}

Box* BoxNotFoundException::getBox( ) const {
  return( m_box );
}
