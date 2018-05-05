#ifndef COMMON_H
#define COMMON_H

#include <iostream>

class Comment {
public:
  static int verbosity;

  static void setVerbosity( int vb ) {
    verbosity = vb;
  }
};

#ifdef DEBUG
#define COMMENT( exp, num )     \
  if( Comment::verbosity > num ) {                                           \
    std::cerr << __FILE__ << ": " << __LINE__ << ": " << __FUNCTION__ << " => " << exp << std::endl; }
#else
#define COMMENT( exp, num )
#endif

#define ERRORMSG( exp )                                         \
  std::string( __FILE__ ) + ": " + std::to_string( __LINE__ ) + ": " + std::string( __FUNCTION__ ) + ": Error: " + exp


#endif // COMMON_H
