#include "boxmapping.h"

BoxMapping::BoxMapping( const ElementVector &elms, const QNEPortVector &inputs, const QNEPortVector &outputs ) :
  ElementMapping( elms ),
  boxInputs( inputs ),
  boxOutputs( outputs ) {

}

BoxMapping::~BoxMapping( ) {

}

void BoxMapping::initialize( ) {
  ElementMapping::initialize( );
  for( QNEPort *port : boxInputs ) {
    inputs.append( map[ port->graphicElement( ) ] );
  }
  for( QNEPort *port : boxOutputs ) {
    outputs.append( map[ port->graphicElement( ) ] );
  }
}

void BoxMapping::clearConnections( ) {
  for( LogicElement *in : inputs ) {
    in->clearPredecessors( );
  }
  for( LogicElement *out : outputs ) {
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
