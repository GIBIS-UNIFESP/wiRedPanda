// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestWirePixelSnapshot.h"

#include <QDir>
#include <QImage>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QTest>

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationThrottleDisabler.h"
#include "App/Wiring/Connection.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

// A named namespace: a unity build can put this file in one translation unit with other snapshots' helpers.
namespace WirePixelSnapshot {

constexpr int kWidth = 320;
constexpr int kHeight = 200;

QString goldensDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/WirePixelSnapshot/";
}

/// A circuit the goldens paint, at each of its scales.
struct Painting {
    QString name;
    QList<qreal> scales;
};

const Painting kSynthetic{"synthetic", {1.0, 0.5, 0.25, 0.1, 0.05}};
/// The dense fixture is painted at three scales, which keeps its goldens few.
const Painting kClocked{"clocked_2000", {1.0, 0.25, 0.05}};

/// The two window positions: 20 pixels in from the circuit's top left corner, and centred on it.
constexpr int kWindows = 2;

QString goldenName(const QString &name, const qreal scale, const int window)
{
    return QString("%1_s%2_w%3.png").arg(name, QString::number(qRound(scale * 100)), QString::number(window));
}

/// Paints the visible wires of \a scene, bottom first in the order Qt stacks them, at \a scale with the scene's point
/// \a origin at the image's top left corner, as a view paints them with antialiasing on.
QImage paintWires(const Scene *scene, const QPointF &origin, const qreal scale)
{
    QImage image(kWidth, kHeight, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.scale(scale, scale);
    painter.translate(-origin);

    for (auto *item : scene->QGraphicsScene::items(Qt::AscendingOrder)) {
        auto *wire = qgraphicsitem_cast<Connection *>(item);
        if (!wire || !wire->isVisible()) {
            continue;
        }
        QStyleOptionGraphicsItem option;
        option.state = QStyle::State_Enabled;
        if (wire->isSelected()) {
            option.state |= QStyle::State_Selected;
        }
        option.exposedRect = wire->boundingRect();
        painter.save();
        painter.setTransform(wire->sceneTransform(), true);
        wire->paint(&painter, &option, nullptr);
        painter.restore();
    }
    painter.end();
    return image;
}

/// The rectangle the elements' bodies span, without their labels, so it does not depend on fonts.
QRectF circuitBounds(const Scene *scene)
{
    QRectF bounds;
    for (const auto *element : scene->unsortedElements()) {
        bounds |= element->sceneBoundingRect();
    }
    return bounds;
}

/// Paints the wires of \a scene at each of \a painting's scales from both window positions, and returns how each image
/// differs from its golden.
QStringList compareAtScales(const Scene *scene, const Painting &painting)
{
    // Nothing measured a paint pass, so the adaptive antialiasing is on: only a view's paint event reports passes.
    if (!scene->wireAntialiasingEnabled()) {
        return {"wire antialiasing is off"};
    }

    const QRectF bounds = circuitBounds(scene);
    QStringList mismatches;
    for (const qreal scale : painting.scales) {
        const QList<QPointF> origins{bounds.topLeft() - QPointF(20, 20) / scale,
                                     bounds.center() - QPointF(kWidth, kHeight) / (2 * scale)};
        Q_ASSERT(origins.size() == kWindows);
        for (int window = 0; window < kWindows; ++window) {
            const QString mismatch = TestUtils::compareWithGoldenImage(goldensDir() + goldenName(painting.name, scale, window),
                                                                       paintWires(scene, origins.at(window), scale));
            if (!mismatch.isEmpty()) {
                mismatches << mismatch;
            }
        }
    }
    return mismatches;
}

} // namespace WirePixelSnapshot

