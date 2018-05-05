#include <stdexcept>
#ifndef BOXNOTFOUNDEXCEPTION_H
#define BOXNOTFOUNDEXCEPTION_H

class Box;
class BoxNotFoundException : public std::runtime_error {
  Box *m_box;
public:
  BoxNotFoundException( std::string message, Box *box );
  Box* getBox( ) const;
};

#endif /* BOXNOTFOUNDEXCEPTION_H */
