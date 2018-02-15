#ifndef BOXMAPPING_H
#define BOXMAPPING_H

#include "elementmapping.h"
#include "graphicelement.h"

class BoxMapping : public ElementMapping {
  ElementVector boxInputs;
  ElementVector boxOutputs;

  QVector< LogicElement* > inputs;
  QVector< LogicElement* > outputs;
public:
  BoxMapping( const ElementVector &elms, const ElementVector &inputs, const ElementVector &outputs );

  virtual ~BoxMapping( );

  void initialize( );

  void clearConnections( );

  LogicElement* getInput( int index );
  LogicElement* getOutput( int index );
};

#endif // BOXMAPPING_H
