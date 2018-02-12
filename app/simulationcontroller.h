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

class Clock;

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

  QMap< GraphicElement*, LogicElement* > m_map;
  QMap< Input*, LogicElement* > m_inputMap;
  QVector< Clock* > m_clocks;
  QVector< LogicElement* > m_logicElms;
  static int calculatePriority( GraphicElement *elm,
                                QMap< GraphicElement*, bool > &beingvisited,
                                QMap< GraphicElement*, int > &priority );
};

#endif /* SIMULATIONCONTROLLER_H */
