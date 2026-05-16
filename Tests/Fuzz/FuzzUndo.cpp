// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness targeting the undo/redo command system (Commands.cpp).
 *
 * Commands.cpp (1190 lines) is entirely dark in all other harnesses because
 * every command is created and pushed by UI handlers that are never exercised
 * headlessly.  This harness:
 *
 *   1. Loads a known-valid pre-built workspace (AND gate + InputSwitch + Led,
 *      built in LLVMFuzzerInitialize) so that commands always have elements to
 *      operate on.  The entire fuzz input is used as the command FDP.
 *
 *   2. Also attempts to load a fuzz-controlled panda stream (first half of input)
 *      so that commands can operate on arbitrary element combinations.
 *
 *   3. Uses FuzzedDataProvider to apply a sequence of random undo/redo commands:
 *        - AddItemsCommand       — add a fresh element
 *        - DeleteItemsCommand    — remove an existing element
 *        - RotateCommand         — rotate elements by ±90°
 *        - MoveCommand           — move elements to new positions
 *        - FlipCommand           — flip elements along x or y axis
 *        - MorphCommand          — change element type in-place
 *        - ChangePortSizeCommand — resize input/output port count
 *        - RegisterBlobCommand   — register an IC blob
 *        - RemoveBlobCommand     — remove an IC blob
 *
 *   4. Undo and redo each command, verifying the scene survives both directions.
 *   5. Winds the entire stack back to the initial state and forward again.
 */

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QScopeGuard>
#include <QString>
#include <QUndoStack>
#include <QVariant>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// Valid IC blob (InputSwitch + Led) for RegisterBlobCommand tests.
QByteArray g_icBlob;

// Pre-built workspace bytes: And gate + two InputSwitches + Led.
// Reloaded at the start of each test so commands always have elements to work on.
QByteArray g_validPanda;

// Element types that are safe to morph to (stable port count, no IC dependency)
constexpr ElementType kMorphTargets[] = {
    ElementType::And, ElementType::Or, ElementType::Not,
    ElementType::Nand, ElementType::Nor, ElementType::Xor,
};

// Element types safe to add in AddItemsCommand.
// TruthTable is excluded: its 2048-bit key causes expensive multi-second
// serialization when UpdateCommand captures all scene elements.
constexpr ElementType kAddTypes[] = {
    ElementType::And,
    ElementType::Or,
    ElementType::InputSwitch,
    ElementType::Led,
    ElementType::Not,
    ElementType::Clock,
};

