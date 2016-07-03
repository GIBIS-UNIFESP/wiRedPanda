#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H

#include "graphicelement.h"
#include "scene.h"

#include <QGraphicsScene>
#include <QMap>
#include <QObject>
#include <QTimer>

class SimulationController : public QObject {
  Q_OBJECT
public:
  explicit SimulationController( Scene *scn );
  ~SimulationController( );
  static QVector< GraphicElement* > sortElements( QVector< GraphicElement* > elms );
signals:

public slots:
  void update( );
  void stop( );
  void start( );
  void reSortElms();

private:
  Scene *scene;
  QTimer timer;
  QVector< GraphicElement* > sortedElements;
  static int calculatePriority( GraphicElement *elm,
                                QMap< GraphicElement*, bool > &beingvisited,
                                QMap< GraphicElement*, int > &priority );
};

#endif /* SIMULATIONCONTROLLER_H */
