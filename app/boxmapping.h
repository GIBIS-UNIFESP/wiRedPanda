#ifndef BOXMAPPING_H
#define BOXMAPPING_H

#include "elementmapping.h"
#include "graphicelement.h"

class BoxMapping : public ElementMapping {
  QNEPortVector boxInputs;
  QNEPortVector boxOutputs;

  QVector< LogicElement* > inputs;
  QVector< LogicElement* > outputs;
public:
  BoxMapping( QString file, const ElementVector &elms, const QNEPortVector &inputs, const QNEPortVector &outputs );

  virtual ~BoxMapping( );

  void initialize( );

  void clearConnections( );

  LogicElement* getInput( int index );
  LogicElement* getOutput( int index );
};

#endif // BOXMAPPING_H