// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness that targets the IC blob deserializer directly.
 *
 * Two paths:
 *   1. Raw bytes: treat the fuzz input as the raw bytes of an IC blob and
 *      call IC::loadFromBlob() directly (raw mutation path).
 *   2. Structured (25% of inputs with first byte & 0xC0 == 0): use
 *      FuzzedDataProvider to build a structurally valid IC panda with 2-5
 *      boundary elements of every input and display type, nested in embedded
 *      ICs up to one level past the nesting limit.  This guarantees
 *      IC::processLoadedItems always sees loadable elements and exercises the
 *      port setup / boundary-element classification path and the embedded
 *      sub-circuit lookup even when raw mutation hasn't yet discovered them.
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
#include "Tests/Fuzz/FuzzSeed.h"
#include "Tests/Fuzz/PandaMutator.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// Boundary elements: IC::processLoadedItems makes one pin of each port of an Input or Output group element, and
// names the pins of an element with more than one port after its ports (InputRotary's, the displays').
constexpr ElementType kInputTypes[] = {ElementType::InputSwitch, ElementType::InputButton, ElementType::Clock,
                                       ElementType::InputRotary};
constexpr ElementType kOutputTypes[] = {ElementType::Led, ElementType::Display7, ElementType::Display14,
                                        ElementType::Display16};

// One level of a structurally valid IC blob (a panda file): 1-3 inputs and 1-2 outputs with fuzz-chosen types and
// labels and, when \a nestedBlob is not empty, an IC element whose sub-circuit is \a nestedBlob, embedded in the
// file's blob registry under its exact name or under its base name.
QByteArray buildICLevel(FuzzedDataProvider &fdp, const QByteArray &nestedBlob)
{
    const int numInputs  = fdp.ConsumeIntegralInRange<int>(1, 3);
    const int numOutputs = fdp.ConsumeIntegralInRange<int>(1, 2);

    QList<QGraphicsItem *> items;
    int nextId = 1;

    const auto addElement = [&](const ElementType type) {
        auto *elm = ElementFactory::buildElement(type);
        elm->setId(nextId++);
        if (elm->maxOutputSize() > elm->minOutputSize()) {
            elm->setOutputSize(fdp.ConsumeIntegralInRange<int>(elm->minOutputSize(), elm->maxOutputSize()));
        }
        if (fdp.ConsumeBool()) {
            elm->setLabel(fdp.ConsumeBool() ? QString("a") : QString("b"));
        }
        items.append(elm);
    };
    for (int i = 0; i < numInputs; ++i) {
        addElement(fdp.PickValueInArray(kInputTypes));
    }
    for (int i = 0; i < numOutputs; ++i) {
        addElement(fdp.PickValueInArray(kOutputTypes));
    }

    QMap<QString, QVariant> meta;
    meta.insert("dolphinFileName", QVariant(QString()));
    const bool byBaseName = fdp.ConsumeBool();
    if (!nestedBlob.isEmpty()) {
        Serialization::serializeBlobRegistry({{QString(byBaseName ? "nested" : "nested.panda"), nestedBlob}}, meta);
    }
    const QByteArray blob = FuzzSeed::pandaFile(meta, [&](QDataStream &stream) {
        Serialization::serialize(items, stream, {.purpose = SerializationPurpose::PortableFile});
        if (nestedBlob.isEmpty()) {
            return;
        }
        // IC element: discriminator + ElementType + base map + port lists + secondary map
        stream << static_cast<int>(GraphicElement::Type);
        stream << static_cast<quint64>(ElementType::IC);
        QMap<QString, QVariant> base;
        base.insert("pos", QVariant::fromValue(QPointF(0.0, 0.0)));
        base.insert("rotation", QVariant(0.0));
        base.insert("label", QVariant(QString()));
        stream << base;
        stream << quint32(0) << quint32(0) << quint32(0);
        QMap<QString, QVariant> icMap;
        icMap.insert("blobName", QVariant(QString("nested.panda")));
        stream << icMap;
    });
    qDeleteAll(items);

    return blob;
}

// A structurally valid IC blob nested 0-17 levels deep: loading one deeper than ICLoader's nesting limit (16)
// throws, so most draws stay shallow.
QByteArray buildStructuredICBlob(FuzzedDataProvider &fdp)
{
    const int depth = fdp.ConsumeBool() ? fdp.ConsumeIntegralInRange<int>(0, 2) : fdp.ConsumeIntegralInRange<int>(0, 17);

    QByteArray blob;
    for (int level = 0; level <= depth; ++level) {
        blob = buildICLevel(fdp, blob);
    }
    return blob;
}

} // namespace

// The raw path's input is a whole .panda file, the IC corpus's all in the current compressed format: the shared
// structure-aware mutator works on its decompressed body (Tests/Fuzz/PandaMutator.h). Inputs that take the
// structured path are not WPCF files and get its raw-byte operations.
extern "C" size_t LLVMFuzzerCustomMutator(uint8_t *Data, size_t Size, size_t MaxSize, unsigned int Seed)
{
    return PandaMutator::mutate(Data, Size, MaxSize, Seed);
}

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
