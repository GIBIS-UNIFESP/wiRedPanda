// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness targeting the undo/redo command system (Commands.cpp).
 *
 * Every command is created and pushed by a UI handler that no other harness drives. This harness loads a circuit,
 * pushes a random sequence of commands onto its scene's undo stack, as the editor does, undoing and redoing between
 * them, then winds the stack fully back and forward again:
 *
 *   - AddItemsCommand          — add an element of a type, a file-backed IC, or an embedded IC (createEmbeddedIC)
 *   - DeleteItemsCommand       — remove an element
 *   - RotateCommand, FlipCommand
 *   - MoveCommand              — move elements, as a drag does
 *   - MorphCommand             — change elements' type in place
 *   - ChangePortSizeCommand    — resize the input or output port count
 *   - UpdateCommand            — an element's state as captured before an edit
 *   - SetPropertyCommand       — set a property of one element or all, as the element editor does, in or out of an
 *                                edit session (merging)
 *   - ToggleTruthTableOutputCommand (on a truth table added first when the circuit holds none), SplitCommand
 *   - RegisterBlobCommand, RemoveBlobCommand, RenameBlobCommand
 *   - UpdateBlobCommand        — through ICRegistry: a file-backed IC embedded, an embedded one extracted to a file
 *
 * A whole .panda input is the circuit, loaded beside the repository's circuits (Tests/Fuzz/FuzzCircuitDir.h) so its
 * file-backed ICs resolve, and its bytes, read again, choose the commands. Any other input chooses the commands
 * applied to a small wired circuit built at start-up.
 */

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <memory>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QScopeGuard>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QUndoStack>
#include <QVariant>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "Tests/Fuzz/FuzzCircuitDir.h"
#include "Tests/Fuzz/FuzzSeed.h"
#include "Tests/Fuzz/PandaMutator.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// Valid IC blob (InputSwitch + Led) for RegisterBlobCommand and embedded ICs.
QByteArray g_icBlob;

// Pre-built workspace bytes: And gate + two InputSwitches + Led, one wire.
QByteArray g_validPanda;

// The repository's circuits, which a loaded circuit's file-backed ICs name and an added file-backed IC is built from
// Held until exit, which removes it
std::unique_ptr<QTemporaryDir> g_dir;
QStringList g_icFiles;

// Element types that are safe to morph to (stable port count, no IC dependency)
constexpr ElementType kMorphTargets[] = {
    ElementType::And, ElementType::Or, ElementType::Not,
    ElementType::Nand, ElementType::Nor, ElementType::Xor,
};

// Element types added by AddItemsCommand, every property SetPropertyCommand sets among them
constexpr ElementType kAddTypes[] = {
    ElementType::And,
    ElementType::Or,
    ElementType::InputSwitch,
    ElementType::InputRotary,
    ElementType::Led,
    ElementType::Not,
    ElementType::Clock,
    ElementType::Node,
    ElementType::DFlipFlop,
    ElementType::TruthTable,
    ElementType::Buzzer,
    ElementType::AudioBox,
};

constexpr ElementProperty kProperties[] = {
    ElementProperty::Label, ElementProperty::Color, ElementProperty::Frequency, ElementProperty::Delay,
    ElementProperty::Volume, ElementProperty::Audio, ElementProperty::Trigger, ElementProperty::Wireless,
    ElementProperty::Locked, ElementProperty::Value,
};

// A value of \a property, as the element editor sets one, or one out of its range
QVariant propertyValue(const ElementProperty property, FuzzedDataProvider &fdp)
{
    const int pick = fdp.ConsumeIntegralInRange<int>(0, 3);
    switch (property) {
    case ElementProperty::Label:     return QStringList{"", "a", "clk", QString(300, 'x')}.at(pick);
    case ElementProperty::Color:     return QStringList{"Red", "Green", "White", "Plaid"}.at(pick);
    case ElementProperty::Frequency: return QList<double>{0.5, 1.0, 50.0, -1.0}.at(pick);
    case ElementProperty::Delay:     return QList<double>{0.0, 0.25, 0.5, -1.0}.at(pick);
    case ElementProperty::Volume:    return QList<double>{0.0, 0.5, 1.0, 4.0}.at(pick);
    case ElementProperty::Audio:     return QStringList{"", "missing.wav", AudioBox::kDefaultAudioPath, "a b"}.at(pick);
    case ElementProperty::Trigger:   return QStringList{"", "A", "Space", "Ctrl+Shift+F12"}.at(pick);
    case ElementProperty::Wireless:  return QList<int>{0, 1, 2, 7}.at(pick);
    case ElementProperty::Locked:    return pick % 2 == 0;
    case ElementProperty::Value:     return QList<int>{0, 1, 3, -1}.at(pick);
    }
    return {};
}