void TestWirePixelSnapshot::testSyntheticWiresMatchGoldens()
{
    const TestUtils::ScopedThemeOverride theme;
    ThemeManager::setTheme(Theme::Light);
    const TestUtils::ScopedRendering rendering;
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // One wire per span, from a switch's output to an Led's input: every length class, flat, steep and diagonal, and
    // ends in both orders.
    struct Span {
        QPointF from;
        QPointF to;
    };
    const QList<Span> spans{
        {{0, 0}, {160, 32}},          // short
        {{0, 160}, {416, 224}},       // medium
        {{0, 320}, {960, 352}},       // long, nearly flat
        {{0, 480}, {2400, 560}},      // very long, nearly flat
        {{2560, 0}, {2624, 800}},     // steep
        {{2880, 0}, {2944, 3200}},    // very long, steep
        {{3200, 3200}, {6400, 6400}}, // very long, diagonal
        {{1600, 1600}, {1344, 1152}}, // the end left of and above the start
        {{5600, 0}, {3520, 2240}},    // the same, long and diagonal
    };
    const QList<Status> statuses{Status::Unknown, Status::Inactive, Status::Active, Status::Error};

    QList<Connection *> wires;
    for (int index = 0; index < spans.size(); ++index) {
        auto *source = new InputSwitch();
        source->setPos(spans.at(index).from);
        scene->addItem(source);
        auto *sink = new Led();
        sink->setPos(spans.at(index).to);
        scene->addItem(sink);

        auto *wire = new Connection();
        wire->setStartPort(source->outputPort(0));
        wire->setEndPort(sink->inputPort(0));
        scene->addItem(wire);
        wire->setStatus(statuses.at(index % statuses.size()));
        wires << wire;
    }
    wires.at(2)->setSelected(true);
    wires.at(5)->setHighLight(true);

    const QStringList mismatches = WirePixelSnapshot::compareAtScales(scene, WirePixelSnapshot::kSynthetic);
    QVERIFY2(mismatches.isEmpty(), qPrintable(mismatches.join('\n')));
}

void TestWirePixelSnapshot::testClockedFixtureWiresMatchGoldens()
{
    const TestUtils::ScopedThemeOverride theme;
    ThemeManager::setTheme(Theme::Light);
    const TestUtils::ScopedRendering rendering;
    WorkSpace workspace;
    TestUtils::loadCircuit(workspace, QString(QUOTE(CURRENTDIR)) + "/Fixtures/Perf/clocked_2000.panda");
    if (QTest::currentTestFailed()) {
        return;
    }
    auto *scene = workspace.scene();

    // The wires take the statuses of three ticks stepped by hand, the timer stopped. The first tick builds the
    // simulation and starts the clock HIGH; locked, it holds that level however long the ticks take.
    int clocks = 0;
    for (auto *element : scene->unsortedElements()) {
        if (element->elementType() == ElementType::Clock) {
            static_cast<GraphicElementInput *>(element)->setLocked(true);
            ++clocks;
        }
    }
    QCOMPARE(clocks, 1);
    auto *simulation = scene->simulation();
    QVERIFY(!simulation->isRunning());
    {
        const SimulationThrottleDisabler noThrottle(simulation);
        for (int tick = 0; tick < 3; ++tick) {
            simulation->update();
        }
    }

    const QStringList mismatches = WirePixelSnapshot::compareAtScales(scene, WirePixelSnapshot::kClocked);
    QVERIFY2(mismatches.isEmpty(), qPrintable(mismatches.join('\n')));
}

void TestWirePixelSnapshot::testEveryGoldenIsCompared()
{
    // A golden no painting names would otherwise stay in the tree, compared with nothing.
    QStringList expected;
    for (const auto &painting : {WirePixelSnapshot::kSynthetic, WirePixelSnapshot::kClocked}) {
        for (const qreal scale : painting.scales) {
            for (int window = 0; window < WirePixelSnapshot::kWindows; ++window) {
                expected << WirePixelSnapshot::goldenName(painting.name, scale, window);
            }
        }
    }
    expected.sort();
    QCOMPARE(QDir(WirePixelSnapshot::goldensDir()).entryList(QDir::Files, QDir::Name), expected);
}
