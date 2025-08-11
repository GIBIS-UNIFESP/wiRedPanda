// Direct ElementFactory fuzzer - targets the real serialization.cpp ElementFactory::buildElement()
#include <QCoreApplication>
#include <QDataStream>
#include <QByteArray>
#include <QIODevice>
#include <QVersionNumber>
#include <QDebug>
#include <QMap>
#include <QGraphicsItem>

#include "app/serialization.h"
#include "app/common.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    static bool initialized = false;
    if (!initialized) {
        static int argc = 1;
        static char arg0[] = "elementfactory_direct_fuzzer";
        static char *argv[] = {arg0, nullptr};
        static QCoreApplication app(argc, argv);
        initialized = true;
    }

    if (size < 8) return 0; // Need minimum data for type + elementtype

    QByteArray byteArray(reinterpret_cast<const char*>(data), size);
    QDataStream stream(&byteArray, QIODevice::ReadOnly);
    
    try {
        // STRATEGY 1: Direct call to Serialization::deserialize() with valid WPCF header
        // This should reach ElementFactory::buildElement() in the actual serialization.cpp
        
        // Try to create a minimal valid header that readPandaHeader() will accept
        stream.device()->seek(0);
        
        // Check if we can read this as a WPCF file
        quint32 possibleMagic;
        stream >> possibleMagic;
        
        if (possibleMagic == 0x46434057) { // "WPCF" magic
            // Reset and try full deserialization
            stream.device()->seek(0);
            
            QVersionNumber version;
            try {
                version = Serialization::readPandaHeader(stream);
                qDebug() << "Successfully read WPCF header, version:" << version.toString();
                
                // Now deserialize - this should reach ElementFactory::buildElement()
                QMap<quint64, QNEPort *> portMap;
                auto items = Serialization::deserialize(stream, portMap, version);
                
                if (!items.isEmpty()) {
                    qDebug() << "MAJOR BREAKTHROUGH! ElementFactory created" << items.size() << "items";
                    
                    // Process items to ensure full coverage
                    for (auto *item : items) {
                        if (item) {
                            // Touch the item to ensure coverage
                            item->type(); // This should hit GraphicsElement or QNEConnection methods
                        }
                    }
                    return 0; // Success!
                }
            } catch (const std::exception& e) {
                qDebug() << "WPCF deserialization failed:" << e.what();
            }
        }
        
        // STRATEGY 2: Try to create fake WPCF header + serialization data
        stream.device()->seek(0);
        
        // If data is large enough, try to interpret it as having a constructed WPCF header
        if (size >= 20) {
            QByteArray fakeHeader;
            QDataStream headerStream(&fakeHeader, QIODevice::WriteOnly);
            
            // Write WPCF header
            headerStream << quint32(0x46434057); // "WPCF" magic
            headerStream << qint32(1);           // 1 version segment
            headerStream << qint32(4);           // Version 4
            
            // Append the fuzz data as serialization content
            fakeHeader.append(byteArray);
            
            // Try to deserialize this constructed data
            QDataStream fakeStream(&fakeHeader, QIODevice::ReadOnly);
            
            try {
                auto version = Serialization::readPandaHeader(fakeStream);
                qDebug() << "Fake header accepted, version:" << version.toString();
                
                QMap<quint64, QNEPort *> portMap;
                auto items = Serialization::deserialize(fakeStream, portMap, version);
                
                if (!items.isEmpty()) {
                    qDebug() << "FAKE HEADER BREAKTHROUGH! Created" << items.size() << "items";
                    
                    for (auto *item : items) {
                        if (item) {
                            item->type();
                        }
                    }
                    return 0;
                }
            } catch (const std::exception& e) {
                qDebug() << "Fake header deserialization failed:" << e.what();
            }
        }
        
        // STRATEGY 3: Try different offsets with constructed headers
        for (size_t offset = 0; offset < size && offset < 32; offset += 4) {
            if (offset + 8 > size) break;
            
            QByteArray offsetHeader;
            QDataStream offsetStream(&offsetHeader, QIODevice::WriteOnly);
            
            // Write WPCF header
            offsetStream << quint32(0x46434057); // "WPCF" magic  
            offsetStream << qint32(2);           // 2 version segments
            offsetStream << qint32(4);           // Version 4
            offsetStream << qint32(2);           // Version 4.2
            
            // Append data from offset
            offsetHeader.append(byteArray.mid(offset));
            
            QDataStream testStream(&offsetHeader, QIODevice::ReadOnly);
            
            try {
                auto version = Serialization::readPandaHeader(testStream);
                QMap<quint64, QNEPort *> portMap;
                auto items = Serialization::deserialize(testStream, portMap, version);
                
                if (!items.isEmpty()) {
                    qDebug() << "OFFSET BREAKTHROUGH at" << offset << "! Created" << items.size() << "items";
                    
                    for (auto *item : items) {
                        if (item) {
                            item->type();
                        }
                    }
                    return 0;
                }
            } catch (...) {
                // Continue trying
            }
        }
        
        // STRATEGY 4: Raw type recognition fallback (like our working approach)
        stream.device()->seek(0);
        
        while (!stream.atEnd()) {
            int type;
            stream >> type;
            
            if (stream.status() != QDataStream::Ok) break;
            
            const int GRAPHICELEMENT_TYPE = 65539;
            const int QNECONNECTION_TYPE = 65538;
            
            if (type == GRAPHICELEMENT_TYPE || type == QNECONNECTION_TYPE) {
                qDebug() << "Found raw type" << type << "- attempting direct ElementFactory call";
                
                // Try to construct minimal serialization context and call ElementFactory directly
                QByteArray elementData;
                QDataStream elementStream(&elementData, QIODevice::WriteOnly);
                
                // Write the type we found
                elementStream << type;
                
                // Add minimal element data based on our working pattern
                if (type == GRAPHICELEMENT_TYPE) {
                    elementStream << qint32(5);      // ElementType: AND gate
                    elementStream << quint64(1);     // ID
                    elementStream << float(100.0);   // x
                    elementStream << float(200.0);   // y  
                    elementStream << float(0.0);     // rotation
                    elementStream << qint32(2);      // input count
                    elementStream << qint32(1);      // output count
                    elementStream << qint32(0);      // empty label
                }
                
                // Try to deserialize this constructed element
                QDataStream readElementStream(&elementData, QIODevice::ReadOnly);
                QMap<quint64, QNEPort *> portMap;
                
                try {
                    auto items = Serialization::deserialize(readElementStream, portMap, QVersionNumber(4, 2, 6));
                    
                    if (!items.isEmpty()) {
                        qDebug() << "RAW TYPE BREAKTHROUGH! ElementFactory created items";
                        
                        for (auto *item : items) {
                            if (item) {
                                item->type();
                            }
                        }
                        return 0;
                    }
                } catch (...) {
                    // Continue
                }
                
                break; // Only try the first valid type we find
            }
        }
        
    } catch (...) {
        // Expected for most inputs
    }
    
    return 0;
}