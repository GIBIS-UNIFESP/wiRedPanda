// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness for the Arduino and SystemVerilog code generators.
 *
 * ArduinoCodeGen.cpp (495 functions) and SystemVerilogCodeGen.cpp (583 functions)
 * are at 0% coverage — they are only invoked through File > Export, never through
 * the file-load code path that all other harnesses exercise.
 *
 * Strategy: load a panda file exactly as fuzz_deserialize does, then pass the
 * resulting scene elements to both code generators writing to QTemporaryFiles.
 * The seed corpus already contains circuits with And/Or/Led/FlipFlop elements
 * that the generators support, so the load step succeeds on most seeds.
 *
 * If the circuit contains unsupported elements (IC, Clock with generate())
 * the generators throw PANDACEPTION — all exceptions are caught.
 *
 * ArduinoCodeGen::generateTestbench() is also exercised: after a successful
 * generate(), the harness calls generateTestbench() with a small set of
 * synthetic test vectors.  This is the only path that exercises the testbench
 * code-generation branch.
 */

#include <cstddef>
#include <cstdint>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QScopeGuard>
#include <QString>
#include <QTemporaryFile>
#include <QVersionNumber>

#include "App/CodeGen/ArduinoCodeGen.h"
#include "App/CodeGen/SystemVerilogCodeGen.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// A workspace with an IC element whose sub-circuit has InputSwitch + And + Led.
// Built in LLVMFuzzerInitialize so the IC has fully loaded internalElements,
// which exercises ArduinoCodeGen::declareAuxVariablesRec() and
// SystemVerilogCodeGen::generateICModules() — dark without embedded ICs.
WorkSpace *g_icWorkspace = nullptr;

void runCodeGen(const QVector<GraphicElement *> &elements)
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

        QByteArray icBlob;
        {
            QDataStream s(&icBlob, QIODevice::WriteOnly);
            Serialization::writePandaHeader(s);
            QMap<QString, QVariant> m;
            m.insert("dolphinFileName", QVariant(QString()));
            s << m;
            Serialization::serialize(inner, s, {.purpose = SerializationPurpose::PortableFile});
        }
        qDeleteAll(inner);

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
        QByteArray outerPanda;
        {
            QDataStream s(&outerPanda, QIODevice::WriteOnly);
            Serialization::writePandaHeader(s);
            QMap<QString, QVariant> meta;
            meta.insert("dolphinFileName", QVariant(QString()));
            meta.insert("embeddedICs", QVariant(regBytes));
            s << meta;
            // IC element: discriminator + ElementType + base map + port lists + secondary map
            s << static_cast<int>(65539);         // GraphicElement::Type
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
        }

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

    // Always run codegen on the pre-built IC workspace first.  Cold-corpus inputs
    // almost never pass the readPandaHeader check, so placing this before the
    // header check guarantees codegen coverage on every run regardless of whether
    // the fuzz input is a valid panda file.
    if (g_icWorkspace) {
        runCodeGen(g_icWorkspace->scene()->elements());
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
        workspace.load(stream, version, /*contextDir=*/QString());
    } catch (...) {
        return 0;
    }

    runCodeGen(workspace.scene()->elements());

    return 0;
}
