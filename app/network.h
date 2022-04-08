#pragma once

#include <QBuffer>
#include <QDataStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextStream>
#include <QtEndian>

class NetworkOutgoingMessage : public QByteArray
{
public:
    explicit NetworkOutgoingMessage(uint8_t opcode) : opcode(opcode) { addByte<quint8>(opcode); }

    template <class T> void addByte(T data) {
        QBuffer buffer(this);
        buffer.open(QIODevice::Append);

        data = qToBigEndian<T>(data);

        buffer.write(reinterpret_cast<const char *>(data));
    }

    void addString(const QString &str) {
        addByte<quint16>(str.size());
        QByteArray inUtf8 = str.toUtf8();
        append(inUtf8);
    }

    void addSize() {
        // insert size into a new byte array
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);

        int size_ = qToBigEndian<int>(size());

        buffer.write(reinterpret_cast<const char *>(size_));

        // push to NetworkMessage at the beggining
        push_front(byteArray);
    }

    uint8_t getOpcode() const {
        return opcode;
    }

private:
    uint8_t opcode;
};

class NetworkIncomingMessage
{
public:
    NetworkIncomingMessage(uint8_t opcode, const QByteArray &byteArray) : opcode(opcode), stream(byteArray), size(byteArray.size()), remainingBytes(size) {}

    template <class T> T pop() {
        T ret;
        stream >> ret;
        remainingBytes -= sizeof(T);
        return ret;
    }
    QString popString() {
        auto size = pop<uint16_t>();

        QByteArray ba;
        for (int i = 0; i < size; i++) {
            ba.push_back(static_cast<char>(pop<uint8_t>()));
        }

        QString ret(ba);

        return ret;
    }

    uint32_t getSize() const {
        return size;
    }
    uint8_t getOpcode() const {
        return opcode;
    }
    uint32_t getRemainingBytes() const {
        return remainingBytes;
    }

private:
    uint8_t opcode;
    QDataStream stream;
    uint32_t size;
    uint32_t remainingBytes;
};
