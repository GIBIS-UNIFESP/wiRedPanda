#ifndef ELEMENTMAPPING_H
#define ELEMENTMAPPING_H

#include "graphicelement.h"
#include "logicelement.h"

#include <QGraphicsScene>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <input.h>

class Clock;

typedef QMap< GraphicElement*, LogicElement* > ElementMap;
typedef QMap< Input*, LogicElement* > InputMap;

class ElementMapping {
public:

  ElementMap map;
  InputMap inputMap;
  QVector< Clock* > clocks;
  QVector< LogicElement* > logicElms;
  QVector< GraphicElement* > elements;

  LogicInput *m_globalVCC;
  LogicInput *m_globalGND;

  ElementMapping( );

  void clear( );

  void resortElements( const QVector< GraphicElement* > &elements );

  static QVector< GraphicElement* > sortElements( QVector< GraphicElement* > elms );

private:


  LogicElement* buildLogicElement( GraphicElement *elm );

  void initialize( );
  void setDefaultValue( GraphicElement *elm, QNEPort *in );
  void applyConnection( GraphicElement *elm, QNEPort *in );
  void generateMap( );
  void generateConnections( );
  void validateElements( );
  void sortLogicElements( );
  static int calculatePriority( GraphicElement *elm,
                                QMap< GraphicElement*, bool > &beingvisited,
                                QMap< GraphicElement*, int > &priority );
};

#endif // ELEMENTMAPPING_H
