#include "remote.h"

#include "globalproperties.h"
#include "protocol.h"
#include "qneport.h"

#include <QMessageBox>
#include <QPainter>
#include <iostream>

Auth_Method toAuthMethod(const QString &auth_type)
{
    static const QMap<QString, Auth_Method> optionStrings{
        {"plain", Auth_Method::Plain},
        {"md5", Auth_Method::MD5},
        {"sha1", Auth_Method::SHA1},
        {"sha-1", Auth_Method::SHA1},
        {"sha256", Auth_Method::SHA256},
        {"sha-256", Auth_Method::SHA256}
    };

    if (optionStrings.contains(auth_type)) {
        return optionStrings.value(auth_type);
    }

    return Auth_Method::None;
}

Pin_Type Pin::convertTypeString(const QString &typeName)
{
    Pin_Type t = Pin_Type::None;

    if (typeName == "INPUT") {
        t = Pin_Type::Input;
    } else if (typeName == "OUTPUT") {
        t = Pin_Type::Output;
    } else if (typeName == "GENERAL_PURPOSE") {
        t = Pin_Type::General_Purpose;
    }

    return t;
}

QList<RemoteLabOption> Remote::s_options;

Remote::Remote(QGraphicsItem *parent)
    : GraphicElement(ElementType::Remote, ElementGroup::Remote, ":/remote_device.png", tr("REMOTE"), tr("Remote"), 0, 55, 0, 55, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/remote/device/base.png"
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setRotatable(false);
    setupPorts();
    updatePorts();
    setPortName("Remote");
    setHasCustomConfig(true);
    m_lastValue = false;
    m_lastClk = false;
    m_deviceId = 0;
    m_deviceTypeId = 0;
    m_authToken = "";
    m_deviceAuth = {"", ""};

    m_minWaitTime = 0;
    m_aliveSince = 0;
    m_allowUntil = 0;
    m_afterTimeStartedEpoch = 0;
    m_startedTimeEpoch = 0;

    // Queue
    m_inQueue = false;
    m_queuePos = 0;
    m_deviceAllowedTime = 0;
    m_queueWaitingSinceEpoch = 0;
    m_queueEstimatedEpoch = 0;

    m_connected = false;

    // Try loading remote lab settings
    QDomDocument xml;
    // Load xml file as raw data

    qDebug() << QDir::currentPath();

    QFile f("remotelab.xml");
    if (!f.open(QIODevice::ReadOnly)) {
        // Error while loading file
        qDebug() << "Error while loading remotelab.xml file";
//        disable();
        return;
    }
    // Set data into the QDomDocument before processing
    xml.setContent(&f);
    f.close();

    if (!loadSettings(xml)) {
//        disable();
        return;
    }

    m_socket.setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    // No need for checking the amount of bytes written
    // connect(&socket, &QTcpSocket::bytesWritten, this, &Remote::handle);
    connect(&m_socket, &QTcpSocket::disconnected, this, &Remote::close);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Remote::readIsDone);

    connect(&m_timer, &QTimer::timeout, this, &Remote::onTimeRefresh);
    m_timer.start(1000);
}

Remote::~Remote()
{
    m_timer.stop();

    if (m_socket.isOpen()) {
        m_socket.disconnectFromHost();
    }
}

void Remote::onTimeRefresh()
{
    if (m_socket.isOpen()) {
        sendPing();
        update();

        if (!isAlive()) {
            if (authToken() != "") {
                auto *messageBox = new QMessageBox();
                messageBox->setAttribute(Qt::WA_DeleteOnClose);
                messageBox->setModal(false);
                messageBox->critical(nullptr, "Disconnected", "Sorry, we were unable to keep your connection up.\nPlease consider reconnecting, or checking if you are still connected to the internet.");
            }

            disconnect();
        }
    }
}

