#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H

#include "graphicelement.h"
#include "logicelement.h"
#include "scene.h"

#include <QGraphicsScene>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <input.h>

class SimulationController : public QObject {
  Q_OBJECT
public:
  explicit SimulationController( Scene *scn );
  ~SimulationController( );
  static QVector< GraphicElement* > sortElements( QVector< GraphicElement* > elms );

  void updateScene( const QRectF &rect );
signals:

public slots:
  void update( );
  void stop( );
  void start( );
  void reSortElms( );
  void clear( );

private:
  Scene *scene;
  QTimer timer;

  QMap< GraphicElement*, LogicElement* > map;
  QMap< Input*, LogicElement* > inputMap;
  QVector< LogicElement* > logicElms;
  static int calculatePriority( GraphicElement *elm,
                                QMap< GraphicElement*, bool > &beingvisited,
                                QMap< GraphicElement*, int > &priority );
};

#endif /* SIMULATIONCONTROLLER_H */
