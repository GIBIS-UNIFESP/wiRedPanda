#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H

#include "elementmapping.h"
#include "scene.h"


class Clock;

class SimulationController : public QObject {
  Q_OBJECT
public:
  explicit SimulationController( Scene *scn );
  ~SimulationController( );

  void updateScene( const QRectF &rect );
  static QVector< GraphicElement* > sortElements( QVector< GraphicElement* > elms );

  bool isRunning( );
signals:

public slots:
  void update( );
  void stop( );
  void start( );
  void reSortElms( );
  void clear( );
private:
  void updateGraphicElement( QNEPort *port );
  void updateConnection( QNEConnection *conn );

  ElementMapping *elMapping;
  Scene *scene;
  QTimer timer;
};

#endif /* SIMULATIONCONTROLLER_H */
