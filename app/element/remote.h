#pragma once

#include "graphicelement.h"

#include <QFile>
#include <QTcpSocket>
#include <QtXml>
#include <list>
#include <map>

enum class Auth_Method
{
    None = 0,
    Plain = 1,
    MD5 = 2,
    SHA1 = 3,
    SHA256 = 4,
}; // TODO: register in meta system

struct RemoteLabOption
{
    QString m_name;
    QString m_url;
    QString m_version;
    Auth_Method m_authMethod;

    RemoteLabOption() = default;
    RemoteLabOption(const QString &name, const QString &url, Auth_Method auth_method) : m_name(name), m_url(url), m_version("v9999.9999.9999"), m_authMethod(auth_method) {}

    QString name() const {
        return m_name;
    }

    QString url() const {
        QString url_ = m_url;

        if (!url_.endsWith("/")) {
            url_.append("/");
        }

        return url_;
    }

    void setVersion(const QString &version) {
        m_version = version;
    }

    QString version() const {
        return m_version;
    }

    Auth_Method authMethod() const {
        return m_authMethod;
    }
};

enum class Pin_Type
{
    None = 0,
    Input = 1,
    Output = 2,
    General_Purpose = 3,
}; // TODO: register in meta system

struct Pin
{
    uint32_t m_id;
    QString m_name;
    Pin_Type m_type;

    Pin() = delete;
    Pin(uint32_t id, const QString &name, Pin_Type type) : m_id(id), m_name(name), m_type(type) {}

    uint32_t id() const {
        return m_id;
    }

    const QString &name() const {
        return m_name;
    }

    Pin_Type type() const {
        return m_type;
    }

    static Pin_Type convertTypeString(const QString &typeName);
};

struct DeviceAuth
{
    QString m_name;
    QString m_token;
};

class Remote : public GraphicElement
{
    Q_OBJECT

public:
    explicit Remote(QGraphicsItem *parent = nullptr);
    ~Remote() override;

    void setupPorts();

    bool connectTo(const QString &host, int port, const QString &token, uint8_t deviceTypeId, uint8_t methodId);
    void sendPing();
    void sendIOInfo();
    void sendUpdateInput(uint32_t id, uint8_t value);
    void sendRequestToEnterQueue(const QString &token);

    void onTimeRefresh();

    void disconnect() {
        if (m_socket.isOpen()) {
            m_socket.disconnectFromHost();
            m_socket.waitForDisconnected(1000);
            m_socket.close();
        }

        // will no longer trigger the setup window
        setAuthToken("");

        // reset available pins, the mapped one shall still be configured (in case of incompatibility, they will not be applied)
        m_availablePins.clear();

        // reset time variables
        m_minWaitTime = 0;
        m_aliveSince = 0;
        m_allowUntil = 0;
        m_afterTimeStartedEpoch = 0;
        m_startedTimeEpoch = 0;

        m_inQueue = false;
        m_queuePos = 0;
        m_deviceAllowedTime = 0;
        m_queueWaitingSinceEpoch = 0;
        m_queueEstimatedEpoch = 0;

        m_connected = false;

        update();
    }

    const QString &authToken() const {
        return m_authToken;
    }

    void setAuthToken(const QString &token) {
        m_authToken = token;
    }

    const QString &deviceMethod() const {
        return m_deviceMethod;
    }

    void setDeviceMethod(const QString &method) {
        m_deviceMethod = method;
    }

    const DeviceAuth &deviceAuth() const {
        return m_deviceAuth;
    }

    void setDeviceAuth(const QString &name, const QString &token) {
        m_deviceAuth = {name, token};
    }

    uint16_t deviceMethodId() const {
        return m_methodId;
    }

    uint16_t deviceTypeId() const {
        return m_deviceTypeId;
    }

    uint16_t deviceId() const {
        return m_deviceId;
    }

    void setDeviceId(uint16_t id) {
        m_deviceId = id;
    }

    uint16_t latency() const {
        return m_latency;
    }

    void setLatency(uint16_t milliseconds) {
        m_latency = milliseconds;
    }

