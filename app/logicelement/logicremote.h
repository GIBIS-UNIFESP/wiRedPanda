#pragma once

#include "logicelement.h"
#include "remote.h"

class LogicRemote : public LogicElement
{
public:
    explicit LogicRemote(GraphicElement *remoteDevice);

    void updateLogic() override;

private:
    Q_DISABLE_COPY(LogicRemote)

    Remote *m_remote;
};
