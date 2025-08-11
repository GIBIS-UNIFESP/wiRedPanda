// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "common.h"
#include "elementfactory.h"
#include "qneport.h"
#include "registertypes.h"
#include "serialization.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QMap>
#include <memory>

static bool g_initialized = false;
static Application *g_app = nullptr;

static void initializeQt() {
    if (!g_initialized) {
        // Initialize Qt application context (required for Qt objects)
        static int argc = 1;
        static char name[] = "fuzz_serialization";
        static char *argv[] = {name, nullptr};
        
        g_app = new Application(argc, argv);
        g_app->setOrganizationName("GIBIS-UNIFESP");
        g_app->setApplicationName("wiRedPanda");
        g_app->setApplicationVersion(APP_VERSION);
        
        // Register Qt types
        registerTypes();
        
        // Suppress Qt logging to reduce noise during fuzzing
        Comment::setVerbosity(-1);
        
        g_initialized = true;
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    initializeQt();
    
    // Skip inputs that are too small to be meaningful
    if (size < 8) {
        return 0;
    }
    
    // Create QDataStream from fuzz input
    QByteArray byteArray(reinterpret_cast<const char*>(data), size);
    QDataStream stream(&byteArray, QIODevice::ReadOnly);
    
    try {
        // Test 1: Header parsing
        QVersionNumber version;
        try {
            version = Serialization::readPandaHeader(stream);
        } catch (const std::exception&) {
            // Header parsing failed, which is expected for malformed input
            return 0;
        } catch (...) {
            // Catch any other exceptions
            return 0;
        }
        
        // Reset stream position to beginning for full deserialization
        stream.device()->seek(0);
        
        // Test 2: Full deserialization
        QMap<quint64, QNEPort *> portMap;
        QList<QGraphicsItem *> items;
        
        try {
            items = Serialization::deserialize(stream, portMap, version);
        } catch (const std::exception&) {
            // Deserialization failed, which is expected for malformed input
            return 0;
        } catch (...) {
            // Catch any other exceptions
            return 0;
        }
        
        // Test 3: Canvas rectangle loading (if stream has more data)
        if (!stream.atEnd()) {
            try {
                stream.device()->seek(0);
                Serialization::loadRect(stream, version);
            } catch (...) {
                // Expected for malformed input
            }
        }
        
        // Test 4: Dolphin filename loading (if stream has more data)
        if (!stream.atEnd()) {
            try {
                stream.device()->seek(0);
                Serialization::loadDolphinFileName(stream, version);
            } catch (...) {
                // Expected for malformed input
            }
        }
        
        // Clean up Qt objects to prevent memory leaks
        qDeleteAll(items);
        
        // Clean up ports from the map
        for (auto *port : portMap) {
            if (port) {
                delete port;
            }
        }
        
    } catch (const std::exception&) {
        // Expected for malformed input
    } catch (...) {
        // Catch any other exceptions to prevent crashes
    }
    
    return 0;
}

// Optional: Custom initialization for libFuzzer
extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv) {
    (void)argc;
    (void)argv;
    
    // Initialize Qt context once
    initializeQt();
    
    return 0;
}