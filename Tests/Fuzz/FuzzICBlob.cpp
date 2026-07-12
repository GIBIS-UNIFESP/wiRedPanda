// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness that targets the IC blob deserializer directly.
 *
 * Two paths:
 *   1. Raw bytes: treat the fuzz input as the raw bytes of an IC blob and
 *      call IC::loadFromBlob() directly (raw mutation path).
 *   2. Structured (25% of inputs with first byte & 0xC0 == 0): use
 *      FuzzedDataProvider to build a structurally valid IC panda with 1-4
 *      boundary elements (InputSwitch as inputs, Led as outputs).  This
 *      guarantees IC::processLoadedItems always sees loadable elements and
 *      exercises the port setup / boundary-element classification path even
 *      when raw mutation hasn't yet discovered the panda format.
 */

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QIODevice>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QScopeGuard>
#include <QString>
#include <QVariant>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// Build a structurally valid IC blob (panda file) with fuzz-controlled
// element properties.  Boundary elements (InputSwitch, Led) are classified
// as IC inputs/outputs by IC::processLoadedItems; the harness exercises the
// full boundary element setup path even on structured (non-mutated) inputs.
QByteArray buildStructuredICBlob(FuzzedDataProvider &fdp)
{
    // Pick 1-3 InputSwitch (inputs) and 1-2 Led (outputs)
    const int numInputs  = fdp.ConsumeIntegralInRange<int>(1, 3);
    const int numOutputs = fdp.ConsumeIntegralInRange<int>(1, 2);

    QList<QGraphicsItem *> items;
    int nextId = 1;

    for (int i = 0; i < numInputs; ++i) {
        auto *elm = ElementFactory::buildElement(ElementType::InputSwitch);
        elm->setId(nextId++);
        items.append(elm);
    }
    for (int i = 0; i < numOutputs; ++i) {
        auto *elm = ElementFactory::buildElement(ElementType::Led);
        elm->setId(nextId++);
        items.append(elm);
    }

    QByteArray buf;
    QDataStream s(&buf, QIODevice::WriteOnly);
    Serialization::writePandaHeader(s);

    QMap<QString, QVariant> meta;
    meta.insert("dolphinFileName", QVariant(QString()));
    s << meta;

    Serialization::serialize(items, s, {.purpose = SerializationPurpose::PortableFile});
    qDeleteAll(items);

    return buf;
}

} // namespace

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_IM_MODULES", "none");

    Comment::setVerbosity(-1);
    Application::interactiveMode = false;
    Application::renderingEnabled = false;
    Application::migrationEnabled = false;

    g_argc = *argc;
    g_argv = *argv;
    g_app  = new QApplication(g_argc, g_argv);

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

    // Use 25% of inputs for the structured path (when top 2 bits of first byte
    // are both 0).  The other 75% use raw bytes so mutations of real IC files
    // from the seed corpus are still exercised.
    QByteArray bytes;
    if ((data[0] & 0xC0) == 0 && size >= 4) {
        FuzzedDataProvider fdp(data + 1, size - 1);
        bytes = buildStructuredICBlob(fdp);
    } else {
        bytes = QByteArray(reinterpret_cast<const char *>(data),
                           static_cast<int>(size));
    }

    // IC::~IC calls qDeleteAll(m_internalElements / m_internalConnections),
    // so all items created by processLoadedItems are owned and freed here.
    auto *ic = static_cast<IC *>(ElementFactory::buildElement(ElementType::IC));
    try {
        ic->loadFromBlob(bytes, /*contextDir=*/QString());
    } catch (...) {}
    delete ic;
    return 0;
}
