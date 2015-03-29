#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include "graphicelement.h"

#include <QList>

class PriorityQueue
{
  public:
    PriorityQueue(QVector<GraphicElement * > elements);
    ~PriorityQueue();
    void push(GraphicElement * elm);

    QList <GraphicElement * >list;
    GraphicElement *pop();
    int size();
    bool isEmpty();
};

#endif // PRIORITYQUEUE_H
