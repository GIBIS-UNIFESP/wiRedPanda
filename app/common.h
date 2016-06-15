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

#endif // COMMON_H

