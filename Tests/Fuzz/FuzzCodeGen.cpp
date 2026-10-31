// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness for the Arduino and SystemVerilog code generators.
 *
 * ArduinoCodeGen.cpp (495 functions) and SystemVerilogCodeGen.cpp (583 functions)
 * are at 0% coverage — they are only invoked through File > Export, never through
 * the file-load code path that all other harnesses exercise.
 *
 * Strategy: a whole .panda input (WPCF, a legacy text header, or a leading 0
 * byte) is loaded exactly as fuzz_deserialize does, beside the repository's
 * circuits so the IC files it names resolve, then the resulting scene
 * elements go to both code generators writing to QTemporaryFiles.  Any other
 * input describes a circuit built in the scene (buildCircuit()): elements of
 * every type the generators take, ICs among them, wired from the input.
 *
 * If the circuit contains unsupported elements (IC, Clock with generate())
 * the generators throw PANDACEPTION — all exceptions are caught.
 *
 * ArduinoCodeGen::generateTestbench() is also exercised: after a successful
 * generate(), the harness calls generateTestbench() with a small set of
 * synthetic test vectors.  This is the only path that exercises the testbench
 * code-generation branch.
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <iterator>
#include <memory>
#include <utility>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QFile>
#include <QIODevice>
#include <QMap>
#include <QPointF>
#include <QScopeGuard>
#include <QString>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QVariant>
#include <QVector>
#include <QVersionNumber>

#include "App/CodeGen/ArduinoCodeGen.h"
#include "App/CodeGen/SystemVerilogCodeGen.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Presentation/ElementPresenter.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Fuzz/FuzzCircuitDir.h"
#include "Tests/Fuzz/FuzzSeed.h"
#include "Tests/Fuzz/PandaMutator.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// A workspace with an IC element whose sub-circuit has InputSwitch + And + DFlipFlop + Led, built and generated
// once in LLVMFuzzerInitialize.
WorkSpace *g_icWorkspace = nullptr;

// Where a structured input's file-backed ICs are written.
std::unique_ptr<QTemporaryDir> g_dir;

// A whole .panda input's context directory: the repository's circuits (Tests/Fuzz/FuzzCircuitDir.h), so the IC files
// it names load; apart from g_dir, whose files are written.
std::unique_ptr<QTemporaryDir> g_circuitDir;

void runCodeGen(const QVector<Element *> &elements)
{
    if (elements.isEmpty()) return;

    // Arduino code gen + testbench
    {
        QTemporaryFile tf;
        tf.setAutoRemove(true);
        if (tf.open()) {
            const QString path = tf.fileName();
            tf.close(); // let ArduinoCodeGen open it for WriteOnly
            try {
                ArduinoCodeGen gen(path, elements);
                gen.generate();

                // generateTestbench() is completely dark without an explicit call.
                // It uses the internal input/output maps populated by generate(),
                // so calling it after a successful generate() exercises the testbench
                // code path with circuit-driven port counts.
                QTemporaryFile tbf;
                tbf.setAutoRemove(true);
                if (tbf.open()) {
                    const QString tbPath = tbf.fileName();
                    tbf.close();
                    // Two synthetic vectors: all-LOW and all-HIGH.
                    QVector<ArduinoCodeGen::TestVector> vectors;
                    vectors.append({{false, false}, {false}});
                    vectors.append({{true,  true},  {true}});
                    try {
                        gen.generateTestbench(tbPath, vectors);
                    } catch (...) {}
                }
            } catch (...) {}
        }
    }

    // SystemVerilog code gen
    {
        QTemporaryFile tf;
        tf.setAutoRemove(true);
        if (tf.open()) {
            const QString path = tf.fileName();
            tf.close();
            try {
                SystemVerilogCodeGen gen(path, elements);
                gen.generate();
            } catch (...) {}
        }
    }
}

constexpr ElementType kSources[] = {ElementType::InputSwitch, ElementType::InputButton, ElementType::InputRotary,
                                    ElementType::Clock, ElementType::InputVcc, ElementType::InputGnd};
constexpr ElementType kLogic[] = {ElementType::And, ElementType::Or, ElementType::Nand, ElementType::Nor,
                                  ElementType::Xor, ElementType::Xnor, ElementType::Not, ElementType::Node,
                                  ElementType::DFlipFlop, ElementType::DLatch, ElementType::JKFlipFlop,
                                  ElementType::SRFlipFlop, ElementType::TFlipFlop, ElementType::SRLatch,
                                  ElementType::Mux, ElementType::Demux, ElementType::TruthTable};
