// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Perf/TestPerfProbe.h"

#include <algorithm>
#include <array>

#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QScrollBar>
#include <QTest>
#include <QUndoStack>

#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ConnectionManager.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

namespace PerfProbe {

constexpr QSize kFrameSize(1280, 800);

/// The circuit under measurement: PROBE_FILE if set, otherwise a fixture small enough for a smoke test.
QString probeFile()
{
    const QString file = qEnvironmentVariable("PROBE_FILE");
    return file.isEmpty() ? QString(QUOTE(CURRENTDIR)) + "/Fixtures/Perf/clocked_2000.panda" : file;
}

struct Probe {
    // On before the load: the suite's default leaves every wire without a path.
    TestUtils::ScopedRendering rendering;
    QFile file{probeFile()};
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    Simulation *simulation = scene->simulation();
};

/// The ends of the wire the edit phases add.
struct WireEnds {
    OutputPort *source = nullptr;
    InputPort *target = nullptr;
};

/// The circuit's elements in ascending id: the order the edits pick their elements in.
QVector<GraphicElement *> elementsInIdOrder(const Scene *scene)
{
    QVector<GraphicElement *> elements = scene->unsortedElements();
    std::sort(elements.begin(), elements.end(), [](const GraphicElement *a, const GraphicElement *b) {
        return a->id() < b->id();
    });
    return elements;
}

/// The first output of the first element that has no inputs, and the first input without a wire that the
/// application lets that output drive.
WireEnds freeWireEnds(const Scene *scene)
{
    WireEnds ends;
    const auto elements = elementsInIdOrder(scene);
    for (auto *element : elements) {
        if ((element->outputSize() > 0) && (element->inputSize() == 0)) {
            ends.source = element->outputPort(0);
            break;
        }
    }
    if (!ends.source) {
        return ends;
    }
    for (auto *element : elements) {
        for (auto *port : element->inputs()) {
            if (port->connections().isEmpty() && ConnectionManager::connectionRejectionReason(ends.source, port).isEmpty()) {
                ends.target = port;
                return ends;
            }
        }
    }
    return ends;
}

/// Reads the circuit into the workspace and builds its simulation: what opening a file costs. The stream
/// load never migrates the file on disk, which loading by file name does to an older format.
Q_NEVER_INLINE void phaseLoad(Probe &probe)
{
    QDataStream stream(&probe.file);
    const QVersionNumber version = Serialization::readPandaHeader(stream);
    probe.workspace.load(stream, version, QFileInfo(probe.file).absolutePath());
    probe.simulation->initialize();
}

/// Opens the circuit and settles it with five ticks, as the phases that edit or paint it start from; false
/// when the file cannot be read or holds no element.
bool prepare(Probe &probe)
{
    if (!probe.file.open(QIODevice::ReadOnly)) {
        return false;
    }
    phaseLoad(probe);
    for (int i = 0; i < 5; ++i) {
        probe.simulation->update();
    }
    return probe.scene->elementCount() > 0;
}

/// The workspace's own view, shown off screen at kFrameSize with fast mode off, as the application shows it.
/// The scene asks this view for the zoom (the grid) and the visible rect (the scene rect), so painting
/// through another view would paint a different frame. The workspace itself is shown and sized: grabbing a hidden
/// child activates the top-level layout, which overrides the view's size.
GraphicsView *showView(WorkSpace &workspace)
{
    workspace.setAttribute(Qt::WA_DontShowOnScreen);
    workspace.show();
    auto *view = workspace.view();
    workspace.resize(workspace.size() + kFrameSize - view->size());
    view->setFastMode(false);
    return view;
}

/// Paints one frame of the canvas with wire antialiasing held on: under callgrind every pass is over the
/// adaptive budget and would turn it off partway through a phase, so the count would depend on time.
void paintFrame(GraphicsView *view, Scene *scene)
{
    scene->restoreWireAntialiasing();
    view->viewport()->grab();
}

/// Sets the view to 1:1 with the circuit's top left corner at the window's.
void showCircuitCorner(GraphicsView *view, const Scene *scene)
{
    view->resetZoom();
    const QSize viewport = view->viewport()->size();
    view->centerOn(scene->cachedItemsBoundingRect().topLeft() + QPointF(viewport.width() / 2.0, viewport.height() / 2.0));
}

/// Three frames at 1:1, then three of the whole circuit zoomed to fit.
Q_NEVER_INLINE void phaseFrames(GraphicsView *view, Scene *scene)
{
    showCircuitCorner(view, scene);
    for (int i = 0; i < 3; ++i) {
        paintFrame(view, scene);
    }
    view->zoomToFit();
    for (int i = 0; i < 3; ++i) {
        paintFrame(view, scene);
    }
}

/// Opens the circuit in a view already on screen, then its first frames.
Q_NEVER_INLINE void phaseLoadThenFrames(Probe &probe, GraphicsView *view)
{
    phaseLoad(probe);
    phaseFrames(view, probe.scene);
}

/// Two frames at 1:1, then eight more, each after scrolling 48 pixels to the right: a pan at a fixed zoom.
Q_NEVER_INLINE void phasePan(GraphicsView *view, Scene *scene)
{
    showCircuitCorner(view, scene);
    for (int i = 0; i < 2; ++i) {
        paintFrame(view, scene);
    }
    auto *scrollBar = view->horizontalScrollBar();
    for (int i = 0; i < 8; ++i) {
        scrollBar->setValue(scrollBar->value() + 48);
        paintFrame(view, scene);
    }
}

/// One simulation tick with nothing changed.
Q_NEVER_INLINE void phaseIdleTick(Probe &probe)
{
    probe.simulation->update();
}

/// Adds a wire as completing one does, then the tick that rebuilds the simulation.
Q_NEVER_INLINE void phaseAddWire(Probe &probe, const WireEnds &ends)
{
    auto *wire = new Connection();
    wire->setStartPort(ends.source);
    wire->setEndPort(ends.target);
    probe.scene->receiveCommand(new AddItemsCommand({wire}, probe.scene));
    probe.simulation->update();
}

/// Undoes the last command, then the tick.
Q_NEVER_INLINE void phaseUndo(Probe &probe)
{
    probe.scene->undoStack()->undo();
    probe.simulation->update();
}

/// Moves \a element 64 units to the right as a drag does (the move, then its command), then the tick.
Q_NEVER_INLINE void phaseMove(Probe &probe, GraphicElement *element)
{
    const QPointF oldPosition = element->pos();
    element->setPos(oldPosition + QPointF(64, 0));
    probe.scene->receiveCommand(new MoveCommand({element}, {oldPosition}, probe.scene));
    probe.simulation->update();
}

/// Deletes the selection as the Delete key does, then the tick.
Q_NEVER_INLINE void phaseDelete(Probe &probe)
{
    probe.scene->deleteAction();
    probe.simulation->update();
}

} // namespace PerfProbe

