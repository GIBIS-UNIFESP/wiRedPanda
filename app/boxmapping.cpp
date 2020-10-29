#include "boxmapping.h"

BoxMapping::BoxMapping( QString file,
                        const ElementVector &elms,
                        const QNEPortVector &inputs,
                        const QNEPortVector &outputs ) :
  ElementMapping( elms, file ),
  boxInputs( inputs ),
  boxOutputs( outputs ) {

}

BoxMapping::~BoxMapping( ) {

}

void BoxMapping::initialize( ) {
  ElementMapping::initialize( );
  for( QNEPort *port : qAsConst( boxInputs ) ) {
    inputs.append( map[ port->graphicElement( ) ] );
  }
  for( QNEPort *port : qAsConst( boxOutputs ) ) {
    outputs.append( map[ port->graphicElement( ) ] );
  }
}

void BoxMapping::clearConnections( ) {
  for( LogicElement *in : qAsConst( inputs ) ) {
    in->clearPredecessors( );
  }
  for( LogicElement *out : qAsConst( outputs ) ) {
    out->clearSucessors( );
  }
}

LogicElement* BoxMapping::getInput( int index ) {
  Q_ASSERT( index < boxInputs.size( ) );
  return( inputs[ index ] );
}

LogicElement* BoxMapping::getOutput( int index ) {
  Q_ASSERT( index < boxOutputs.size( ) );
  return( outputs[ index ] );
}