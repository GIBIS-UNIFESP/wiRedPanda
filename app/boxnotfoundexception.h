#ifndef BOXNOTFOUNDEXCEPTION_H
#define BOXNOTFOUNDEXCEPTION_H

#include <stdexcept>

class Box;
class BoxNotFoundException : public std::runtime_error {
  Box *m_box;
public:
  BoxNotFoundException( std::string message, Box *box );
  Box* getBox( ) const;
};

#endif /* BOXNOTFOUNDEXCEPTION_H */