constexpr ElementType kSinks[] = {ElementType::Led, ElementType::Display7, ElementType::Display14,
                                  ElementType::Display16, ElementType::Buzzer};
// Labels a generator must turn into identifiers: empty, plain, reserved in C++ or SystemVerilog, starting with a
// digit, with a space, and two alike.
const char *const kLabels[] = {"", "a", "a", "clk", "if", "module", "1x", "a b"};

template <std::size_t N>
ElementType pick(FuzzedDataProvider &fdp, const ElementType (&types)[N])
{
    return types[fdp.ConsumeIntegralInRange<std::size_t>(0, N - 1)];
}

// IC names that a generator's module names must tell apart: plain, alike once the forbidden characters go, reserved,
// starting with a digit.
const char *const kICNames[] = {"sub", "sub-", "module", "1sub"};

void writeFile(const QString &path, const QByteArray &bytes)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(bytes);
    }
}

void buildCircuit(FuzzedDataProvider &fdp, Scene *scene, bool withICs);

// An IC whose body is a circuit the input describes, embedded in the scene's registry or loaded from a file in the
// temporary directory, under one of kICNames; nullptr when the body does not load as an IC.
Element *buildIC(FuzzedDataProvider &fdp, Scene *scene)
{
    QByteArray blob;
    {
        WorkSpace body;
        buildCircuit(fdp, body.scene(), false);
        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        blob = FuzzSeed::pandaFile(meta, body.scene()->items());
    }
    const QString name = kICNames[fdp.ConsumeIntegralInRange<std::size_t>(0, std::size(kICNames) - 1)];
    auto *ic = new IC();
    try {
        if (fdp.ConsumeBool()) {
            scene->icRegistry()->setBlob(name, blob);
            scene->icRegistry()->initEmbeddedIC(ic, name);
        } else {
            const QString path = g_dir->filePath(name + ".panda");
            writeFile(path, blob);
            ic->loadFile(path, g_dir->path());
        }
    } catch (...) {
        delete ic;
        return nullptr;
    }
    return ic;
}

// A circuit the input describes, built in the scene: 1 to 12 elements, each a source, a logic element, a sink or (at
// the top level) an IC, with port counts drawn within its type's range, every input wired from any element's output,
// earlier or later (a later one makes a loop), or left open.  The corpus's circuits reach the generators only when
// they load and fit the board's pins, which the latches, the SR and T flip-flops, the rotary switch, the truth table
// and an IC with names a module must escape seldom do, so this builds them.
void buildCircuit(FuzzedDataProvider &fdp, Scene *scene, const bool withICs)
{
    QVector<Element *> elements;
    const int count = fdp.ConsumeIntegralInRange<int>(1, 12);
    for (int i = 0; i < count; ++i) {
        const int kind = fdp.ConsumeIntegralInRange<int>(0, withICs ? 4 : 3);
        Element *elm = nullptr;
        if (kind == 4) {
            elm = buildIC(fdp, scene);
            if (!elm) {
                continue;
            }
        } else {
            elm = ElementFactory::buildElement((kind == 0) ? pick(fdp, kSources)
                                               : (kind == 3) ? pick(fdp, kSinks)
                                                             : pick(fdp, kLogic));
        }
        // Half the elements up to their type's maximum, enough rotary positions to outrun the largest board's pins
        const int extra = fdp.ConsumeBool() ? 4 : 64;
        if (elm->maxInputSize() > elm->minInputSize()) {
            elm->setInputSize(fdp.ConsumeIntegralInRange<int>(elm->minInputSize(), std::min(elm->maxInputSize(), elm->minInputSize() + extra)));
        }
        if (elm->maxOutputSize() > elm->minOutputSize()) {
            elm->setOutputSize(fdp.ConsumeIntegralInRange<int>(elm->minOutputSize(), std::min(elm->maxOutputSize(), elm->minOutputSize() + extra)));
        }
        elm->setLabel(kLabels[fdp.ConsumeIntegralInRange<std::size_t>(0, std::size(kLabels) - 1)]);
        elm->setPos(QPointF(64.0 * i, 0.0));
        scene->addItem(elm);
        elements.append(elm);
    }
    for (auto *elm : std::as_const(elements)) {
        for (auto *input : elm->inputs()) {
            if (fdp.ConsumeIntegralInRange<int>(0, 7) == 0) {
                continue;
            }
            auto *source = elements[fdp.ConsumeIntegralInRange<qsizetype>(0, elements.size() - 1)];
            if (source->outputSize() == 0) {
                continue;
            }
            auto *conn = new Connection();
            conn->setStartPort(source->outputPort(fdp.ConsumeIntegralInRange<int>(0, source->outputSize() - 1)));
            conn->setEndPort(input);
            scene->addItem(conn);
        }
    }
}

} // namespace

