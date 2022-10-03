#include "protocol.h"

#include <iostream>

#include <QMessageBox>

QMap<uint8_t, parse_function> RemoteProtocol::parseMapping;
bool RemoteProtocol::initialized = false;
QWidget *RemoteProtocol::warningMsgParent = nullptr;

void RemoteProtocol::init(QWidget *parent)
{
    if (!initialized) {
        parseMapping.insert(OPCODE_START_SESSION, parse_session_start);
        parseMapping.insert(OPCODE_PONG, parse_pong);
        parseMapping.insert(OPCODE_UPDATE_OUTPUT, parse_output);
        parseMapping.insert(OPCODE_TIME_WARNING, parse_time_warning);
        parseMapping.insert(OPCODE_QUEUE_INFO, parse_queue_info);

        initialized = true;
        warningMsgParent = parent;
    }
}

void RemoteProtocol::parse_session_start(Remote *elm, NetworkIncomingMessage &imsg)
{
    auto token = imsg.popString();
    auto device_id = imsg.pop<quint16>();

    // error occured
    if (device_id == 0) {
        auto errorCode = imsg.pop<quint8>();
        auto errorMsg = imsg.popString();

        switch (errorCode) {
        // unable to authenticate
        case 0: {
            elm->disconnect();
            QMessageBox messageBox;
            messageBox.critical(nullptr, "Error", errorMsg);
            break;
        }

            // not enough devices, enter queue?
        case 1: {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(nullptr, "Not enough devices", errorMsg, QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                elm->sendRequestToEnterQueue(token);
            } else {
                elm->disconnect();
            }

            break;
        }

        default: {
            elm->disconnect();
            QMessageBox messageBox;
            messageBox.critical(nullptr, "Error", errorMsg);
            break;
        }
        }

        return;
    }

    auto method = imsg.popString();
    auto device_name = imsg.popString();
    auto device_token = imsg.popString();
    auto min_wait_time = imsg.pop<quint32>();
    auto allow_until = imsg.pop<quint64>();
    auto pin_amount = imsg.pop<quint16>();

    bool alreadyLoaded = false;
    if (!elm->availablePins().isEmpty()) {
        alreadyLoaded = true;
    }

    bool portMappingReseted = false;

    for (int i = 0; i < pin_amount; i++) {
        auto id = imsg.pop<quint32>();
        auto pinName = imsg.popString();
        auto type = imsg.pop<quint8>();

        qDebug() << "> pin " << i << ": " << static_cast<int>(id) << ", " << pinName << ", " << static_cast<int>(type);

        const QList<Pin> &availablePins = elm->availablePins();
        if (alreadyLoaded) {
            // we must verify that all the available pins are present on this new connection
            auto it = availablePins.begin();
            bool found = false;
            while (it != availablePins.end()) {
                if (it->id() == id && it->name() == pinName && static_cast<uint8_t>(it->type()) == type) {
                    found = true;
                    break;
                }
                ++it;
            }

            if (!found) {
                // could not found, reseting port mapping list because we cannot trust it
                portMappingReseted = true;

                elm->resetPortMapping();
                elm->addPin(id, pinName, type);
                qDebug() << "> adding not found pin " << i << ": " << static_cast<int>(id) << ", " << pinName << ", " << static_cast<int>(type);
            }
        } else {
            elm->addPin(id, pinName, type);
            qDebug() << "> adding pin " << i << ": " << static_cast<int>(id) << ", " << pinName << ", " << static_cast<int>(type);
        }
    }

    qDebug() << token;

    elm->setDeviceId(device_id);
    elm->setDeviceMethod(method);
    elm->setDeviceAuth(device_name, device_token);
    elm->setAuthToken(token);
    elm->setMinWaitTime(min_wait_time);
    elm->setAllowUntil(allow_until);
    elm->initTimeCount();

    elm->setIsInQueue(false);
    elm->setQueuePos(0);
    elm->setQueueEstimatedEpoch(0);

    if (portMappingReseted) {
        auto *msgBox = new QMessageBox(warningMsgParent);
        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setText("Since available pins are slightly different, the mapped pins have been reseted.");
        msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
        msgBox->show();
        msgBox->raise();
    }

    auto *msgBox = new QMessageBox(warningMsgParent);
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setText("Connection estabilished!");
    msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
    msgBox->show();
    msgBox->raise();
}

void RemoteProtocol::parse_pong(Remote *elm, NetworkIncomingMessage &imsg)
{
    qDebug() << "PONG";

    uint64_t timestamp = imsg.pop<quint64>();

    uint64_t current = QDateTime::currentMSecsSinceEpoch();

    elm->setLatency(current - timestamp);
    elm->setAliveSince(QDateTime::currentMSecsSinceEpoch() / 1000);
}

void RemoteProtocol::parse_output(Remote *elm, NetworkIncomingMessage &imsg)
{
    qDebug() << "OUTPUT";
    auto pinId = imsg.pop<quint32>();
    auto value = imsg.pop<quint8>();

    elm->setOutput(pinId, value);
}

