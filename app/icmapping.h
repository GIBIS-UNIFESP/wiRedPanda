#ifndef ICMAPPING_H
#define ICMAPPING_H

#include "elementmapping.h"
#include "graphicelement.h"

class LogicElement;

class ICMapping : public ElementMapping
{
    QNEPortVector icInputs;
    QNEPortVector icOutputs;

    QVector<LogicElement *> inputs;
    QVector<LogicElement *> outputs;

public:
    ICMapping(QString file, const ElementVector &elms, const QNEPortVector &inputs, const QNEPortVector &outputs);

    ~ICMapping() override;

    void initialize() override;

    void clearConnections();

    LogicElement *getInput(int index);
    LogicElement *getOutput(int index);
};

#endif // ICMAPPING_H