// The input is a whole .panda file: the shared structure-aware mutator works on a current-format file's decompressed
// body (Tests/Fuzz/PandaMutator.h)
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
    installPresenters();
    // Both held until exit, which removes them
    g_dir  = std::make_unique<QTemporaryDir>();
    g_circuitDir = std::make_unique<QTemporaryDir>();
    FuzzCircuitDir::fill(g_circuitDir->path(), "fuzz_codegen");

    // Build two IC blobs: combinational (And gate) and sequential (DFlipFlop).
    // Embedding both exercises IC module generation AND detectSequentialICType.
    {
        // Sub-circuit blob: InputSwitch + And gate + Led + DFlipFlop (mixed circuit)
        auto *sw   = ElementFactory::buildElement(ElementType::InputSwitch);
        auto *andG = ElementFactory::buildElement(ElementType::And);
        auto *dff  = ElementFactory::buildElement(ElementType::DFlipFlop);
        auto *led  = ElementFactory::buildElement(ElementType::Led);
        sw->setId(1);  andG->setId(2);  dff->setId(3);  led->setId(4);
        QList<QGraphicsItem *> inner;
        inner.append(sw);  inner.append(andG);  inner.append(dff);  inner.append(led);

        QMap<QString, QVariant> innerMeta;
        innerMeta.insert("dolphinFileName", QVariant(QString()));
        const QByteArray icBlob = FuzzSeed::pandaFile(innerMeta, inner);
        qDeleteAll(inner);
        FuzzSeed::requireAccepted(icBlob, "fuzz_codegen");

        // Serialize the IC blob into the embeddedICs registry
        QByteArray regBytes;
        {
            QDataStream rs(&regBytes, QIODevice::WriteOnly);
            rs.setVersion(QDataStream::Qt_5_12);
            QMap<QString, QByteArray> reg;
            reg.insert("sub.panda", icBlob);
            rs << reg;
        }

        // Build outer panda: one IC element referencing "sub.panda"
        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        meta.insert("embeddedICs", QVariant(regBytes));
        const QByteArray outerPanda = FuzzSeed::pandaFile(meta, [](QDataStream &s) {
            // IC element: discriminator + ElementType + base map + port lists + secondary map
            s << static_cast<int>(65539);         // Element::Type
            s << static_cast<quint64>(22);         // ElementType::IC
            QMap<QString, QVariant> base;
            base.insert("pos", QVariant::fromValue(QPointF(100.0, 100.0)));
            base.insert("rotation", QVariant(0.0));
            base.insert("label", QVariant(QString()));
            s << base;
            s << quint32(0) << quint32(0) << quint32(0); // empty port lists
            QMap<QString, QVariant> icMap;
            icMap.insert("blobName", QVariant(QString("sub.panda")));
            s << icMap;
        });
        FuzzSeed::requireAccepted(outerPanda, "fuzz_codegen");

        g_icWorkspace = new WorkSpace();
        QDataStream stream(outerPanda);
        QVersionNumber ver;
        try {
            ver = Serialization::readPandaHeader(stream);
            g_icWorkspace->load(stream, ver, QString());
        } catch (...) {
            delete g_icWorkspace;
            g_icWorkspace = nullptr;
        }
    }

    // The circuit is the same for every input, so it is generated once
    if (g_icWorkspace) {
        runCodeGen(g_icWorkspace->scene()->elements());
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

    if (!PandaMutator::isPandaFile(data, size)) {
        FuzzedDataProvider fdp(data, size);
        WorkSpace workspace;
        buildCircuit(fdp, workspace.scene(), true);
        runCodeGen(workspace.scene()->elements());
        return 0;
    }

    // Load the panda file exactly as fuzz_deserialize does.  The seed corpus
    // contains valid circuits; once loaded, call both code generators on whatever
    // elements ended up in the scene.
    const QByteArray buffer = QByteArray::fromRawData(reinterpret_cast<const char *>(data),
                                                      static_cast<int>(size));
    QDataStream stream(buffer);

    QVersionNumber version;
    try {
        version = Serialization::readPandaHeader(stream);
    } catch (...) {
        return 0;
    }

    WorkSpace workspace;
    try {
        workspace.load(stream, version, g_circuitDir->path());
    } catch (...) {
        return 0;
    }

    runCodeGen(workspace.scene()->elements());

    return 0;
}
