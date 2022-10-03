#pragma once

#include "network.h"
#include "remote.h"

#include <QDataStream>

#include <map>

using parse_function = void (*)(Remote *, NetworkIncomingMessage &);

// TODO: change to enum class
enum Opcodes
{
    OPCODE_NONE = 0,
    OPCODE_START_SESSION,
    OPCODE_PONG,
    OPCODE_UPDATE_OUTPUT,
    OPCODE_TIME_WARNING,
    OPCODE_QUEUE_INFO,
};

class RemoteProtocol
{
public:
    static void init(QWidget *parent = nullptr);
    static void parse(Remote *elm, uint8_t opcode, const QByteArray &byteArray);

    static void parse_session_start(Remote *elm, NetworkIncomingMessage &imsg);
    static void parse_pong(Remote *elm, NetworkIncomingMessage &imsg);
    static void parse_output(Remote *elm, NetworkIncomingMessage &imsg);
    static void parse_time_warning(Remote *elm, NetworkIncomingMessage &imsg);
    static void parse_queue_info(Remote *elm, NetworkIncomingMessage &imsg);

    static NetworkOutgoingMessage sendPing();
    static NetworkOutgoingMessage sendIOInfo(uint16_t latency, const QList<Pin> &mappedPins);
    static NetworkOutgoingMessage sendUpdateInput(uint32_t id, uint8_t value);
    static NetworkOutgoingMessage sendRequestToWaitOnQueue(Remote *remoteDevice, const QString &token);

private:
    static QMap<uint8_t, parse_function> parseMapping;
    static QWidget *warningMsgParent;
    static bool initialized;
};
