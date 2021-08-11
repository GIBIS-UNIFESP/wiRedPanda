#ifndef REMOTEDEVICE_H
#define REMOTEDEVICE_H

#include <QtXml>
#include <QFile>
#include <QTcpSocket>

#include <list>
#include <map>
#include "graphicelement.h"

enum AUTH_METHOD {
  NONE = 0,
  PLAIN = 1,
  MD5,
  SHA1,
  SHA256
};

AUTH_METHOD toAuthMethod(QString auth_type);

struct RemoteLabOption {
  std::string name;
  std::string url;
  std::string version;
  AUTH_METHOD authMethod;

  RemoteLabOption() = default;
  RemoteLabOption(std::string name, std::string url, AUTH_METHOD auth_method) : name(name), url(url), version("v9999.9999.9999"), authMethod(auth_method) {}

  std::string getName() const { return name; }
  std::string getUrl() const {
      QString url = QString::fromStdString(this->url);

      if (!url.endsWith("/"))
          url.append("/");

      return url.toStdString();
  }

  void setVersion(const std::string& version) { this->version = version; }
  std::string getVersion() const { return version; }
  AUTH_METHOD getAuthMethod() const { return authMethod; }
};

enum PIN_TYPE : uint8_t {
  PIN_NONE = 0,
  PIN_INPUT = 1,
  PIN_OUTPUT = 2,
  PIN_GENERAL_PURPOSE = 3,
};

struct Pin {
  uint32_t id;
  std::string name;
  PIN_TYPE type;

  Pin() = delete;
  Pin(uint32_t id, const std::string& name, PIN_TYPE type) : id(id), name(name), type(type) {}

  uint32_t getId() const { return id; }
  const std::string& getName() const { return name; }
  PIN_TYPE getType() const { return type; }

  static PIN_TYPE convertTypeString(const std::string& typeName);
};

struct DeviceAuth {
    std::string name;
    std::string token;
};

class RemoteDevice : public GraphicElement {
  bool lastClk;
  bool lastValue;
  bool connected;

  uint8_t deviceTypeId;
  uint8_t methodId;
  uint16_t deviceId;
  uint16_t latency;
  uint32_t minWaitTime;
  uint64_t afterTimeStartedEpoch;
  uint64_t aliveSince;
  uint64_t allowUntil;
  uint64_t startedTimeEpoch;
  long timeAllowed;
  std::string authToken;
  std::string deviceMethod;
  DeviceAuth deviceAuth;

  // Queue
  bool inQueue;
  uint8_t queuePos;
  uint32_t deviceAllowedTime;
  uint64_t queueWaitingSinceEpoch;
  uint64_t queueEstimatedEpoch;

  static std::list<RemoteLabOption> options;
  std::list<Pin> availablePins;
  std::list<Pin> mappedPins;

  std::map<uint32_t, bool> mappedInputs;
  std::map<uint32_t, bool> mappedOutputs;

  RemoteLabOption currentOption;
  QTcpSocket socket;
  QTimer timer;

public:
  explicit RemoteDevice( QGraphicsItem *parent = nullptr );
  virtual ~RemoteDevice( ) override;

  void setupPorts( );

  bool connectTo(const std::string& host, int port, const std::string& token, uint8_t deviceTypeId, uint8_t methodId);
  void sendPing();
  void sendIOInfo();
  void sendUpdateInput(uint32_t id, uint8_t value);
  void sendRequestToEnterQueue(const QString& token);

  void onTimeRefresh();

  void disconnect() {
      if (socket.isOpen()) {
          socket.disconnectFromHost();
          socket.waitForDisconnected(1000);
          socket.close();
      }

      // will no longer trigger the setup window
      setAuthToken("");

      // reset available pins, the mapped one shall still be configured (in case of incompatibility, they will not be applied)
      availablePins.clear();

      // reset time variables
      minWaitTime = 0;
      aliveSince = 0;
      allowUntil = 0;
      afterTimeStartedEpoch = 0;
      startedTimeEpoch = 0;

      inQueue = false;
      queuePos = 0;
      deviceAllowedTime = 0;
      queueWaitingSinceEpoch = 0;
      queueEstimatedEpoch = 0;

      connected = false;

      update();
  }

  const std::string& getAuthToken() const { return authToken; }
  void setAuthToken(const std::string& token) {
      authToken = token;
  }

  const std::string& getDeviceMethod() const { return deviceMethod; }
  void setDeviceMethod(const std::string& method) {
      deviceMethod = method;
  }

  const DeviceAuth& getDeviceAuth() const { return deviceAuth; }
  void setDeviceAuth(const std::string& name, const std::string& token) {
      deviceAuth = {name, token};
  }

  uint16_t getDeviceMethodId() const { return methodId; }
  uint16_t getDeviceTypeId() const { return deviceTypeId; }
  uint16_t getDeviceId() const { return deviceId; }
  void setDeviceId(uint16_t id) {
      deviceId = id;
  }

  uint16_t getLatency() const { return latency; }
  void setLatency(uint16_t milliseconds) {
      latency = milliseconds;
  }

  bool isAlive() const { return (static_cast<uint64_t>(QDateTime::currentSecsSinceEpoch()) < (getAliveSince() + 10)); }
  uint64_t getAliveSince() const { return aliveSince; }
  void setAliveSince(uint64_t epochInSeconds) {
      aliveSince = epochInSeconds;
  }

  uint64_t getTotalAllowUntil() const { return allowUntil + static_cast<uint64_t>(getMinWaitTime()); }
  uint64_t getAllowUntil() const { return allowUntil; }
  void setAllowUntil(uint64_t epochInSeconds) {
      allowUntil = epochInSeconds;
  }

