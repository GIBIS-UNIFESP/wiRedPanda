#include "protocol.h"

#include <iostream>

#include <QMessageBox>

std::map<uint8_t, parse_function> RemoteProtocol::parseMapping;
bool RemoteProtocol::initialized = false;
QWidget* RemoteProtocol::warningMsgParent = nullptr;

#define REGISTER_PARSE_OPCODE(opcode, parse_func_id) parseMapping.insert(it, std::pair<Opcodes, parse_function>(Opcodes::opcode, parse_func_id))

void RemoteProtocol::init(QWidget *parent) {
    if (!initialized) {
        auto it = parseMapping.begin();

        REGISTER_PARSE_OPCODE(OPCODE_START_SESSION, parse_session_start);
        REGISTER_PARSE_OPCODE(OPCODE_PONG, parse_pong);
        REGISTER_PARSE_OPCODE(OPCODE_UPDATE_OUTPUT, parse_output);
        REGISTER_PARSE_OPCODE(OPCODE_TIME_WARNING, parse_time_warning);
        REGISTER_PARSE_OPCODE(OPCODE_QUEUE_INFO, parse_queue_info);

        initialized = true;
        warningMsgParent = parent;
    }
}

void RemoteProtocol::parse_session_start(RemoteDevice* elm, NetworkIncomingMessage& imsg) {
    QString token = imsg.popString();
    uint16_t device_id = imsg.pop<quint16>();

    // error occured
    if (device_id == 0) {
        uint8_t errorCode = imsg.pop<quint8>();
        QString errorMsg = imsg.popString();

        switch (errorCode) {
            // unable to authenticate
            case 0: {
                elm->disconnect();
                QMessageBox messageBox;
                messageBox.critical(0,"Error",errorMsg);
                break;
            }

            // not enough devices, enter queue?
            case 1: {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(0, "Not enough devices", errorMsg,
                    QMessageBox::Yes|QMessageBox::No);

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
                messageBox.critical(0,"Error",errorMsg);
                break;
            }
        }

        return;
    }

    QString method = imsg.popString();
    QString device_name = imsg.popString();
    QString device_token = imsg.popString();
    uint32_t min_wait_time = imsg.pop<quint32>();
    uint64_t allow_until = imsg.pop<quint64>();
    uint16_t pin_amount = imsg.pop<quint16>();

    bool alreadyLoaded = false;
    if (elm->getAvailablePins().size() > 0)
        alreadyLoaded = true;

    bool portMappingReseted = false;

    for (int i = 0; i<pin_amount; i++) {
        uint32_t id = imsg.pop<quint32>();
        QString pinName = imsg.popString();
        uint8_t type = imsg.pop<quint8>();

        std::cerr << "> pin " << i << ": " << static_cast<int>(id) << ", " << pinName.toStdString() << ", " << static_cast<int>(type) << std::endl;

        const std::list<Pin>& availablePins = elm->getAvailablePins();
        if (alreadyLoaded) {
            // we must verify that all the available pins are present on this new connection
            std::list<Pin>::const_iterator it = availablePins.begin();
            bool found = false;
            while(it != availablePins.end()) {
                if (it->getId() == id && QString::fromStdString(it->getName()).compare(pinName) == 0 && static_cast<uint8_t>(it->getType()) == type) {
                    found = true;
                    break;
                }
                ++it;
            }

            if (!found) {
                // could not found, reseting port mapping list because we cannot trust it
                portMappingReseted = true;

                elm->resetPortMapping();
                elm->addPin(id, pinName.toStdString(), type);
                std::cerr << "> adding not found pin " << i << ": " << static_cast<int>(id) << ", " << pinName.toStdString() << ", " << static_cast<int>(type) << std::endl;
            }
        } else {
            elm->addPin(id, pinName.toStdString(), type);
            std::cerr << "> adding pin " << i << ": " << static_cast<int>(id) << ", " << pinName.toStdString() << ", " << static_cast<int>(type) << std::endl;
        }
    }

    std::cout << token.toStdString() << std::endl;

    elm->setDeviceId(device_id);
    elm->setDeviceMethod(method.toStdString());
    elm->setDeviceAuth(device_name.toStdString(), device_token.toStdString());
    elm->setAuthToken(token.toStdString());
    elm->setMinWaitTime(min_wait_time);
    elm->setAllowUntil(allow_until);
    elm->initTimeCount();

    elm->setIsInQueue(false);
    elm->setQueuePos(0);
    elm->setQueueEstimatedEpoch(0);

    if (portMappingReseted) {
        QMessageBox *msgBox = new QMessageBox(warningMsgParent);
        msgBox->setIcon( QMessageBox::Warning );
        msgBox->setText("Since available pins are slightly different, the mapped pins have been reseted.");
        msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
        msgBox->show();
        msgBox->raise();
    }

    QMessageBox *msgBox = new QMessageBox(warningMsgParent);
    msgBox->setIcon( QMessageBox::Information );
    msgBox->setText("Connection estabilished!");
    msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
    msgBox->show();
    msgBox->raise();

}

void RemoteProtocol::parse_pong(RemoteDevice* elm, NetworkIncomingMessage& imsg) {
    std::cout << "PONG" << std::endl;

    uint64_t timestamp = imsg.pop<quint64>();

    uint64_t current = QDateTime::currentMSecsSinceEpoch();

    elm->setLatency(current-timestamp);
    elm->setAliveSince(QDateTime::currentSecsSinceEpoch());
}

