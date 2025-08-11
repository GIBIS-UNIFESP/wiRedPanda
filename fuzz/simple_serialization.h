#pragma once

#include <QVersionNumber>
#include <QString>
#include <QRectF>

class QDataStream;

namespace SimpleSerialization {
    QVersionNumber readPandaHeader(QDataStream &stream);
    QString loadDolphinFileName(QDataStream &stream, const QVersionNumber version);
    QRectF loadRect(QDataStream &stream, const QVersionNumber version);
    int testDeserialize(QDataStream &stream, const QVersionNumber version);
}