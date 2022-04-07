#include "logicremotedevice.h"
#include "qneport.h"

LogicRemoteDevice::LogicRemoteDevice(RemoteDevice *remoteDevice) : LogicElement(remoteDevice->inputSize(), remoteDevice->outputSize()), m_elm(remoteDevice) {
    // setOutputValue(0, false);
}

void LogicRemoteDevice::_updateLogic(const std::vector<bool> &inputs) {
    for (size_t i = 0; i < inputs.size(); i++) {
        auto it = m_elm->getInputs().find(m_elm->input(i)->getRemoteId());
        if (it != m_elm->getInputs().end()) {
            m_elm->setInput(it.key(), inputs[i]);
        }
    }

    for (size_t i = 0; i < getOutputAmount(); i++) {
        auto it = m_elm->getOutputs().find(m_elm->output(i)->getRemoteId());
        if (it != m_elm->getOutputs().end()) {
            setOutputValue(i, it.value());
        }
    }
}