void RemoteProtocol::parse_output(RemoteDevice* elm, NetworkIncomingMessage& imsg) {
    std::cout << "OUTPUT" << std::endl;
    uint32_t pinId = imsg.pop<quint32>();
    uint8_t value = imsg.pop<quint8>();

    elm->setOutput(pinId, value == 0 ? false : true);
}

void RemoteProtocol::parse_time_warning(RemoteDevice* elm, NetworkIncomingMessage& imsg) {
    std::cout << "TIME WARNING" << std::endl;
    uint8_t isWarning = imsg.pop<quint8>();

    // the second time the connection is disconnected
    if (isWarning == 0) {
        uint64_t startedEpoch = elm->getStartedTimeEpoch();

        long amountOfSeconds = static_cast<long>(QDateTime::currentSecsSinceEpoch() - static_cast<int64_t>(startedEpoch));
        QString time = QDateTime::fromSecsSinceEpoch(static_cast<uint32_t>(amountOfSeconds), Qt::UTC).toString("hh:mm:ss");

        elm->disconnect();

        QMessageBox *msgBox = new QMessageBox(warningMsgParent);
        msgBox->setIcon( QMessageBox::Warning );
        msgBox->setText("Once there was other users waiting to use the device, you have been disconnected after " + time + " time of continuous usage.");
        msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
        msgBox->show();
        msgBox->raise();

        return;
    }

    uint64_t afterTimeStartedEpoch = imsg.pop<quint64>();

    QString time;
    if (elm->getMinWaitTime() > 60) {
        time = QString::number(static_cast<int>(static_cast<long>(elm->getMinWaitTime())/60)) + " more minutes";
    } else {
        time = QString::number(static_cast<int>(elm->getMinWaitTime())) + " more seconds";
    }

    elm->startAfterTime(afterTimeStartedEpoch);
    std::cerr << "afterTimeStartedEpoch: " << afterTimeStartedEpoch << std::endl;

    QMessageBox *msgBox = new QMessageBox(warningMsgParent);
    msgBox->setIcon( QMessageBox::Warning );
    msgBox->setText("There are other users in queue for using the system, you have been granted " + time + " to finish your use. If you are no longer using the remote device, please disconnect.");
    msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
    msgBox->show();
    msgBox->raise();
}

// TODO: Needs to update minWaitTime (it's on default state, minWaitTime = 0)
void RemoteProtocol::parse_queue_info(RemoteDevice* elm, NetworkIncomingMessage& imsg) {
    std::cout << "QUEUE INFO" << std::endl;

    QString userToken = imsg.popString();

    // user amount - not in use
    imsg.pop<quint8>();

    uint8_t currPos = imsg.pop<quint8>();
    uint32_t allowedTimeInSeconds = imsg.pop<quint32>();
    uint64_t estimatedEpoch = imsg.pop<quint64>();

    if (!elm->isInQueue()) {
        elm->setIsInQueue(true);
        elm->setAuthToken(userToken.toStdString());
        elm->setQueuePos(currPos);
        elm->setDeviceAllowedTime(allowedTimeInSeconds);
        elm->setQueueEstimatedEpoch(estimatedEpoch);
    } else {
        // let's just update the estimated time
        elm->setQueuePos(currPos);

        // the estimated time shall only decrease, never increase.
        if (elm->getQueueEstimatedEpoch() > estimatedEpoch)
            elm->setQueueEstimatedEpoch(estimatedEpoch);
        // if time is increasing more than the minimim wait time in seconds
        // (plus 20 seconds from connect/disconnect required time and clock diferences), then update.
        else if(static_cast<int>(estimatedEpoch-elm->getQueueEstimatedEpoch()) > static_cast<int>(elm->getMinWaitTime() + 20))
            elm->setQueueEstimatedEpoch(estimatedEpoch);
    }
}

void RemoteProtocol::parse(RemoteDevice* elm, uint8_t opcode, QByteArray byteArray) {

    assert(initialized);

    NetworkIncomingMessage imsg(opcode, byteArray);

    auto it = parseMapping.find(opcode);

    if (it != parseMapping.end()) {
        it->second(elm, imsg);

        // all bytes should be read
        assert(imsg.getRemainingBytes() <= 0);
    }
}

NetworkOutgoingMessage RemoteProtocol::sendPing() {
    NetworkOutgoingMessage msg(2);

    uint64_t timestamp = QDateTime::currentMSecsSinceEpoch();
    msg.addByte<quint64>(timestamp);
    msg.addSize();

    return msg;
}

NetworkOutgoingMessage RemoteProtocol::sendIOInfo(uint16_t latency, const std::list<Pin>& mappedPins) {
    NetworkOutgoingMessage msg(3);

    msg.addByte<quint16>(latency);
    msg.addByte<quint16>(mappedPins.size());

    for ( const Pin& pin : mappedPins) {
        msg.addByte<quint32>(pin.getId());
        msg.addByte<quint8>(pin.getType());
    }

    msg.addSize();

    return msg;
}

NetworkOutgoingMessage RemoteProtocol::sendUpdateInput(uint32_t id, uint8_t value) {
    NetworkOutgoingMessage msg(4);
    msg.addByte<quint32>(id);
    msg.addByte<quint8>(value);

    msg.addSize();

    return msg;
}

// TODO: send device type id
NetworkOutgoingMessage RemoteProtocol::sendRequestToWaitOnQueue(RemoteDevice* remoteDevice, const QString& token) {
    std::cout << "sendRequestToWaitOnQueue" << std::endl;
    NetworkOutgoingMessage msg(5);
    msg.addString(token);
    msg.addByte<quint8>(remoteDevice->getDeviceTypeId());
    msg.addByte<quint8>(remoteDevice->getDeviceMethodId());
    msg.addSize();

    return msg;
}
