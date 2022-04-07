#pragma once

#include "logicelement.h"
#include "remotedevice.h"

class LogicRemoteDevice : public LogicElement
{
public:
    explicit LogicRemoteDevice(RemoteDevice *RemoteDevice);

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;

private:
    //    bool m_lastClk = false;
    //    bool m_lastValue;
    RemoteDevice *m_elm;
};
