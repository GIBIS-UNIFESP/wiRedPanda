// Simple direct fuzzer using our working SimpleSerialization
#include <QCoreApplication>
#include <QDataStream>
#include <QByteArray>
#include <QIODevice>
#include <QVersionNumber>
#include <QDebug>

#include "simple_serialization.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    static bool initialized = false;
    if (!initialized) {
        static int argc = 1;
        static char arg0[] = "simple_direct_fuzzer";
        static char *argv[] = {arg0, nullptr};
        static QCoreApplication app(argc, argv);
        initialized = true;
    }

    if (size < 4) return 0;

    QByteArray byteArray(reinterpret_cast<const char*>(data), size);
    QDataStream stream(&byteArray, QIODevice::ReadOnly);
    
    try {
        // Direct deserialization test - this works in our debug tests
        int itemCount = SimpleSerialization::testDeserialize(stream, QVersionNumber(4, 2, 6));
        
        if (itemCount > 0) {
            qDebug() << "BREAKTHROUGH ACHIEVED! Found" << itemCount << "items";
            return 0; // Success - we found serialized items!
        }
        
        // Try different offsets
        for (size_t offset = 4; offset < size && offset < 32; offset += 4) {
            stream.device()->seek(offset);
            try {
                int offsetItems = SimpleSerialization::testDeserialize(stream, QVersionNumber(4, 2, 6));
                if (offsetItems > 0) {
                    qDebug() << "BREAKTHROUGH at offset" << offset << "! Found" << offsetItems << "items";
                    return 0;
                }
            } catch (...) {
                // Try next offset
            }
        }
        
    } catch (...) {
        // Expected for most inputs
    }
    
    return 0;
}