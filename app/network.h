#ifndef NETWORK_H
#define NETWORK_H

#include <QBuffer>
#include <QtEndian>
#include <QTextStream>
#include <QDataStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class NetworkOutgoingMessage : public QByteArray {
    uint8_t opcode;
public:
    NetworkOutgoingMessage(uint8_t opcode) : opcode(opcode) { addByte<quint8>(opcode); }

    template<class T>
    void addByte(T data) {
        QBuffer buffer(this);
        buffer.open(QIODevice::Append);

        data = qToBigEndian<T>(data);

        buffer.write((char*) &data, sizeof(data));
    }

    void addString(QString str) {
        addByte<quint16>(str.size());
        QByteArray inUtf8 = str.toUtf8();
        append(inUtf8);
    }

    void addSize() {
        // gets message size
        int siz = size();

        // insert size into a new byte array
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);

        siz = qToBigEndian<int>(siz);

        buffer.write((char*) &siz, sizeof(siz));

        // push to NetworkMessage at the beggining
        push_front(byteArray);
    }

    uint8_t getOpcode() { return opcode; }
};

class NetworkIncomingMessage {
    uint8_t opcode;
    QDataStream stream;
    uint32_t size;
    uint32_t remainingBytes;

public:
    NetworkIncomingMessage(uint8_t opcode, QByteArray byteArray) : opcode(opcode), stream(byteArray), size(byteArray.size()), remainingBytes(size) {}

    template <class T>
    T pop() { T ret; stream >> ret; remainingBytes-=sizeof(T); return ret; }
    QString popString() {
        uint16_t size = pop<uint16_t>();

        QByteArray ba;
        for (int i = 0; i < size; i++)
            ba.push_back(static_cast<char>(pop<uint8_t>()));

        QString ret(ba);

        return ret;
    }

    uint32_t getSize() { return size; }
    uint8_t getOpcode() { return opcode; }
    uint32_t getRemainingBytes() { return remainingBytes; }
};


#endif // NETWORK_H
