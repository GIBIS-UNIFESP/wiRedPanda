#include "logicremotedevice.h"
#include "qneport.h"

LogicRemoteDevice::LogicRemoteDevice(RemoteDevice *remoteDevice) :
    LogicElement(remoteDevice->inputSize(), remoteDevice->outputSize()), lastClk(false), elm(remoteDevice) {
    //setOutputValue(0, false);
}

void LogicRemoteDevice::_updateLogic(const std::vector<bool> &inputs) {

    for (size_t i = 0; i < inputs.size(); i++) {
        auto it = elm->getInputs().find(elm->input(i)->getRemoteId());
        if (it != elm->getInputs().end()) {
            elm->setInput(it->first, inputs[i]);
        }
    }

    for (size_t i = 0; i < getOutputAmount(); i++) {
        auto it = elm->getOutputs().find(elm->output(i)->getRemoteId());
        if (it != elm->getOutputs().end()) {
            setOutputValue(i, it->second);
        }
    }
}