  uint32_t getMinWaitTime() const { return minWaitTime; }
  void setMinWaitTime(uint32_t minWaitTimeInSeconds) {
      minWaitTime = minWaitTimeInSeconds;
  }

  long getTimeAllowed() const { return timeAllowed; }
  uint64_t getStartedTimeEpoch() const { return startedTimeEpoch; }
  void initTimeCount() {
      startedTimeEpoch = QDateTime::currentSecsSinceEpoch();
      timeAllowed = static_cast<long>(getTotalAllowUntil() - startedTimeEpoch);

      if (timeAllowed < 0)
          timeAllowed = 0;
  }

  uint64_t getAfterTimeStartedEpoch() const { return afterTimeStartedEpoch; }
  bool hasAfterTimeStarted() const { return afterTimeStartedEpoch != 0; }
  long getAvailableAfterTime() const {
      if (!hasAfterTimeStarted())
          return minWaitTime;

      uint64_t currentEpoch = QDateTime::currentSecsSinceEpoch();
      long passedTime = static_cast<long>(currentEpoch - afterTimeStartedEpoch);

      if (passedTime < 0)
          return minWaitTime;

      return static_cast<long>(minWaitTime) - passedTime;
  }
  void startAfterTime(uint64_t epochInSeconds) { afterTimeStartedEpoch = epochInSeconds; }

  const RemoteLabOption getCurrentOption() const { return currentOption; }
  void setCurrentOption(const RemoteLabOption option) {
      currentOption = option;
  }

  const std::list<Pin>& getAvailablePins() const { return availablePins; }
  void addPin(uint32_t id, const std::string& port, uint8_t pinType) {
      availablePins.push_back(Pin(id, port, static_cast<PIN_TYPE>(pinType)));
  }

  void resetPortMapping() { mappedPins.clear(); mappedOutputs.clear(); }
  const std::list<Pin>& getMappedPins() const { return mappedPins; }

  uint32_t getPinId(const std::string& name, uint8_t pinType) {
      const std::list<Pin>& list = getAvailablePins();
      std::list<Pin>::const_iterator it;

      for (it = list.begin(); it != list.end() ; ++it) {
          const Pin& p = (*it);

          if (p.getName().compare(name) == 0) {
              if (static_cast<int>(p.getType()) == PIN_GENERAL_PURPOSE || static_cast<int>(p.getType()) == static_cast<int>(pinType)) {

                  std::list<Pin>::const_iterator mapped_it = getMappedPins().begin();

                  while (mapped_it != getMappedPins().end()) {
                      if (mapped_it->getName().compare(name) == 0)
                          return 0;

                      ++mapped_it;
                  }

                  return p.id;
              } else {
                  return 0;
              }
          }
      }

      return 0;
  }

  bool canBeMapped(const std::string& name, uint8_t pinType) {
      return (getPinId(name, pinType) != 0);
  }

  bool mapPin(const std::string& name, uint8_t pinType) {
      uint32_t id = getPinId(name, pinType);

      if (id != 0) {
          //std::cerr << "Pin(" << id << ", " << name << ", " << static_cast<PIN_TYPE>(pinType) << ")" << std::endl;
          mappedPins.push_back(Pin(id, name, static_cast<PIN_TYPE>(pinType)));
          if (pinType == PIN_TYPE::PIN_INPUT) { mappedInputs[id] = false; }
          if (pinType == PIN_TYPE::PIN_OUTPUT) { mappedOutputs[id] = false; }
          return true;
      }

      return false;
  }

  const std::map<uint32_t, bool>& getInputs() { return mappedInputs; }
  void setInput(uint32_t id, bool value) {
      if (mappedInputs[id] != value) {
          mappedInputs[id] = value;
          sendUpdateInput(id, value == true ? 1 : 0);
      }
  }

  const std::map<uint32_t, bool>& getOutputs() { return mappedOutputs; }
  void setOutput(uint32_t id, bool value) { mappedOutputs[id] = value; }

  void loadAvailablePin( QDataStream &ds );
  void loadMappedPin( QDataStream &ds );
  void loadRemoteIO( QDataStream &ds, double version );

  // Queue
  bool isInQueue() const { return inQueue; }
  void setIsInQueue(bool boolean) {
    inQueue = boolean;
    queueWaitingSinceEpoch = QDateTime::currentSecsSinceEpoch();
  }
  uint8_t getQueuePos() const { return queuePos; }
  void setQueuePos(uint8_t pos) { queuePos = pos; }
  uint64_t getQueueEstimatedEpoch() const { return queueEstimatedEpoch; }
  void setQueueEstimatedEpoch(uint64_t epoch) { queueEstimatedEpoch = epoch; }
  uint64_t getWaitingSince() const { return queueWaitingSinceEpoch; }
  uint32_t getDeviceAllowedTime() const { return deviceAllowedTime; }
  void setDeviceAllowedTime(uint32_t timeInSeconds) { deviceAllowedTime = timeInSeconds; }

  void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget ) override;
  bool isConnected() const { return connected; }

private slots:
    void readIsDone();
    void close();

  // GraphicElement interface
public:
  virtual void updatePorts( ) override;
  void setSkin( __attribute__((unused)) bool defaultSkin, __attribute__((unused)) const QString& filename ) override { };
  static bool loadSettings( const QDomDocument& xml );
  const std::list<RemoteLabOption>& getOptions() { return options; }

  void load( QDataStream &ds, QMap< quint64, QNEPort* > &portMap, double version ) override;
  void save( QDataStream &ds ) const override;
};

#endif
