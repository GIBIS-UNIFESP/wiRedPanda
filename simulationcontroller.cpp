#include "simulationcontroller.h"

#include <QDebug>
#include <limits>

#include <element/clock.h>

#include <nodes/qneconnection.h>

#include <priorityqueue.h>

SimulationController::SimulationController(QGraphicsScene * scn) : QObject(dynamic_cast<QObject *>(scn)), timer(this) {
  scene = scn;
  timer.start(500);
  connect(&timer,&QTimer::timeout,this,&SimulationController::update);
}

SimulationController::~SimulationController() {

}

int SimulationController::calculatePriority(GraphicElement * elm){
  if(elm->beingVisited()){
    elm->setChanged(true);
    return 0;
  }
  if(elm->visited()){
    return elm->priority();
  }
  int max = 0;
  foreach (QNEPort * port, elm->outputs()) {
    foreach (QNEConnection * conn, port->connections()) {
      QNEPort * sucessor = conn->port1();
      if(sucessor == port){
        sucessor = conn->port2();
      }
      max = qMax(calculatePriority(sucessor->graphicElement()),max);
    }
  }
  elm->setPriority(max + 1);
  elm->setBeingVisited(false);
  elm->setVisited(true);
  return( elm->priority() );
}

void SimulationController::update() {
  QList<QGraphicsItem*> items = scene->items();
  QVector<GraphicElement *> elements;
  foreach (QGraphicsItem * item, items) {
    GraphicElement * elm = qgraphicsitem_cast<GraphicElement *>(item);
    if(elm){
      elements.append(elm);
    }
  }
  QVector<GraphicElement*> changed;
  foreach (GraphicElement * elm, elements) {
    if(elm->changed()){
      elm->setChanged(false);
      elm->setBeingVisited(false);
      elm->setVisited(false);
      elm->setPriority(-1);
      changed.append(elm);
    }
  }
  qDebug() << changed.size();

  if(changed.isEmpty()){
    return;
  }

  foreach (GraphicElement * elm, changed) {
    calculatePriority(elm);
  }

  PriorityQueue queue (elements);
  while (! queue.isEmpty()) {
    GraphicElement * elm = queue.pop();
    elm->updateLogic();
  }
}