void applyRandomCommands(Scene *scene, FuzzedDataProvider &fdp)
{
    QUndoStack stack;

    // Grab the first connection already in the scene (came from g_validPanda or fuzz load).
    // SplitCommand needs this; no per-run scene::addItem is needed since the connection
    // was serialized into g_validPanda at initialization time.
    QNEConnection *splitableConn = nullptr;
    for (auto *item : scene->items()) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            splitableConn = conn;
            break;
        }
    }

    const int numOps = fdp.ConsumeIntegralInRange<int>(1, 6);
    for (int op = 0; op < numOps; ++op) {
        const auto elems = scene->elements();

        // After all existing elements have been deleted, only AddItemsCommand makes sense
        if (elems.isEmpty() && op > 0) break;

        const int action = fdp.ConsumeIntegralInRange<int>(0, 11);

        switch (action) {
        case 0: { // AddItemsCommand
            const auto type = kAddTypes[fdp.ConsumeIntegralInRange<size_t>(
                0, std::size(kAddTypes) - 1)];
            auto *elm = ElementFactory::buildElement(type);
            try {
                stack.push(new AddItemsCommand({elm}, scene));
            } catch (...) {
                delete elm;
            }
            break;
        }
        case 1: { // DeleteItemsCommand
            if (elems.isEmpty()) break;
            const int idx = fdp.ConsumeIntegralInRange<int>(
                0, static_cast<int>(elems.size()) - 1);
            try {
                stack.push(new DeleteItemsCommand(
                    {static_cast<QGraphicsItem *>(elems.at(idx))}, scene));
            } catch (...) {}
            break;
        }
        case 2: { // RotateCommand
            if (elems.isEmpty()) break;
            const int angle = fdp.ConsumeBool() ? 90 : -90;
            try {
                stack.push(new RotateCommand(elems, angle, scene));
            } catch (...) {}
            break;
        }
        case 3: { // MoveCommand
            if (elems.isEmpty()) break;
            QList<QPointF> oldPositions;
            for (auto *e : elems) {
                oldPositions << e->pos();
            }
            try {
                stack.push(new MoveCommand(elems, oldPositions, scene));
            } catch (...) {}
            break;
        }
        case 4: { // FlipCommand (axis 0 = horizontal, 1 = vertical)
            if (elems.isEmpty()) break;
            const int axis = fdp.ConsumeBool() ? 1 : 0;
            try {
                stack.push(new FlipCommand(elems, axis, scene));
            } catch (...) {}
            break;
        }
        case 5: { // MorphCommand — changes element type in-place
            if (elems.isEmpty()) break;
            const int idx = fdp.ConsumeIntegralInRange<int>(
                0, static_cast<int>(elems.size()) - 1);
            const auto newType = kMorphTargets[fdp.ConsumeIntegralInRange<size_t>(
                0, std::size(kMorphTargets) - 1)];
            try {
                stack.push(new MorphCommand({elems.at(idx)}, newType, scene));
            } catch (...) {}
            break;
        }
        case 6: { // ChangePortSizeCommand
            if (elems.isEmpty()) break;
            const int idx = fdp.ConsumeIntegralInRange<int>(
                0, static_cast<int>(elems.size()) - 1);
            const int newSize = fdp.ConsumeIntegralInRange<int>(1, 4);
            const bool isInput = fdp.ConsumeBool();
            try {
                stack.push(new ChangePortSizeCommand({elems.at(idx)}, newSize, scene, isInput));
            } catch (...) {}
            break;
        }
        case 7: { // RegisterBlobCommand
            const QString blobName = QString("fuzz_blob_%1.panda").arg(op);
            try {
                stack.push(new RegisterBlobCommand(blobName, g_icBlob, scene));
            } catch (...) {}
            break;
        }
        case 8: { // RemoveBlobCommand
            const QString blobName = QString("fuzz_blob_%1.panda").arg(op > 0 ? op - 1 : 0);
            try {
                stack.push(new RemoveBlobCommand(blobName, scene));
            } catch (...) {}
            break;
        }
        case 9: { // UpdateCommand — captures current element state as "old data", then
            // creates an UpdateCommand.  Since new == old, undo/redo restores the same
            // state, but all of UpdateCommand::undo/redo serialisation paths are hit.
            //
            // IMPORTANT: must use the same wire format as UpdateCommand::loadData, which
            // reads header + raw elm->save() per element (NO type discriminators).
            // CommandUtils::saveItems writes full Serialization::serialize format (WITH
            // discriminators) which causes format desync on undo.
            if (elems.isEmpty()) break;
            QByteArray oldData;
            {
                QDataStream s(&oldData, QIODevice::WriteOnly);
                Serialization::writePandaHeader(s);
                for (auto *e : elems) e->save(s);
            }
            try {
                stack.push(new UpdateCommand(elems, oldData, scene));
            } catch (...) {}
            break;
        }
        case 10: { // ToggleTruthTableOutputCommand — requires a TruthTable element
            TruthTable *tt = nullptr;
            for (auto *e : elems) {
                if (e->elementType() == ElementType::TruthTable) {
                    tt = static_cast<TruthTable *>(e);
                    break;
                }
            }
            if (!tt) break;
            const int pos = fdp.ConsumeIntegralInRange<int>(0, 3);
            try {
                stack.push(new ToggleTruthTableOutputCommand(tt, pos, scene));
            } catch (...) {}
            break;
        }
        case 11: { // SplitCommand — inserts a Node junction into a connection
            // Use the pre-wired connection created at the start; if it was consumed
            // by a previous undo or deleted, we skip gracefully.
            if (!splitableConn || !splitableConn->scene()) break;
            const QPointF midPos(
                fdp.ConsumeFloatingPointInRange<double>(-200.0, 200.0),
                fdp.ConsumeFloatingPointInRange<double>(-200.0, 200.0));
            try {
                stack.push(new SplitCommand(splitableConn, midPos, scene));
                splitableConn = nullptr; // consumed — don't split again
            } catch (...) {}
            break;
        }
        default:
            break;
        }

        // Randomly undo/redo after each push
        if (fdp.ConsumeBool() && stack.canUndo()) {
            try { stack.undo(); } catch (...) {}
        }
        if (fdp.ConsumeBool() && stack.canRedo()) {
            try { stack.redo(); } catch (...) {}
        }
    }

    // Fully unwind the stack and replay forward to exercise all undo/redo paths.
    // Track the index before/after each undo to break if the stack index doesn't
    // change (indicates a broken command that swallows exceptions and doesn't advance).
    int prevIdx = stack.index();
    while (stack.canUndo()) {
        try { stack.undo(); } catch (...) {}
        if (stack.index() == prevIdx) break; // stuck: undo didn't advance index
        prevIdx = stack.index();
    }
    prevIdx = stack.index();
    while (stack.canRedo()) {
        try { stack.redo(); } catch (...) {}
        if (stack.index() == prevIdx) break;
        prevIdx = stack.index();
    }
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

    // Build a valid IC blob for RegisterBlobCommand tests.
    {
        auto *sw  = ElementFactory::buildElement(ElementType::InputSwitch);
        auto *led = ElementFactory::buildElement(ElementType::Led);
        sw->setId(1);
        led->setId(2);
        QList<QGraphicsItem *> items;
        items.append(sw);
        items.append(led);

        g_icBlob.clear();
        QDataStream s(&g_icBlob, QIODevice::WriteOnly);
        Serialization::writePandaHeader(s);
        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        s << meta;
        Serialization::serialize(items, s);
        qDeleteAll(items);
    }

    // Build g_validPanda: a wired circuit with And gate + two InputSwitches + Led.
    // Includes a real connection (sw1 output → And input0) so SplitCommand can
    // split it without requiring a per-run scene::addItem call.
    // The connection is serialized alongside elements so it's in the scene on load.
    {
        auto *andG = ElementFactory::buildElement(ElementType::And);
        auto *sw1  = ElementFactory::buildElement(ElementType::InputSwitch);
        auto *sw2  = ElementFactory::buildElement(ElementType::InputSwitch);
        auto *led  = ElementFactory::buildElement(ElementType::Led);
        andG->setId(1);  sw1->setId(2);  sw2->setId(3);  led->setId(4);

        QList<QGraphicsItem *> items;
        items.append(andG);  items.append(sw1);
        items.append(sw2);   items.append(led);

        g_validPanda.clear();
        QDataStream s(&g_validPanda, QIODevice::WriteOnly);
        Serialization::writePandaHeader(s);
        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        s << meta;
        Serialization::serialize(items, s);
        qDeleteAll(items);

        // Append one QNEConnection (sw1 output → And input0) as raw wire bytes.
        // Writing it after serialize() ensures the element port serialIds are already
        // in the stream (portMap is populated by elements before connections are wired).
        //
        // Port serialId formula (from GraphicElement::save):
        //   serialId = (element_id << 16) | portIndex
        // sw1 (id=2) output0: (2<<16)|0 = 131072
        // andG (id=1) input0:  (1<<16)|0 = 65536
        {
            constexpr quint64 sw1Out0 = (quint64(2) << 16) | 0;  // sw1 output port 0
            constexpr quint64 andIn0  = (quint64(1) << 16) | 0;  // andG input port 0
            s << static_cast<int>(QGraphicsItem::UserType + 2);   // QNEConnection::Type
            QMap<QString, QVariant> connMap;
            connMap.insert("startPortId", QVariant::fromValue(sw1Out0));
            connMap.insert("endPortId",   QVariant::fromValue(andIn0));
            s << connMap;
        }
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

    if (size < 2) return 0;

    // Use the first byte to decide scene source:
    //   0–127: use pre-built g_validPanda (guarantees elements to operate on)
    //   128+:  try to load fuzz input as panda (exercises diverse scene states)
    const bool usePrebuild = (data[0] < 128);

    // Use the entire fuzz input as the command FDP (after panda portion is consumed).
    // For the pre-built path, the full input drives commands.
    // For the fuzz-load path, the first half drives the panda, second half drives commands.
    const size_t pandaSize = usePrebuild ? 0 : std::min<size_t>(size - 1, size / 2);
    const size_t cmdStart  = pandaSize;
    const size_t cmdSize   = size - cmdStart;

    WorkSpace ws;

    if (usePrebuild) {
        // Load pre-built workspace (guaranteed to have 4 elements)
        QDataStream stream(g_validPanda);
        QVersionNumber ver;
        try {
            ver = Serialization::readPandaHeader(stream);
            ws.load(stream, ver, /*contextDir=*/QString());
        } catch (...) {
            return 0; // should never happen with a valid pre-built panda
        }
    } else {
        // Load fuzz-controlled panda
        const QByteArray buffer = QByteArray::fromRawData(
            reinterpret_cast<const char *>(data + 1), static_cast<int>(pandaSize));
        QDataStream stream(buffer);
        QVersionNumber version;
        try {
            version = Serialization::readPandaHeader(stream);
        } catch (...) {
            return 0;
        }
        try {
            ws.load(stream, version, /*contextDir=*/QString());
        } catch (...) {}
    }

    if (cmdSize == 0) return 0;
    FuzzedDataProvider fdp(data + cmdStart, cmdSize);
    applyRandomCommands(ws.scene(), fdp);

    return 0;
}
