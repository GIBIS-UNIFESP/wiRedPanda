#ifndef SCSTOP_H
#define SCSTOP_H

class SimulationController;

class SCStop
{
    SimulationController *sc;
    bool restart = false;

public:
    SCStop(SimulationController *sc);
    void release();
    ~SCStop();
};

#endif /* SCSTOP_H */