QList<IC *> icsOf(const QList<GraphicElement *> &elements, const bool embedded)
{
    QList<IC *> ics;
    for (auto *elm : elements) {
        if (auto *ic = qobject_cast<IC *>(elm); ic && (ic->isEmbedded() == embedded)) {
            ics.append(ic);
        }
    }
    return ics;
}

void applyRandomCommands(WorkSpace &workspace, FuzzedDataProvider &fdp, const QString &outDir)
{
    Scene *scene = workspace.scene();
    QUndoStack *stack = scene->undoStack();
    ICRegistry *registry = scene->icRegistry();

    const int numOps = fdp.ConsumeIntegralInRange<int>(1, 8);
    for (int op = 0; op < numOps; ++op) {
        const auto elems = scene->elements();
        const auto pickElement = [&]() {
            return elems.at(fdp.ConsumeIntegralInRange<int>(0, static_cast<int>(elems.size()) - 1));
        };

        const int action = fdp.ConsumeIntegralInRange<int>(0, 16);

        try {
            switch (action) {
            case 0: { // AddItemsCommand
                auto *elm = ElementFactory::buildElement(kAddTypes[fdp.ConsumeIntegralInRange<size_t>(
                    0, std::size(kAddTypes) - 1)]);
                elm->setPos(fdp.ConsumeIntegralInRange<int>(-20, 20) * 8.0, fdp.ConsumeIntegralInRange<int>(-20, 20) * 8.0);
                try {
                    scene->receiveCommand(new AddItemsCommand({elm}, scene));
                } catch (...) {
                    delete elm;
                }
                break;
            }
            case 1: { // DeleteItemsCommand
                if (elems.isEmpty()) break;
                scene->receiveCommand(new DeleteItemsCommand({static_cast<QGraphicsItem *>(pickElement())}, scene));
                break;
            }
            case 2: { // RotateCommand
                if (elems.isEmpty()) break;
                scene->receiveCommand(new RotateCommand(elems, fdp.ConsumeBool() ? 90 : -90, scene));
                break;
            }
            case 3: { // MoveCommand: the elements moved, then the command given where they were, as a drag ends
                if (elems.isEmpty()) break;
                const QList<GraphicElement *> moved = fdp.ConsumeBool() ? elems : QList<GraphicElement *>{pickElement()};
                QList<QPointF> oldPositions;
                const QPointF delta(fdp.ConsumeIntegralInRange<int>(-8, 8) * 8.0, fdp.ConsumeIntegralInRange<int>(-8, 8) * 8.0);
                for (auto *e : moved) {
                    oldPositions << e->pos();
                    e->setPos(e->pos() + delta);
                }
                scene->receiveCommand(new MoveCommand(moved, oldPositions, scene));
                break;
            }
            case 4: { // FlipCommand (axis 0 = horizontal, 1 = vertical)
                if (elems.isEmpty()) break;
                scene->receiveCommand(new FlipCommand(elems, fdp.ConsumeBool() ? 1 : 0, scene));
                break;
            }
            case 5: { // MorphCommand
                if (elems.isEmpty()) break;
                const auto newType = kMorphTargets[fdp.ConsumeIntegralInRange<size_t>(0, std::size(kMorphTargets) - 1)];
                scene->receiveCommand(new MorphCommand({pickElement()}, newType, scene));
                break;
            }
            case 6: { // ChangePortSizeCommand
                if (elems.isEmpty()) break;
                auto *elm = pickElement();
                const int newSize = fdp.ConsumeIntegralInRange<int>(1, 8);
                scene->receiveCommand(new ChangePortSizeCommand({elm}, newSize, scene, fdp.ConsumeBool()));
                break;
            }
            case 7: { // RegisterBlobCommand
                scene->receiveCommand(new RegisterBlobCommand(registry->uniqueBlobName("fuzz_blob"), g_icBlob, scene));
                break;
            }
            case 8: { // RemoveBlobCommand, as the palette removes an embedded IC: with every IC using it, in one macro
                const auto names = registry->blobMap().keys();
                if (names.isEmpty()) break;
                workspace.removeEmbeddedIC(names.at(fdp.ConsumeIntegralInRange<int>(0, static_cast<int>(names.size()) - 1)));
                break;
            }
            case 9: { // UpdateCommand: the elements' state captured before an edit, then the edit
                if (elems.isEmpty()) break;
                const QByteArray oldData = ICRegistry::captureSnapshot(elems);
                elems.first()->setLabel(QString("edited_%1").arg(op));
                scene->receiveCommand(new UpdateCommand(elems, oldData, scene));
                break;
            }
            case 10: { // ToggleTruthTableOutputCommand, on a truth table added first when the circuit holds none
                TruthTable *tt = nullptr;
                for (auto *e : elems) {
                    if (e->elementType() == ElementType::TruthTable) {
                        tt = static_cast<TruthTable *>(e);
                        break;
                    }
                }
                if (!tt) {
                    tt = static_cast<TruthTable *>(ElementFactory::buildElement(ElementType::TruthTable));
                    try {
                        scene->receiveCommand(new AddItemsCommand({tt}, scene));
                    } catch (...) {
                        delete tt;
                        break;
                    }
                }
                // One past the key, which the command refuses
                const int pos = fdp.ConsumeIntegralInRange<int>(0, static_cast<int>(tt->key().size()));
                scene->receiveCommand(new ToggleTruthTableOutputCommand(tt, pos, scene));
                break;
            }
            case 11: { // SplitCommand: re-resolved every time, since a command can free the previous wire
                Connection *conn = nullptr;
                for (auto *item : scene->items()) {
                    if (auto *c = qgraphicsitem_cast<Connection *>(item)) {
                        conn = c;
                        break;
                    }
                }
                if (!conn) break;
                const QPointF midPos(fdp.ConsumeFloatingPointInRange<double>(-200.0, 200.0),
                                     fdp.ConsumeFloatingPointInRange<double>(-200.0, 200.0));
                scene->receiveCommand(new SplitCommand(conn, midPos, scene));
                break;
            }
            case 12: { // SetPropertyCommand: one element or every one, in an edit session or not
                if (elems.isEmpty()) break;
                const auto property = kProperties[fdp.ConsumeIntegralInRange<size_t>(0, std::size(kProperties) - 1)];
                const QList<GraphicElement *> targets = fdp.ConsumeBool() ? elems : QList<GraphicElement *>{pickElement()};
                const int session = fdp.ConsumeBool() ? 1 : -1;
                // One to three edits in a row, as a field typed into or a slider dragged: in a session they merge
                const int edits = fdp.ConsumeIntegralInRange<int>(1, 3);
                for (int edit = 0; edit < edits; ++edit) {
                    if (auto *command = SetPropertyCommand::apply(targets, property, propertyValue(property, fdp), scene, session)) {
                        scene->receiveCommand(command);
                    }
                }
                break;
            }
            case 13: { // RenameBlobCommand, as the element editor renames an embedded IC's blob
                const auto names = registry->blobMap().keys();
                if (names.isEmpty()) break;
                const QString oldName = names.at(fdp.ConsumeIntegralInRange<int>(0, static_cast<int>(names.size()) - 1));
                const QString newName = registry->uniqueBlobName(fdp.ConsumeBool() ? QString("renamed") : oldName);
                scene->receiveCommand(new RenameBlobCommand(oldName, newName, scene));
                break;
            }
            case 14: { // An IC added: file-backed, from one of the repository's circuits, or embedded
                if (fdp.ConsumeBool() && !g_icFiles.isEmpty()) {
                    auto *ic = new IC();
                    try {
                        ic->loadFile(g_icFiles.at(fdp.ConsumeIntegralInRange<int>(0, static_cast<int>(g_icFiles.size()) - 1)),
                                     g_dir->path());
                    } catch (...) {
                        delete ic;
                        throw;
                    }
                    try {
                        scene->receiveCommand(new AddItemsCommand({ic}, scene));
                    } catch (...) {
                        delete ic;
                    }
                } else {
                    registry->createEmbeddedIC(registry->uniqueBlobName("embedded"), g_icBlob, g_dir->path());
                }
                break;
            }
            case 15: { // UpdateBlobCommand: a file-backed IC's file embedded in every IC using it
                const auto ics = icsOf(elems, false);
                if (ics.isEmpty()) break;
                const QString file = ics.at(fdp.ConsumeIntegralInRange<int>(0, static_cast<int>(ics.size()) - 1))->file();
                QFile source(QDir(scene->contextDir()).absoluteFilePath(file));
                if (!source.open(QIODevice::ReadOnly)) break;
                registry->embedICsByFile(file, source.readAll(), registry->uniqueBlobName(QFileInfo(file).baseName()));
                break;
            }
            case 16: { // UpdateBlobCommand: an embedded IC's blob extracted to a file, its ICs then using the file
                const auto ics = icsOf(elems, true);
                if (ics.isEmpty()) break;
                const QString blobName = ics.at(fdp.ConsumeIntegralInRange<int>(0, static_cast<int>(ics.size()) - 1))->blobName();
                registry->extractToFile(blobName, QDir(outDir).filePath(QString("%1_%2.panda").arg(blobName).arg(op)));
                break;
            }
            default:
                break;
            }
        } catch (...) {}

        // Randomly undo/redo after each push
        if (fdp.ConsumeBool() && stack->canUndo()) {
            try { stack->undo(); } catch (...) {}
        }
        if (fdp.ConsumeBool() && stack->canRedo()) {
            try { stack->redo(); } catch (...) {}
        }
    }

    // Fully unwind the stack and replay forward to exercise all undo/redo paths.
    // Track the index before/after each undo to break if the stack index doesn't
    // change (indicates a broken command that swallows exceptions and doesn't advance).
    int prevIdx = stack->index();
    while (stack->canUndo()) {
        try { stack->undo(); } catch (...) {}
        if (stack->index() == prevIdx) break; // stuck: undo didn't advance index
        prevIdx = stack->index();
    }
    prevIdx = stack->index();
    while (stack->canRedo()) {
        try { stack->redo(); } catch (...) {}
        if (stack->index() == prevIdx) break;
        prevIdx = stack->index();
    }
}

} // namespace

