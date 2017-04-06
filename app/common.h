#include <iostream>

#ifndef COMMON_H
#define COMMON_H

#ifdef DEBUG
#define COMMENT( exp, num )     \
  if( DEBUG > num ) {						\
  std::cerr << __FILE__ << ": " << __LINE__ << ": " << __FUNCTION__ << " => " << exp << std::endl; }
#else
#define COMMENT( exp, num )
#endif

#define ERRORMSG( exp )						\
  std::string( __FILE__ ) + ": " + std::to_string( __LINE__ ) + ": " + std::string( __FUNCTION__ ) + ": Error: " + exp


#endif // COMMON_H