bool Remote::connectTo(const QString &host, int port, const QString &token, uint8_t deviceTypeId, uint8_t methodId)
{
    qDebug() << "Connection to " << host;

    m_deviceTypeId = deviceTypeId;
    m_methodId = methodId;

    setAliveSince(QDateTime::currentMSecsSinceEpoch() / 1000);
    m_socket.connectToHost(host, port);
    if (m_socket.waitForConnected(5000)) {
        NetworkOutgoingMessage msg(1);
        msg.addByte<quint8>(deviceTypeId);
        msg.addByte<quint8>(methodId);
        msg.addString(token);
        msg.addSize();

        m_socket.write(msg);
        m_socket.waitForReadyRead(5000);
        // qDebug() << socket.readLine();

        return true;
    }

    return false;
}

void Remote::sendPing()
{
    NetworkOutgoingMessage msg = RemoteProtocol::sendPing();
    m_socket.write(msg);
    m_socket.waitForReadyRead(1000);
}

void Remote::sendIOInfo()
{
    NetworkOutgoingMessage msg = RemoteProtocol::sendIOInfo(m_latency, mappedPins());
    m_socket.write(msg);
    m_socket.waitForReadyRead(1000);
}

void Remote::sendUpdateInput(uint32_t id, uint8_t value)
{
    NetworkOutgoingMessage msg = RemoteProtocol::sendUpdateInput(id, value);
    m_socket.write(msg);
}

void Remote::sendRequestToEnterQueue(const QString &token)
{
    NetworkOutgoingMessage msg = RemoteProtocol::sendRequestToWaitOnQueue(this, token);
    m_socket.write(msg);
}

void Remote::readIsDone()
{
    while (m_socket.bytesAvailable() > 0) {
        QByteArray headerBytes = m_socket.read(4);

        QDataStream hds(headerBytes);
        uint32_t size;
        hds >> size;

        QByteArray opcodeBytes = m_socket.read(1);
        QDataStream op_ds(opcodeBytes);

        uint8_t opcode;
        op_ds >> opcode;

        QByteArray bytes = m_socket.read(size - 1);
        QDataStream ds(bytes);

        RemoteProtocol::parse(this, opcode, bytes);
    }
}

void Remote::close()
{
    qDebug() << "Closing connection!";
}

void Remote::setupPorts()
{
    int inputAmount = 0;
    int outputAmount = 0;

    const auto &mappedPins_ = mappedPins();

    if (mappedPins_.size() <= 0) {
        setInputSize(0);
        setOutputSize(0);
        return;
    }

    for (const auto &p : mappedPins_) {
        if (p.type() == Pin_Type::Input) {
            inputAmount++;
        }
        if (p.type() == Pin_Type::Output) {
            outputAmount++;
        }
    }

    setInputSize(inputAmount);
    setOutputSize(outputAmount);

    inputAmount = 0;
    outputAmount = 0;
    for (const auto &p : mappedPins_) {
        if (p.type() == Pin_Type::Input) {
//            input(inputAmount)->setName(p.getName());
//            input(inputAmount)->setRemoteId(p.getId());
            inputAmount++;
        }
        if (p.type() == Pin_Type::Output) {
//            output(outputAmount)->setName(p.getName());
//            output(outputAmount)->setRemoteId(p.getId());
            outputAmount++;
        }
    }

    m_connected = true;
}

void Remote::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPixmap status_on(":/remote/device/status_on.png");
    QPixmap status_off(":/remote/device/status_off.png");

    QPixmap latency_0(":/remote/device/latency_0.png");
    QPixmap latency_1(":/remote/device/latency_1.png");
    QPixmap latency_2(":/remote/device/latency_2.png");
    QPixmap latency_3(":/remote/device/latency_3.png");
    QPixmap latency_4(":/remote/device/latency_4.png");

    GraphicElement::paint(painter, option, widget);

    if (isConnected()) {
        painter->drawPixmap(QPoint(0, 0), status_on, status_on.rect());
    } else {
        painter->drawPixmap(QPoint(0, 0), status_off, status_off.rect());
    }
    if (authToken() != "") {
        if (m_latency <= 20) {
            painter->drawPixmap(QPoint(0, 0), latency_4, latency_4.rect());
        } else if (m_latency > 20 && m_latency <= 70) {
            painter->drawPixmap(QPoint(0, 0), latency_3, latency_3.rect());
        } else if (m_latency > 70 && m_latency <= 130) {
            painter->drawPixmap(QPoint(0, 0), latency_2, latency_2.rect());
        } else if (m_latency > 130 && m_latency <= 180) {
            painter->drawPixmap(QPoint(0, 0), latency_1, latency_1.rect());
        } else if (m_latency > 180) {
            painter->drawPixmap(QPoint(0, 0), latency_0, latency_0.rect());
        } else {
            painter->drawPixmap(QPoint(0, 0), latency_0, latency_0.rect());
        }
    }
}

