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
class Box;


class ElementMapping;
class BoxMapping;
typedef QMap< GraphicElement*, LogicElement* > ElementMap;
typedef QMap< Input*, LogicElement* > InputMap;

class ElementMapping {
public:
  QString currentFile;
  ElementMap map;
  InputMap inputMap;
  QVector< Clock* > clocks;
  QVector< GraphicElement* > elements;
  QMap< Box*, BoxMapping* > boxMappings;
  QVector< LogicElement* > logicElms;

  LogicInput globalGND;
  LogicInput globalVCC;

  ElementMapping( QString file, const QVector< GraphicElement* > &elms );
  virtual ~ElementMapping( );

  void clear( );

  static QVector< GraphicElement* > sortGraphicElements( QVector< GraphicElement* > elms );

  virtual void initialize( );

  void sort( );

protected:
  QVector< LogicElement* > deletableElements;

  LogicElement* buildLogicElement( GraphicElement *elm );

  void setDefaultValue( GraphicElement *elm, QNEPort *in );
  void applyConnection( GraphicElement *elm, QNEPort *in );
  void generateMap( );
  void connectElements( );
  void validateElements( );
  void sortLogicElements( );
  static int calculatePriority( GraphicElement *elm,
                                QMap< GraphicElement*, bool > &beingvisited,
                                QMap< GraphicElement*, int > &priority );
  void insertElement( GraphicElement *elm );
  void insertBox( Box *box );
};

#endif // ELEMENTMAPPING_H
