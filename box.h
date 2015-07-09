#ifndef BOX_H
#define BOX_H

#include "elementfactory.h"
#include "graphicelement.h"
#include "scene.h"
#include "simulationcontroller.h"

class Box : public GraphicElement {

public:
  Box(ElementFactory * factory, QGraphicsItem * parent = 0);
  ~Box();
  // GraphicElement interface
  ElementType elementType();
  void save(QDataStream & ds);
  void load(QDataStream & ds, QMap<quint64, QNEPort *> & portMap, double version);
  void updateLogic();
  void loadFile(QString fname);

private:
  ElementFactory * factory;
  QString m_file;
  QVector <QNEPort *> inputMap;
  QVector <QNEPort *> outputMap;
  Scene myScene;
  SimulationController simulationController;

  void sortMap( QVector< QNEPort * > & map);
};

#endif // BOX_H
