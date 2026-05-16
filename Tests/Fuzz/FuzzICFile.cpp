// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness for file-backed IC loading (IC::loadFromFile path).
 *
 * fuzz_ic_blob exercises IC::loadFromBlob (embedded blob path).
 * The file-backed path — IC::loadFromFile → IC::loadFileDirectly — is completely
 * separate and dark: it opens a QFile, reads a panda stream, handles cycle
 * detection, and calls IC::migrateFile on old-format sub-circuits.
 *
 * This harness writes the fuzz bytes to a QTemporaryFile, then loads a
 * workspace panda stream whose IC element's `name` property points to that
 * temp file (relative to its directory).  When the IC element is loaded and
 * the blob registry has no matching entry, the loader falls through to
 * IC::loadFromFile, which exercises the file-backed path end-to-end.
 */

#include <cstddef>
#include <cstdint>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QEvent>
#include <QIODevice>
#include <QMap>
#include <QPointF>
#include <QScopeGuard>
#include <QString>
#include <QTemporaryFile>
#include <QVariant>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Workspace.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// Valid IC panda blob at current version (v5.1) — exercises IC::loadFileDirectly success path.
QByteArray g_validICBytes;

// Build an outer panda file referencing an IC by filename (not blob name).
// When loaded, the IC element will have no matching blob in the registry,
// causing it to fall through to IC::loadFromFile(contextDir).
QByteArray buildOuterPanda(const QString &icFileName)
{
    QByteArray buf;
    QDataStream s(&buf, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_5_12);

    // v5.1 header
    s.writeRawData("WPCF", 4);
    s << QList<int>{5, 1};

    // Metadata: empty (no embeddedICs → IC must load from file)
    QMap<QString, QVariant> meta;
    meta.insert("dolphinFileName", QVariant(QString()));
    s << meta;

    // One IC element whose file name references the temp file (no blob registry entry
    // → IC falls through to IC::loadFromFile which opens the temp file).
    //
    // Correct format (NO count header — Serialization::deserialize uses while(!atEnd)):
    //   int  GraphicElement::Type = 65539
    //   u64  ElementType::IC = 22
    //   [1] base map (pos, rotation, label)
    //   [2] port lists × 3 (each: quint32 count=0)
    //   [3] IC secondary map with "name" key → triggers file-backed load path
    s << static_cast<int>(65539);              // GraphicElement::Type discriminator
    s << static_cast<quint64>(22);             // ElementType::IC

    // [1] Base map (GraphicElement::loadNewFormat reads pos/rotation/label/etc.)
    QMap<QString, QVariant> baseProps;
    baseProps.insert("pos",      QVariant::fromValue(QPointF(100.0, 100.0)));
    baseProps.insert("rotation", QVariant(0.0));
    baseProps.insert("label",    QVariant(QString()));
    s << baseProps;

    // [2] Port lists (empty)
    s << quint32(0) << quint32(0) << quint32(0);

    // [3] IC secondary map — IC::load reads this after the base load()
    // "name" key with the file's basename triggers IC::loadFromFile(contextDir)
    QMap<QString, QVariant> icProps;
    icProps.insert("name", QVariant(icFileName));
    s << icProps;

    return buf;
}

} // namespace

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_IM_MODULES", "none");

    Comment::setVerbosity(-1);
    Application::interactiveMode = false;
    Application::migrationEnabled = false;

    g_argc = *argc;
    g_argv = *argv;
    g_app  = new QApplication(g_argc, g_argv);

    // Build a valid IC blob (InputSwitch + Led) for the success path of
    // IC::loadFileDirectly → processLoadedItems → loadBoundaryElement.
    {
        auto *sw  = ElementFactory::buildElement(ElementType::InputSwitch);
        auto *led = ElementFactory::buildElement(ElementType::Led);
        sw->setId(1);  led->setId(2);
        QList<QGraphicsItem *> items;
        items.append(sw);  items.append(led);

        g_validICBytes.clear();
        QDataStream s(&g_validICBytes, QIODevice::WriteOnly);
        Serialization::writePandaHeader(s);
        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        s << meta;
        Serialization::serialize(items, s);
        qDeleteAll(items);
    }

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Drain Qt's deferred-delete queue on every exit path.  Audio elements
    // (Buzzer, AudioBox) queue DeferredDelete events for the audio backend's
    // internal QObject helpers when destroyed; without spinning the event
    // loop, those helpers stay alive and hold PipeWire stream FDs open
    // across iterations, exhausting the audio subsystem after ~1000 inputs.
    auto drainQtEvents = qScopeGuard([] {
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
    });

    if (size == 0) return 0;

    // 25% of inputs (top 2 bits of first byte both 0) use a valid IC blob so
    // IC::loadFileDirectly succeeds and exercises processLoadedItems.
    // The other 75% use raw fuzz bytes (error-path coverage).
    const QByteArray fileContent = ((data[0] & 0xC0) == 0)
        ? g_validICBytes
        : QByteArray(reinterpret_cast<const char *>(data), static_cast<int>(size));

    QTemporaryFile tmp;
    tmp.setAutoRemove(true);
    if (!tmp.open()) return 0;
    tmp.write(fileContent);
    tmp.flush();

    const QString icFileName = QDir(tmp.fileName()).dirName(); // basename only
    const QString contextDir = QFileInfo(tmp.fileName()).absolutePath();

    // Build and load the outer panda that references the temp file.
    const QByteArray outer = buildOuterPanda(QFileInfo(tmp.fileName()).fileName());
    QDataStream stream(outer);

    QVersionNumber version;
    try {
        version = Serialization::readPandaHeader(stream);
    } catch (...) {
        return 0;
    }

    WorkSpace ws;
    try {
        ws.load(stream, version, contextDir); // contextDir → IC::loadFromFile uses it
    } catch (...) {}

    return 0;
}
