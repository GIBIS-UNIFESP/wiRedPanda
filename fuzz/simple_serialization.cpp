#include "simple_serialization.h"
#include <QDataStream>
#include <QVersionNumber>
#include <QString>
#include <QDebug>

// Simple versions of serialization functions for fuzzing
namespace SimpleSerialization {

QVersionNumber readPandaHeader(QDataStream &stream) {
    stream.setVersion(QDataStream::Qt_5_12);

    qint64 originalPos = stream.device()->pos();
    quint32 magicHeader;
    stream >> magicHeader;

    if (stream.status() != QDataStream::Ok) {
        throw std::runtime_error("Stream error while reading file header.");
    }

    QVersionNumber version;
    const quint32 MAGIC_HEADER_CIRCUIT = 0x46434057; // "WPCF"

    if (magicHeader == MAGIC_HEADER_CIRCUIT) {
        // Read version safely
        qint32 segmentCount;
        stream >> segmentCount;
        
        if (segmentCount < 0 || segmentCount > 10) {
            throw std::runtime_error("Invalid version segment count.");
        }
        
        QVector<int> segments;
        segments.reserve(segmentCount);
        
        for (qint32 i = 0; i < segmentCount; ++i) {
            qint32 segment;
            stream >> segment;
            segments.append(segment);
        }
        
        version = QVersionNumber(segments);
    } else {
        // Legacy format - try to read as QString
        stream.device()->seek(originalPos);
        
        qint32 length;
        stream >> length;
        
        if (length == 0) {
            // Empty string - copy/paste stream mode
            stream.device()->seek(originalPos);
            
            float x, y;
            stream >> x >> y;  // Read as QPointF
            
            if (x == 0.0f && y == 0.0f) {
                throw std::runtime_error("Invalid file format.");
            }
            
            stream.device()->seek(originalPos);
            version = QVersionNumber(4, 1); // Assume version 4.1
        } else if (length > 0 && length < 256) {
            // Try to read as string
            QString appName;
            appName.reserve(length);
            
            for (qint32 i = 0; i < length; ++i) {
                quint16 ch;
                stream >> ch;
                appName.append(QChar(ch));
            }
            
            if (appName.startsWith("wiRedPanda", Qt::CaseInsensitive)) {
                QStringList split = appName.split(" ");
                if (split.size() >= 2) {
                    version = QVersionNumber::fromString(split.at(1));
                }
            }
            
            if (version.isNull()) {
                throw std::runtime_error("Invalid file format.");
            }
        } else {
            throw std::runtime_error("String length out of bounds.");
        }
    }

    return version.normalized();
}

QString loadDolphinFileName(QDataStream &stream, const QVersionNumber version) {
    QString filename;
    if (version >= QVersionNumber(3, 0)) {
        stream >> filename;
        if (version < QVersionNumber(3, 3) && filename == "none") {
            filename.clear();
        }
    }
    return filename;
}

QRectF loadRect(QDataStream &stream, const QVersionNumber version) {
    QRectF rect;
    if (version >= QVersionNumber(1, 4)) {
        stream >> rect;
    }
    return rect;
}

// Simplified deserialize that just checks for the right type values
int testDeserialize(QDataStream &stream, const QVersionNumber version) {
    int itemCount = 0;
    const int GRAPHICELEMENT_TYPE = 65539;
    const int QNECONNECTION_TYPE = 65538;
    
    while (!stream.atEnd() && itemCount < 100) { // Limit to prevent infinite loops
        int type;
        stream >> type;
        
        qDebug() << "Read type:" << type << "expected GE:" << GRAPHICELEMENT_TYPE << "expected Conn:" << QNECONNECTION_TYPE;
        
        if (type == GRAPHICELEMENT_TYPE) {
            // SUCCESS! We reached the GraphicElement case
            qDebug() << "SUCCESS: Found GraphicElement::Type!";
            
            int elementType;
            stream >> elementType;
            qDebug() << "ElementType:" << elementType;
            
            // Skip rest of element data to continue parsing
            stream.device()->seek(stream.device()->pos() + 32); // Skip some bytes
            itemCount++;
            
        } else if (type == QNECONNECTION_TYPE) {
            // SUCCESS! We reached the QNEConnection case  
            qDebug() << "SUCCESS: Found QNEConnection::Type!";
            
            // Skip connection data
            stream.device()->seek(stream.device()->pos() + 16); // Skip some bytes
            itemCount++;
            
        } else {
            // Unknown type - this is the barrier we've been hitting
            qDebug() << "Unknown type:" << type << "(0x" << Qt::hex << type << ")";
            throw std::runtime_error("Invalid type. Data is possibly corrupted.");
        }
    }
    
    return itemCount;
}

} // namespace SimpleSerialization