void Remote::updatePorts()
{
//    GraphicElement::updatePorts();
}

bool Remote::loadSettings(const QDomDocument &xml)
{
    // Extract the root markup
    QDomElement root = xml.documentElement();

    if (root.tagName() != "endpoints") {
        qDebug() << "Malformed remotelab.xml file";
        return false;
    }

    s_options.clear();

    QDomElement optionElm = root.firstChild().toElement();

    // Loop while there is a child
    while (!optionElm.isNull()) {
        RemoteLabOption option;

        // Check if the child tag name is option
        if (optionElm.tagName() == "option") {
            // Read and display the component name
            option.m_name = optionElm.attribute("name", "Unknown");

            // Get the first child of the component
            QDomElement component = optionElm.firstChild().toElement();

            // Read each child of the component node
            while (!component.isNull()) {
                // Read Name and value
                if (component.tagName() == "url") {
                    option.m_url = component.firstChild().toText().data();
                }
                if (component.tagName() == "auth") {
                    option.m_authMethod = toAuthMethod(component.firstChild().toText().data());
                }

                // Next child component
                component = component.nextSibling().toElement();
            }
        }

        s_options.push_back(option);
        qDebug() << "Creating option " << option.name() << " with url: " << option.url();

        // Next component
        optionElm = optionElm.nextSibling().toElement();
    }

    return true;
}

void Remote::loadAvailablePin(QDataStream &ds)
{
    quint32 portId; ds >> portId;
    QString portName; ds >> portName;
    quint8 portType; ds >> portType;

    addPin(portId, portName, portType);
    qDebug() << "> ADD at `loadAvailablePin` " << static_cast<int>(portId) << ", " << portName << ", " << static_cast<int>(portType);
}

void Remote::loadMappedPin(QDataStream &ds)
{
    QString portName; ds >> portName;
    quint8 portType; ds >> portType;

    mapPin(portName, portType);
}

void Remote::loadRemoteIO(QDataStream &ds, const QVersionNumber version)
{
    if (version >= VERSION("2.7")) {
        qDebug() << "Loading remote IO.";

        resetPortMapping();

        quint32 availablePinsAmount;
        ds >> availablePinsAmount;
        qDebug() << "> availablePinsAmount `loadRemoteIO` " << static_cast<int>(availablePinsAmount);
        for (size_t index = 0; index < availablePinsAmount; ++index) {
            loadAvailablePin(ds);
        }

        quint32 mappedPinsAmount;
        ds >> mappedPinsAmount;
        for (size_t index = 0; index < mappedPinsAmount; ++index) {
            loadMappedPin(ds);
        }
        refresh();
    }
}

void Remote::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(ds, portMap, version);

    loadRemoteIO(ds, version);
}

void Remote::save(QDataStream &ds) const
{
    GraphicElement::save(ds);

    /* <\Version2.7> */
    qDebug() << "Saving remote IO.";
    ds << static_cast<quint32>(m_availablePins.size());
    for (const Pin &pin : m_availablePins) {
        ds << static_cast<quint32>(pin.id());
        ds << pin.name();
        ds << static_cast<quint8>(pin.type());
    }

    ds << static_cast<quint32>(m_mappedPins.size());
    for (const Pin &pin : m_mappedPins) {
        ds << pin.name();
        ds << static_cast<quint8>(pin.type());
    }
}
