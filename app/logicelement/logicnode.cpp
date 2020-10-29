#include "logicnode.h"

LogicNode::LogicNode( ) : LogicElement( 1, 1 ) {

}

void LogicNode::_updateLogic( const std::vector< bool > &inputs ) {
  setOutputValue( inputs[ 0 ] );
}