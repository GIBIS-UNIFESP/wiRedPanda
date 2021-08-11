#ifndef LOGICRemoteDevice_H
#define LOGICRemoteDevice_H

#include "logicelement.h"
#include "remotedevice.h"

class LogicRemoteDevice : public LogicElement {
public:
  explicit LogicRemoteDevice( RemoteDevice* RemoteDevice );

  /* LogicElement interface */
protected:
  virtual void _updateLogic( const std::vector< bool > &inputs );

private:
  bool lastClk;
  bool lastValue;
  RemoteDevice* elm;
};


#endif // LOGICRemoteDevice_H
