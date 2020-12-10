#include "scstop.h"
#include "simulationcontroller.h"
#include "input.h"
#include <cmath>

SCStop::SCStop( SimulationController *sc ) : sc( sc ) {
  if( sc->isRunning( ) ) {
    restart = true;
    sc->stop( );
  }
}

void SCStop::release( ) {
  if( restart ) {
    sc->start( );
  }
}
SCStop::~SCStop( ) {
  release( );
}

