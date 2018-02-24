#include "common.h"

#ifdef DEBUG
int Comment::verbosity = DEBUG;
#else
int Comment::verbosity = -1;
#endif
