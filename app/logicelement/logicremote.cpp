#include "logicremote.h"
#include "qneport.h"

LogicRemote::LogicRemote(GraphicElement *remoteDevice) : LogicElement(remoteDevice->inputSize(), remoteDevice->outputSize()), m_remote(dynamic_cast<Remote *>(remoteDevice)) {
    // setOutputValue(0, false);
}

void LogicRemote::updateLogic() {
    for (int i = 0; i < m_inputValues.size(); ++i) {
        auto it = m_remote->inputs().find(m_remote->inputPort(i)->remoteId());
        if (it != m_remote->inputs().end()) {
            m_remote->setInput(it.key(), m_inputValues.at(i));
        }
    }

    for (int i = 0; i < m_outputValues.size(); ++i) {
        auto it = m_remote->outputs().find(m_remote->outputPort(i)->remoteId());
        if (it != m_remote->outputs().end()) {
            setOutputValue(i, it.value());
        }
    }
}