// A whole .panda input is the circuit: the shared structure-aware mutator works on a current-format file's
// decompressed body (Tests/Fuzz/PandaMutator.h)
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

    g_dir = std::make_unique<QTemporaryDir>();
    FuzzCircuitDir::fill(g_dir->path(), "fuzz_undo");
    // The integration circuits under IC/ are sub-circuits, the ones a project uses as file-backed ICs
    for (const QString &name : QDir(g_dir->path()).entryList({"*.panda"}, QDir::Files)) {
        if (QFile::exists(QCoreApplication::applicationDirPath() + "/../Tests/Integration/IC/" + name)) {
            g_icFiles.append(QDir(g_dir->path()).filePath(name));
        }
    }

    // Build a valid IC blob for RegisterBlobCommand tests.
    {
        auto *sw  = ElementFactory::buildElement(ElementType::InputSwitch);
        auto *led = ElementFactory::buildElement(ElementType::Led);
        sw->setId(1);
        led->setId(2);
        QList<QGraphicsItem *> items;
        items.append(sw);
        items.append(led);

        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        g_icBlob = FuzzSeed::pandaFile(meta, items);
        qDeleteAll(items);
        FuzzSeed::requireAccepted(g_icBlob, "fuzz_undo");
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

        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        g_validPanda = FuzzSeed::pandaFile(meta, [&items](QDataStream &s) {
            Serialization::serialize(items, s, {.purpose = SerializationPurpose::PortableFile});

            // Append one Connection (sw1 output → And input0) as raw wire bytes.
            // Writing it after serialize() ensures the element port serialIds are already
            // in the stream (portMap is populated by elements before connections are wired).
            //
            // Port serialId formula (from GraphicElement::save):
            //   serialId = (element_id << 16) | portIndex
            // sw1 (id=2) output0: (2<<16)|0 = 131072
            // andG (id=1) input0:  (1<<16)|0 = 65536
            constexpr quint64 sw1Out0 = (quint64(2) << 16) | 0;  // sw1 output port 0
            constexpr quint64 andIn0  = (quint64(1) << 16) | 0;  // andG input port 0
            s << static_cast<int>(QGraphicsItem::UserType + 2);   // Connection::Type
            QMap<QString, QVariant> connMap;
            connMap.insert("startPortId", QVariant::fromValue(sw1Out0));
            connMap.insert("endPortId",   QVariant::fromValue(andIn0));
            s << connMap;
        });
        qDeleteAll(items);
        FuzzSeed::requireAccepted(g_validPanda, "fuzz_undo");
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

    // A .panda input is the circuit, its own bytes choosing the commands; any other input chooses the commands
    // applied to the pre-built circuit
    const bool isPanda = PandaMutator::isPandaFile(data, size);
    const QByteArray circuit = isPanda ? QByteArray::fromRawData(reinterpret_cast<const char *>(data),
                                                                 static_cast<qsizetype>(size))
                                       : g_validPanda;

    WorkSpace ws;
    {
        QDataStream stream(circuit);
        try {
            const QVersionNumber version = Serialization::readPandaHeader(stream);
            ws.load(stream, version, g_dir->path());
        } catch (...) {
            return 0;
        }
    }

    QTemporaryDir outDir;
    if (!outDir.isValid()) return 0;
    FuzzedDataProvider fdp(data, size);
    applyRandomCommands(ws, fdp, outDir.path());

    return 0;
}