void RemoteProtocol::parse_time_warning(Remote *elm, NetworkIncomingMessage &imsg)
{
    qDebug() << "TIME WARNING";
    auto isWarning = imsg.pop<quint8>();

    // the second time the connection is disconnected
    if (isWarning == 0) {
        uint64_t startedEpoch = elm->startedTimeEpoch();

        long amountOfSeconds = static_cast<long>(QDateTime::currentMSecsSinceEpoch() / 1000 - static_cast<int64_t>(startedEpoch));
        QString time = QDateTime::fromMSecsSinceEpoch(static_cast<uint32_t>(amountOfSeconds * 1000), Qt::UTC).toString("hh:mm:ss");

        elm->disconnect();

        auto *msgBox = new QMessageBox(warningMsgParent);
        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setText("Once there was other users waiting to use the device, you have been disconnected after " + time + " time of continuous usage.");
        msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
        msgBox->show();
        msgBox->raise();

        return;
    }

    uint64_t afterTimeStartedEpoch = imsg.pop<quint64>();

    QString time;
    if (elm->minWaitTime() > 60) {
        time = QString::number(static_cast<int>(static_cast<long>(elm->minWaitTime()) / 60)) + " more minutes";
    } else {
        time = QString::number(static_cast<int>(elm->minWaitTime())) + " more seconds";
    }

    elm->startAfterTime(afterTimeStartedEpoch);
    qDebug() << "afterTimeStartedEpoch: " << afterTimeStartedEpoch;

    auto *msgBox = new QMessageBox(warningMsgParent);
    msgBox->setIcon(QMessageBox::Warning);
    msgBox->setText("There are other users in queue for using the system, you have been granted " + time + " to finish your use. If you are no longer using the remote device, please disconnect.");
    msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
    msgBox->show();
    msgBox->raise();
}

// TODO: Needs to update minWaitTime (it's on default state, minWaitTime = 0)
void RemoteProtocol::parse_queue_info(Remote *elm, NetworkIncomingMessage &imsg)
{
    qDebug() << "QUEUE INFO";

    QString userToken = imsg.popString();

    // user amount - not in use
    imsg.pop<quint8>();

    auto currPos = imsg.pop<quint8>();
    auto allowedTimeInSeconds = imsg.pop<quint32>();
    auto estimatedEpoch = imsg.pop<quint64>();

    if (!elm->isInQueue()) {
        elm->setIsInQueue(true);
        elm->setAuthToken(userToken);
        elm->setQueuePos(currPos);
        elm->setDeviceAllowedTime(allowedTimeInSeconds);
        elm->setQueueEstimatedEpoch(estimatedEpoch);
    } else {
        // let's just update the estimated time
        elm->setQueuePos(currPos);

        // the estimated time shall only decrease, never increase.
        if (elm->queueEstimatedEpoch() > estimatedEpoch) {
            elm->setQueueEstimatedEpoch(estimatedEpoch);
        }
        // if time is increasing more than the minimum wait time in seconds
        // (plus 20 seconds from connect/disconnect required time and clock diferences), then update.
        else if (static_cast<int>(estimatedEpoch - elm->queueEstimatedEpoch()) > static_cast<int>(elm->minWaitTime() + 20)) {
            elm->setQueueEstimatedEpoch(estimatedEpoch); // FIX: this else is a copy of the if above
        }
    }
}

void RemoteProtocol::parse(Remote *elm, uint8_t opcode, const QByteArray &byteArray)
{
    Q_ASSERT(initialized);

    NetworkIncomingMessage imsg(opcode, byteArray);

    if (parseMapping.contains(opcode)) {
        parseMapping.value(opcode)(elm, imsg);

        // all bytes should be read
        Q_ASSERT(imsg.getRemainingBytes() <= 0);
    }
}

NetworkOutgoingMessage RemoteProtocol::sendPing()
{
    NetworkOutgoingMessage msg(2);

    uint64_t timestamp = QDateTime::currentMSecsSinceEpoch();
    msg.addByte<quint64>(timestamp);
    msg.addSize();

    return msg;
}

NetworkOutgoingMessage RemoteProtocol::sendIOInfo(uint16_t latency, const QList<Pin> &mappedPins)
{
    NetworkOutgoingMessage msg(3);

    msg.addByte<quint16>(latency);
    msg.addByte<quint16>(mappedPins.size());

    for (const Pin &pin : mappedPins) {
        msg.addByte<quint32>(static_cast<quint32>(pin.id()));
        msg.addByte<quint8>(static_cast<quint8>(pin.type()));
    }

    msg.addSize();

    return msg;
}

NetworkOutgoingMessage RemoteProtocol::sendUpdateInput(uint32_t id, uint8_t value)
{
    NetworkOutgoingMessage msg(4);
    msg.addByte<quint32>(id);
    msg.addByte<quint8>(value);

    msg.addSize();

    return msg;
}

// TODO: send device type id
NetworkOutgoingMessage RemoteProtocol::sendRequestToWaitOnQueue(Remote *remoteDevice, const QString &token)
{
    qDebug() << "sendRequestToWaitOnQueue";
    NetworkOutgoingMessage msg(5);
    msg.addString(token);
    msg.addByte<quint8>(remoteDevice->deviceTypeId());
    msg.addByte<quint8>(remoteDevice->deviceMethodId());
    msg.addSize();

    return msg;
}
