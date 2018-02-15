#include "boxmapping.h"

BoxMapping::BoxMapping( const ElementVector &elms, const ElementVector &inputs, const ElementVector &outputs ) :
  ElementMapping( elms ),
  boxInputs( inputs ),
  boxOutputs( outputs ) {

}

BoxMapping::~BoxMapping( ) {

}

void BoxMapping::initialize( ) {
  ElementMapping::initialize( );
  for( GraphicElement *elm : boxInputs ) {
    inputs.append( map[ elm ] );
  }
  for( GraphicElement *elm : boxOutputs ) {
    outputs.append( map[ elm ] );
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
  return( map[ boxInputs[ index ] ] );
}

LogicElement* BoxMapping::getOutput( int index ) {
  Q_ASSERT( index < boxOutputs.size( ) );
  return( map[ boxOutputs[ index ] ] );
}
