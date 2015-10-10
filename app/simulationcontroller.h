#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H

#include "graphicelement.h"
#include "scene.h"

#include <QGraphicsScene>
#include <QObject>
#include <QTimer>

class SimulationController : public QObject {
  Q_OBJECT
public:
  explicit SimulationController(Scene * scn);
    ~SimulationController();
  signals:

public slots:
  void update();
  void stop();
  void start();


private:
  Scene * scene;
  QTimer timer;

  int calculatePriority(GraphicElement * elm);
};

#endif // SIMULATIONCONTROLLER_H
