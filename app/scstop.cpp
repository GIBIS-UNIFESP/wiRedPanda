#include "scstop.h"

#include <cmath>

#include "input.h"
#include "simulationcontroller.h"

SCStop::SCStop(SimulationController *sc)
    : sc(sc)
{
    if (sc->isRunning()) {
        restart = true;
        sc->stop();
    }
}

void SCStop::release()
{
    if (restart) {
        sc->start();
    }
}
SCStop::~SCStop()
{
    release();
}