void TestPerfProbe::initTestCase()
{
    // The probe is slow enough to keep out of the default run; Scripts/profile_probe.sh sets the variable that runs it.
    if (!qEnvironmentVariableIsSet("WIREDPANDA_PERF_PROBE")) {
        QSKIP("the performance probe runs only with WIREDPANDA_PERF_PROBE set");
    }
}

void TestPerfProbe::load()
{
    PerfProbe::Probe probe;
    QVERIFY2(probe.file.open(QIODevice::ReadOnly), qPrintable("Could not open " + probe.file.fileName()));
    PerfProbe::phaseLoad(probe);
    QVERIFY(probe.scene->elementCount() > 0);
}

void TestPerfProbe::loadThenFrames()
{
    PerfProbe::Probe probe;
    QVERIFY2(probe.file.open(QIODevice::ReadOnly), qPrintable("Could not open " + probe.file.fileName()));
    auto *view = PerfProbe::showView(probe.workspace);
    QCOMPARE(view->size(), PerfProbe::kFrameSize);
    PerfProbe::phaseLoadThenFrames(probe, view);
    QVERIFY(probe.scene->elementCount() > 0);
}

void TestPerfProbe::idleTick()
{
    PerfProbe::Probe probe;
    QVERIFY2(PerfProbe::prepare(probe), qPrintable("No circuit in " + probe.file.fileName()));
    PerfProbe::phaseIdleTick(probe);
}

void TestPerfProbe::addWire()
{
    PerfProbe::Probe probe;
    QVERIFY2(PerfProbe::prepare(probe), qPrintable("No circuit in " + probe.file.fileName()));
    const PerfProbe::WireEnds ends = PerfProbe::freeWireEnds(probe.scene);
    QVERIFY2(ends.source && ends.target, "the circuit has no element without inputs or no input it may drive");
    const int commands = probe.scene->undoStack()->count();
    PerfProbe::phaseAddWire(probe, ends);
    // Setting the wire's ends attaches it to the ports, so only the scene and the undo stack show the command ran.
    QCOMPARE(ends.target->connections().size(), 1);
    QCOMPARE(ends.target->connections().constFirst()->scene(), probe.scene);
    QCOMPARE(probe.scene->undoStack()->count(), commands + 1);
}