    bool isAlive() const {
        return (static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch() / 1000) < (aliveSince() + 10));
    }

    uint64_t aliveSince() const {
        return m_aliveSince;
    }

    void setAliveSince(uint64_t epochInSeconds) {
        m_aliveSince = epochInSeconds;
    }

    uint64_t totalAllowUntil() const {
        return m_allowUntil + static_cast<uint64_t>(minWaitTime());
    }

    uint64_t allowUntil() const {
        return m_allowUntil;
    }

    void setAllowUntil(uint64_t epochInSeconds) {
        m_allowUntil = epochInSeconds;
    }

    uint32_t minWaitTime() const {
        return m_minWaitTime;
    }

    void setMinWaitTime(uint32_t minWaitTimeInSeconds) {
        m_minWaitTime = minWaitTimeInSeconds;
    }

    long timeAllowed() const {
        return m_timeAllowed;
    }

    uint64_t startedTimeEpoch() const {
        return m_startedTimeEpoch;
    }

    void initTimeCount() {
        m_startedTimeEpoch = QDateTime::currentMSecsSinceEpoch() / 1000;
        m_timeAllowed = static_cast<long>(totalAllowUntil() - m_startedTimeEpoch);

        if (m_timeAllowed < 0) {
            m_timeAllowed = 0;
        }
    }

    uint64_t afterTimeStartedEpoch() const {
        return m_afterTimeStartedEpoch;
    }

    bool hasAfterTimeStarted() const {
        return m_afterTimeStartedEpoch != 0;
    }

    long availableAfterTime() const {
        if (!hasAfterTimeStarted()) {
            return m_minWaitTime;
        }

        uint64_t currentEpoch = QDateTime::currentMSecsSinceEpoch() / 1000;
        long passedTime = static_cast<long>(currentEpoch - m_afterTimeStartedEpoch);

        if (passedTime < 0) {
            return m_minWaitTime;
        }

        return static_cast<long>(m_minWaitTime) - passedTime;
    }

    void startAfterTime(uint64_t epochInSeconds) {
        m_afterTimeStartedEpoch = epochInSeconds;
    }

    RemoteLabOption currentOption() const {
        return m_currentOption;
    }

    void setCurrentOption(const RemoteLabOption &option) {
        m_currentOption = option;
    }

    const QList<Pin> &availablePins() const {
        return m_availablePins;
    }

    void addPin(uint32_t id, const QString &port, uint8_t pinType) {
        m_availablePins.push_back({id, port, static_cast<Pin_Type>(pinType)});
    }

    void resetPortMapping() {
        m_mappedPins.clear();
        m_mappedOutputs.clear();
    }

    const QList<Pin> &mappedPins() const {
        return m_mappedPins;
    }

    uint32_t pinId(const QString &name, uint8_t pinType) const {
        const auto &pins = availablePins();

        for (auto pin : pins) {
            if (pin.name() != name) {
                continue;
            }

            if (pin.type() != Pin_Type::General_Purpose && pin.type() != static_cast<Pin_Type>(pinType)) {
                return 0;
            }

            auto mappedPins_ = mappedPins();

            for (auto mappedPin : mappedPins_) {
                if (mappedPin.name() == name) {
                    return 0;
                }
            }

            return pin.m_id;
        }

        return 0;
    }

    bool canBeMapped(const QString &name, uint8_t pinType) {
        return (pinId(name, pinType) != 0);
    }

    bool mapPin(const QString &name, uint8_t pinType) {
        uint32_t id = pinId(name, pinType);

        if (id == 0) {
            return false;
        }

        // qDebug() << "Pin(" << id << ", " << name << ", " << static_cast<PIN_TYPE>(pinType) << ")";
        m_mappedPins.push_back(Pin(id, name, static_cast<Pin_Type>(pinType)));

        if (static_cast<Pin_Type>(pinType) == Pin_Type::Input) {
            m_mappedInputs[id] = false;
        }

        if (static_cast<Pin_Type>(pinType) == Pin_Type::Output) {
            m_mappedOutputs[id] = false;
        }

        return true;
    }

    const QMap<uint32_t, bool> &inputs() {
        return m_mappedInputs;
    }

    void setInput(uint32_t id, bool value) {
        if (m_mappedInputs[id] != value) {
            m_mappedInputs[id] = value;
            sendUpdateInput(id, value);
        }
    }

    const QMap<uint32_t, bool> &outputs() {
        return m_mappedOutputs;
    }

    void setOutput(uint32_t id, bool value) {
        m_mappedOutputs[id] = value;
    }

    void loadAvailablePin(QDataStream &ds);
    void loadMappedPin(QDataStream &ds);
    void loadRemoteIO(QDataStream &ds, const QVersionNumber version);

    // Queue
    bool isInQueue() const {
        return m_inQueue;
    }

    void setIsInQueue(bool boolean) {
        m_inQueue = boolean;
        m_queueWaitingSinceEpoch = QDateTime::currentMSecsSinceEpoch() / 1000;
    }

    uint8_t queuePos() const {
        return m_queuePos;
    }

    void setQueuePos(uint8_t pos) {
        m_queuePos = pos;
    }

    uint64_t queueEstimatedEpoch() const {
        return m_queueEstimatedEpoch;
    }

    void setQueueEstimatedEpoch(uint64_t epoch) {
        m_queueEstimatedEpoch = epoch;
    }

    uint64_t waitingSince() const {
        return m_queueWaitingSinceEpoch;
    }

    uint32_t deviceAllowedTime() const {
        return m_deviceAllowedTime;
    }

    void setDeviceAllowedTime(uint32_t timeInSeconds) {
        m_deviceAllowedTime = timeInSeconds;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    bool isConnected() const {
        return m_connected;
    }

    void updatePorts();

    void setSkin(bool defaultSkin, const QString &filename) override {
        Q_UNUSED(defaultSkin)
        Q_UNUSED(filename)
    };

    static bool loadSettings(const QDomDocument &xml);

    const QList<RemoteLabOption> &options() {
        return s_options;
    }

    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version);
    void save(QDataStream &ds) const override;

private:
    void readIsDone();
    void close();

    bool m_lastClk;
    bool m_lastValue;
    bool m_connected;

    uint8_t m_deviceTypeId;
    uint8_t m_methodId;
    uint16_t m_deviceId;
    uint16_t m_latency;
    uint32_t m_minWaitTime;
    uint64_t m_afterTimeStartedEpoch;
    uint64_t m_aliveSince;
    uint64_t m_allowUntil;
    uint64_t m_startedTimeEpoch;
    long m_timeAllowed;
    QString m_authToken;
    QString m_deviceMethod;
    DeviceAuth m_deviceAuth;

    // Queue
    bool m_inQueue;
    uint8_t m_queuePos;
    uint32_t m_deviceAllowedTime;
    uint64_t m_queueWaitingSinceEpoch;
    uint64_t m_queueEstimatedEpoch;

    static QList<RemoteLabOption> s_options;
    QList<Pin> m_availablePins;
    QList<Pin> m_mappedPins;

    QMap<uint32_t, bool> m_mappedInputs;
    QMap<uint32_t, bool> m_mappedOutputs;

    RemoteLabOption m_currentOption;
    QTcpSocket m_socket;
    QTimer m_timer;
};
