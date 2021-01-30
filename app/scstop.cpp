#include "scstop.h"
#include "input.h"
#include "simulationcontroller.h"
#include <cmath>

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
