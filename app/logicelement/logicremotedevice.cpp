#include "logicremotedevice.h"
#include "qneport.h"

LogicRemoteDevice::LogicRemoteDevice( RemoteDevice* remoteDevice ) :
  LogicElement( remoteDevice->inputSize(), remoteDevice->outputSize() ), lastClk( false ), elm(remoteDevice) {
  //setOutputValue( 0, false );
}

void LogicRemoteDevice::_updateLogic( const std::vector< bool > &inputs ) {

  for (size_t i = 0; i < inputs.size(); i++) {
      std::map<uint32_t, bool>::const_iterator it = elm->getInputs().find(elm->input(i)->getRemoteId());
      if (it != elm->getInputs().end()) {
        elm->setInput(it->first, inputs[i]);
      }
  }

  for (size_t i = 0; i < getOutputAmount(); i++) {
    std::map<uint32_t, bool>::const_iterator it = elm->getOutputs().find(elm->output(i)->getRemoteId());
    if (it != elm->getOutputs().end()) {
      setOutputValue( i, it->second );
    }
  }
}