void TestPerfProbe::addWireThenUndo()
{
    PerfProbe::Probe probe;
    QVERIFY2(PerfProbe::prepare(probe), qPrintable("No circuit in " + probe.file.fileName()));
    const PerfProbe::WireEnds ends = PerfProbe::freeWireEnds(probe.scene);
    QVERIFY2(ends.source && ends.target, "the circuit has no element without inputs or no input it may drive");
    PerfProbe::phaseAddWire(probe, ends);
    PerfProbe::phaseUndo(probe);
    QVERIFY(ends.target->connections().isEmpty());
}

void TestPerfProbe::moveElement()
{
    PerfProbe::Probe probe;
    QVERIFY2(PerfProbe::prepare(probe), qPrintable("No circuit in " + probe.file.fileName()));
    auto *element = PerfProbe::elementsInIdOrder(probe.scene).constFirst();
    const QPointF oldPosition = element->pos();
    const int commands = probe.scene->undoStack()->count();
    PerfProbe::phaseMove(probe, element);
    QCOMPARE(element->pos(), oldPosition + QPointF(64, 0));
    QCOMPARE(probe.scene->undoStack()->count(), commands + 1);
}

void TestPerfProbe::deleteElement()
{
    PerfProbe::Probe probe;
    QVERIFY2(PerfProbe::prepare(probe), qPrintable("No circuit in " + probe.file.fileName()));
    const qsizetype count = probe.scene->elementCount();
    PerfProbe::elementsInIdOrder(probe.scene).constLast()->setSelected(true);
    PerfProbe::phaseDelete(probe);
    QCOMPARE(probe.scene->elementCount(), count - 1);
}

void TestPerfProbe::frames()
{
    PerfProbe::Probe probe;
    QVERIFY2(PerfProbe::prepare(probe), qPrintable("No circuit in " + probe.file.fileName()));
    auto *view = PerfProbe::showView(probe.workspace);
    QCOMPARE(view->size(), PerfProbe::kFrameSize);
    PerfProbe::phaseFrames(view, probe.scene);
}

void TestPerfProbe::pan()
{
    PerfProbe::Probe probe;
    QVERIFY2(PerfProbe::prepare(probe), qPrintable("No circuit in " + probe.file.fileName()));
    auto *view = PerfProbe::showView(probe.workspace);
    QCOMPARE(view->size(), PerfProbe::kFrameSize);
    PerfProbe::phasePan(view, probe.scene);
}

void TestPerfProbe::wireExtents()
{
    // How long the circuit's wires are (the longer side of each path's bounds): the distribution a renderer
    // chooses its threshold between short and long wires from.
    PerfProbe::Probe probe;
    QVERIFY2(probe.file.open(QIODevice::ReadOnly), qPrintable("Could not open " + probe.file.fileName()));
    PerfProbe::phaseLoad(probe);

    constexpr std::array<int, 8> kLimits{32, 64, 128, 256, 512, 1024, 2048, 4096};
    std::array<int, kLimits.size() + 1> counts{};
    int total = 0;
    int withoutPath = 0;
    for (const auto *wire : TestUtils::sceneConnections(probe.scene)) {
        if (wire->path().isEmpty()) {
            ++withoutPath;
        }
        const QRectF bounds = wire->path().boundingRect();
        const qreal extent = std::max(bounds.width(), bounds.height());
        std::size_t bucket = 0;
        while ((bucket < kLimits.size()) && (extent >= kLimits[bucket])) {
            ++bucket;
        }
        ++counts[bucket];
        ++total;
    }

    QStringList lines;
    for (std::size_t i = 0; i < counts.size(); ++i) {
        QString range;
        if (i == 0) {
            range = QString("< %1").arg(kLimits.front());
        } else if (i == kLimits.size()) {
            range = QString(">= %1").arg(kLimits.back());
        } else {
            range = QString("%1 .. %2").arg(kLimits[i - 1]).arg(kLimits[i]);
        }
        lines << QString("  %1: %2").arg(range, 12).arg(counts[i]);
    }
    qInfo().noquote() << "WIRE EXTENTS" << total << "wires\n" + lines.join('\n');
    QVERIFY(total > 0);
    // Rendering is on before the load, or every wire would be left without a path, and every phase would measure that.
    QCOMPARE(withoutPath, 0);
}
