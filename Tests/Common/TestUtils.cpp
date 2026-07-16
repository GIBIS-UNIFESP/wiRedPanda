// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/TestUtils.h"

#include <limits>

#include <QAbstractButton>
#include <QGraphicsScene>
#include <QPainter>
#include <QRandomGenerator>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include "App/Core/Application.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace TestUtils {

void setupTestEnvironment()
{
#ifdef Q_OS_LINUX
    // Default to headless, but never clobber an explicit choice — CI workflows
    // and developers set QT_QPA_PLATFORM to pick the platform per-OS (e.g.
    // xcb to exercise native window-activation timing locally).
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    // Disable input method plugins — ibus daemon is single-threaded and serializes
    // parallel test startup, causing 8 processes to take 4s instead of 0.06s.
    if (!qEnvironmentVariableIsSet("QT_IM_MODULES")) {
        qputenv("QT_IM_MODULES", "none");
    }
#endif
    // Redirect QSettings to a per-process temporary directory. Settings uses
    // IniFormat/UserScope, so without this every test that touches Settings::
    // (recent files, theme, language, autosave lists) reads and WRITES the
    // developer's real ~/.config/GIBIS-UNIFESP/wiRedPanda.ini. Must run before
    // the first QSettings instance is created (this function is called at the
    // top of runTestSuite(), before Application is constructed).
    static QTemporaryDir settingsDir;
    if (settingsDir.isValid()) {
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir.path());
    }

    // Redirect QStandardPaths::AppDataLocation (and the other locations Qt's test
    // mode covers) to a per-process sandbox. Without this, ExerciseTourResources::
    // managedContentDir() and anything else touching AppDataLocation creates and
    // writes into the developer's/CI runner's real
    // ~/.local/share/GIBIS-UNIFESP/wiRedPanda — confirmed accumulating thousands of
    // leaked autosave files and stray test-category folders across real test runs.
    QStandardPaths::setTestModeEnabled(true);

    Comment::setVerbosity(-1);
    Application::interactiveMode = false;   // Suppress UI dialogs in tests
    Application::renderingEnabled = false;  // Skip wire-geometry work — tests never paint
    Application::migrationEnabled = false;  // Preserve backward-compatibility test files as-is
}

void configureApp()
{
    QCoreApplication::setOrganizationName("GIBIS-UNIFESP");
    QCoreApplication::setApplicationName("wiRedPanda");
    QCoreApplication::setApplicationVersion(APP_VERSION);
}

std::unique_ptr<WorkSpace> createWorkspace()
{
    return std::make_unique<WorkSpace>();
}

QString examplesDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/../Examples/";
}

QString backwardCompatibilityDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/../Tests/BackwardCompatibility/";
}

QString cpuComponentsDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/IC/Components/";
}

QString arduinoExpectedDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/Arduino/";
}

QString systemVerilogExpectedDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/SystemVerilog/";
}

bool inputStatus(GraphicElement *elm, int port)
{
    auto *inputPort = elm->inputPort(port);
    if (!inputPort) {
        qFatal("FATAL ERROR in inputStatus():\n"
               "  Attempted to read input port %d from element %s\n"
               "  Element type: %s\n"
               "  Element has ZERO input ports!\n"
               "  This element may only have OUTPUT ports (e.g., InputSwitch)\n"
               "  Use outputStatus() to read output ports, or read downstream elements to verify signal propagation.",
               port, qPrintable(elm->objectName()), elm->metaObject()->className());
    }
    return inputPort->status() == Status::Active;
}

bool outputStatus(GraphicElement *elm, int port)
{
    auto *outputPort = elm->outputPort(port);
    if (!outputPort) {
        qFatal("FATAL ERROR in outputStatus():\n"
               "  Attempted to read output port %d from element %s\n"
               "  Element type: %s\n"
               "  Element has no output port at this index!\n"
               "  Check that the port index is within valid range for this element type",
               port, qPrintable(elm->objectName()), elm->metaObject()->className());
    }
    return outputPort->status() == Status::Active;
}

int countConnections(Scene *scene)
{
    int count = 0;
    auto items = scene->items();
    for (auto *item : std::as_const(items)) {
        if (qgraphicsitem_cast<Connection *>(item)) {
            ++count;
        }
    }
    return count;
}

QList<Connection *> sceneConnections(Scene *scene)
{
    QList<Connection *> connections;
    auto items = scene->items();
    for (auto *item : std::as_const(items)) {
        if (auto *conn = qgraphicsitem_cast<Connection *>(item)) {
            connections.append(conn);
        }
    }
    return connections;
}

void setMultiBitInput(const QVector<InputSwitch *> &inputs, int value)
{
    for (int i = 0; i < inputs.size(); ++i) {
        bool bit = (value >> i) & 1;
        inputs[i]->setOn(bit);
    }
}

void clockCycle(Simulation *simulation, InputSwitch *clk)
{
    if (!simulation) {
        qFatal("FATAL ERROR in clockCycle(): Simulation pointer is nullptr");
    }
    if (!clk) {
        qFatal("FATAL ERROR in clockCycle(): Clock switch pointer is nullptr");
    }
    // Complete clock pulse: ensure rising edge followed by falling edge
    // This guarantees a rising edge occurs regardless of initial state
    clk->setOn(true);   // Rising edge (triggers latch on edge-triggered flip-flops)
    simulation->update();
    clk->setOff();      // Falling edge (return to low for next cycle)
    simulation->update();
}

void initSrc(GraphicElement &elm) { elm.initSimulationVectors(0, 1); }
void initElm(GraphicElement &elm) { elm.initSimulationVectors(elm.inputSize(), elm.outputSize()); }

void clockToggle(Simulation *simulation, InputSwitch *clk)
{
    if (!simulation) {
        qFatal("FATAL ERROR in clockToggle(): Simulation pointer is nullptr");
    }
    if (!clk) {
        qFatal("FATAL ERROR in clockToggle(): Clock switch pointer is nullptr");
    }
    // Single edge toggle for fine-grained clock control
    clk->setOn(!clk->isOn());
    simulation->update();
}

bool pixmapHasInk(const QPixmap &pixmap)
{
    const QImage image = pixmap.toImage();
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(image.pixel(x, y)) != 0) {
                return true;
            }
        }
    }
    return false;
}

bool waitFor(const std::function<bool()> &pred, int timeoutMs)
{
    return QTest::qWaitFor(pred, timeoutMs);
}

AutoDismisser::AutoDismisser(std::function<bool(QWidget *)> handler)
{
    // Scan all top-level widgets rather than only activeModalWidget():
    // window activation is asynchronous on native platforms (macOS), so a
    // dialog can be visible and modal-blocking before it becomes "active".
    // Each handler applies its own widget filter — QMessageBox::about(), for
    // example, is shown NON-modal on native macOS, so a blanket modality
    // check here would skip it.
    m_timer.setInterval(10);
    QObject::connect(&m_timer, &QTimer::timeout, &m_timer, [this, handler = std::move(handler)] {
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget *w : widgets) {
            if (w->isVisible() && handler(w)) {
                ++m_dismissCount;
            }
        }
    });
    m_timer.start();
}

AutoDismisser AutoDismisser::closeAnyModal()
{
    return AutoDismisser([](QWidget *w) {
        // Message boxes included regardless of modality (macOS About boxes);
        // anything else only when it's actually modal-blocking.
        if (w->isModal() || qobject_cast<QMessageBox *>(w)) {
            w->close();
            return true;
        }
        return false;
    });
}

AutoDismisser AutoDismisser::acceptMessageBox()
{
    return AutoDismisser([](QWidget *w) {
        if (auto *msgBox = qobject_cast<QMessageBox *>(w)) {
            msgBox->accept();
            return true;
        }
        return false;
    });
}

AutoDismisser AutoDismisser::clickMessageBoxButton(const QString &text)
{
    return AutoDismisser([text](QWidget *w) {
        auto *msgBox = qobject_cast<QMessageBox *>(w);
        if (!msgBox) {
            return false;
        }
        const auto buttons = msgBox->buttons();
        for (auto *btn : buttons) {
            if (btn->text() == text) {
                btn->click();
                return true;
            }
        }
        return false;
    });
}

AutoDismisser AutoDismisser::answerMessageBox(QMessageBox::StandardButton answer)
{
    return AutoDismisser([answer](QWidget *w) {
        auto *msgBox = qobject_cast<QMessageBox *>(w);
        if (!msgBox) {
            return false;
        }
        if (auto *btn = msgBox->button(answer)) {
            btn->click();
            return true;
        }
        return false;
    });
}

QImage renderElementForComparison(QGraphicsScene *scene, GraphicElement *elm, QPoint &centerOut)
{
    // Render 1:1 from an integer-aligned source rect so every orientation rasterises scene
    // content at the same scale and sub-pixel phase — a fractional source origin would shift
    // antialiasing between renders and defeat pixel comparison.
    const QRect source = elm->sceneBoundingRect().toAlignedRect();
    QImage image(source.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    QPainter painter(&image);
    scene->render(&painter, QRectF(image.rect()), source);
    painter.end();

    // pixmapCenter() is already an absolute point in the element's local frame (it derives from
    // boundingRect().center() for procedural-render elements like IC/TruthTable) — no topLeft
    // offset needed on top of it.
    const QPointF centerScene = elm->mapToScene(elm->pixmapCenter());
    centerOut = (centerScene - source.topLeft()).toPoint();
    return image;
}

int alignedMismatch(const QImage &a, const QPoint ca, const QImage &b, const QPoint cb,
                     const int halfSize, const int tolerance)
{
    int best = std::numeric_limits<int>::max();

    for (int oy = -2; oy <= 2; ++oy) {
        for (int ox = -2; ox <= 2; ++ox) {
            int count = 0;
            for (int dy = -halfSize; dy < halfSize; ++dy) {
                for (int dx = -halfSize; dx < halfSize; ++dx) {
                    const QColor pa = a.pixelColor(ca + QPoint(dx, dy));
                    const QColor pb = b.pixelColor(cb + QPoint(dx + ox, dy + oy));
                    if (qAbs(pa.red() - pb.red()) > tolerance
                        || qAbs(pa.green() - pb.green()) > tolerance
                        || qAbs(pa.blue() - pb.blue()) > tolerance) {
                        ++count;
                    }
                }
            }
            best = qMin(best, count);
        }
    }

    return best;
}

QVector<DiffStep> generateDifferentialVectors(
    const QVector<InputSwitch *> &switches,
    const QVector<Led *> &leds,
    Simulation *sim,
    const int clockIdx,
    const QVector<int> &asyncLowIdx,
    const QVector<int> &asyncHighIdx,
    const quint32 seed,
    const int numSteps)
{
    QVector<DiffStep> steps;
    QRandomGenerator rng(seed);
    const QSet<int> asyncSet(asyncLowIdx.cbegin(), asyncLowIdx.cend());
    const QSet<int> asyncHighSet(asyncHighIdx.cbegin(), asyncHighIdx.cend());

    auto captureInputs = [&]() {
        QVector<int> bits;
        bits.reserve(switches.size());
        for (auto *sw : switches) {
            bits.append(sw->isOn() ? 1 : 0);
        }
        return bits;
    };
    auto captureOutputs = [&]() {
        QVector<int> bits;
        bits.reserve(leds.size());
        for (auto *led : leds) {
            bits.append(inputStatus(led, 0) ? 1 : 0);
        }
        return bits;
    };
    auto record = [&]() {
        steps.append({captureInputs(), captureOutputs()});
    };
    // A setup step drives inputs but records no expected output, so the testbench
    // replays it without checking. Used for the reset preamble, which brings a
    // freshly seeded comb-loop latch to a defined state both the engine and the
    // export agree on before the checked stimulus begins.
    auto recordSetup = [&]() {
        steps.append({captureInputs(), {}});
    };
    // Settle the engine to a fixed point after each input change before sampling.
    // A SINGLE update is not enough for gated-clock multi-cycle designs: the
    // combinational path that feeds the flip-flops (cycle-counter → decode →
    // enable) needs several passes to converge, and a non-settled D would be
    // captured at the next clock edge, drifting the engine's state one cycle off
    // the exported (non-blocking) SystemVerilog. The committed engine semantics
    // are validated against iverilog only at the settled fixed point, so we
    // reproduce the same multi-pass settle the original stimulus used.
    constexpr int kSettleIterations = 20;
    auto settle = [&]() {
        for (int u = 0; u < kSettleIterations; ++u) {
            sim->update();
        }
    };

    // Initial (unrecorded) settle: clock and data LOW, active-low async controls
    // HIGH (inactive). This matches the testbench's t=0 init.
    for (int i = 0; i < switches.size(); ++i) {
        switches[i]->setOn(asyncSet.contains(i));
    }
    settle();

    // Reset preamble (SETUP, not checked): a cross-coupled flip-flop powers on
    // into a free-running bistable state the engine and a faithful gate-level
    // export resolve to *opposite* values. The export's seeded latch regs start
    // at the metastable (0,0) point, and a *single* async assert can't climb out
    // of it (and a clock pulse there only corrupts it). Asserting BOTH controls
    // at once injects the input events that drive the latch off (0,0); releasing
    // all but one then leaves a single async control asserted, which forces a
    // defined state the engine and export agree on. No clock pulse is needed —
    // async preset/clear set the latch directly. These steps are replayed by the
    // testbench but not checked, so checks begin from the settled state.
    if (!asyncLowIdx.isEmpty()) {
        for (const int idx : asyncLowIdx) {
            switches[idx]->setOn(false);              // assert all (active-low)
        }
        settle();
        recordSetup();
        for (int i = 1; i < asyncLowIdx.size(); ++i) {
            switches[asyncLowIdx[i]]->setOn(true);    // release all but the first
        }
        settle();
        recordSetup();
        switches[asyncLowIdx.first()]->setOn(true);   // release; state now held
        settle();
        recordSetup();
    }

    // Active-high reset/seed preamble (SETUP, not checked): a counter or register
    // exposes an active-HIGH Reset/Init that drives its embedded flip-flops to a
    // defined state (0 for Reset, the seed pattern for Init). Without it the
    // run starts from the flip-flops' ambiguous power-on value, which the engine
    // (settles q=1) and a faithful gate-level export (settles q=0) resolve
    // oppositely. Pulse it HIGH then release so both begin the checked stimulus
    // from the same defined state; it stays inactive (LOW) for the random body.
    if (!asyncHighIdx.isEmpty()) {
        for (const int idx : asyncHighIdx) {
            switches[idx]->setOn(true);               // assert reset/seed
        }
        settle();
        recordSetup();
        for (const int idx : asyncHighIdx) {
            switches[idx]->setOn(false);              // release; state now held
        }
        settle();
        recordSetup();
    }

    for (int s = 0; s < numSteps; ++s) {
        // Randomise non-clock, non-async inputs (stable across the clock pulse so
        // an edge-triggered element captures a defined value).
        for (int i = 0; i < switches.size(); ++i) {
            if (i == clockIdx || asyncSet.contains(i) || asyncHighSet.contains(i)) {
                continue;
            }
            switches[i]->setOn((rng.generate() & 1U) != 0U);
        }
        // Hold async controls inactive during synchronous operation: the preamble
        // already established a defined state, so the random body exercises the
        // clocked data path. (Asserting async mid-run only adds metastable corners
        // where iverilog's comb-loop evaluation and the engine's settle diverge on
        // undefined behaviour — not export defects.)
        for (const int idx : asyncLowIdx) {
            switches[idx]->setOn(true);
        }
        // Hold active-high reset/seed inactive (LOW) during synchronous operation.
        for (const int idx : asyncHighIdx) {
            switches[idx]->setOn(false);
        }

        if (clockIdx >= 0) {
            // Change data ONLY while the clock is stable (currently LOW), then
            // pulse the clock. This keeps every clock edge — rising AND falling —
            // free of a simultaneous data change, so an edge-triggered element
            // captures deterministically (no setup-time race between the engine's
            // and the export's evaluation order). Sampling at clock-HIGH also
            // makes the check edge-polarity sensitive.
            settle();
            record();                       // new data, clock LOW
            switches[clockIdx]->setOn(true);
            settle();
            record();                       // rising edge (data stable)
            switches[clockIdx]->setOn(false);
            settle();
            record();                       // falling edge (data stable)
        } else {
            settle();
            record();
        }
    }

    return steps;
}
} // namespace TestUtils

// CircuitBuilder implementation

CircuitBuilder::CircuitBuilder(Scene *scene)
    : m_scene(scene)
{
}

// Trait implementations
int CircuitBuilder::InputPortTraits::count(GraphicElement *elm)
{
    return elm->inputSize();
}

Port *CircuitBuilder::InputPortTraits::port(GraphicElement *elm, int idx)
{
    return elm->inputPort(idx);
}

const char *CircuitBuilder::InputPortTraits::portType()
{
    return "input";
}

const char *CircuitBuilder::InputPortTraits::noPortsMessage()
{
    return "[No input ports available]";
}

int CircuitBuilder::OutputPortTraits::count(GraphicElement *elm)
{
    return elm->outputSize();
}

Port *CircuitBuilder::OutputPortTraits::port(GraphicElement *elm, int idx)
{
    return elm->outputPort(idx);
}

const char *CircuitBuilder::OutputPortTraits::portType()
{
    return "output";
}

const char *CircuitBuilder::OutputPortTraits::noPortsMessage()
{
    return "[No output ports available]";
}

// Generic template implementations
template<typename PortTraits>
int CircuitBuilder::portByLabelImpl(GraphicElement *element, const QString &label)
{
    if (!element) {
        qFatal("FATAL ERROR in CircuitBuilder::portByLabel(): Element is nullptr");
    }

    int portCount = PortTraits::count(element);
    for (int i = 0; i < portCount; ++i) {
        auto *port = PortTraits::port(element, i);
        if (port && port->name() == label) {
            return i;
        }
    }

    QString errorMsg = QString(
        "FATAL ERROR in CircuitBuilder::portByLabel():\n"
        "  %1 port '%2' not found on element '%3'\n"
        "  Available %1 ports:\n    %4"
    ).arg(PortTraits::portType(),
          label,
          element->objectName(),
          availablePortsImpl<PortTraits>(element));

    qFatal("%s", qPrintable(errorMsg));
    return -1;
}

template<typename PortTraits>
QString CircuitBuilder::availablePortsImpl(GraphicElement *element) const
{
    if (!element) {
        return "ERROR: Element is nullptr";
    }

    QStringList ports;
    int portCount = PortTraits::count(element);

    if (portCount == 0) {
        return PortTraits::noPortsMessage();
    }

    for (int i = 0; i < portCount; ++i) {
        auto *port = PortTraits::port(element, i);
        if (port) {
            QString portName = port->name().isEmpty() ? "(unnamed)" : port->name();
            ports.append(QString("[%1] %2").arg(i).arg(portName));
        } else {
            ports.append(QString("[%1] (port is null)").arg(i));
        }
    }

    return ports.join("\n    ");
}

void CircuitBuilder::addElement(GraphicElement *elm)
{
    if (!m_scene) {
        qFatal("FATAL ERROR in CircuitBuilder::addElement(): Scene is nullptr");
    }
    if (!elm) {
        qFatal("FATAL ERROR in CircuitBuilder::addElement(): Element is nullptr");
    }
    m_scene->addItem(elm);
}

Connection *CircuitBuilder::connect(GraphicElement *from, int fromPort,
                                        GraphicElement *to, int toPort)
{
    if (!m_scene) {
        qFatal("FATAL ERROR in CircuitBuilder::connect(): Scene is nullptr");
    }
    if (!from) {
        qFatal("FATAL ERROR in CircuitBuilder::connect(): 'from' element is nullptr");
    }
    if (!to) {
        qFatal("FATAL ERROR in CircuitBuilder::connect(): 'to' element is nullptr");
    }

    auto *fromOutput = from->outputPort(fromPort);
    if (!fromOutput) {
        qFatal("FATAL ERROR in CircuitBuilder::connect():\n"
               "  'from' element %s (type: %s) has no output port %d\n"
               "  Check that the port index is within valid range for this element type",
               qPrintable(from->objectName()),
               from->metaObject()->className(),
               fromPort);
    }

    auto *toInput = to->inputPort(toPort);
    if (!toInput) {
        qFatal("FATAL ERROR in CircuitBuilder::connect():\n"
               "  'to' element %s (type: %s) has no input port %d\n"
               "  Check that the port index is within valid range for this element type",
               qPrintable(to->objectName()),
               to->metaObject()->className(),
               toPort);
    }

    auto *conn = new Connection();
    conn->setStartPort(fromOutput);
    conn->setEndPort(toInput);
    m_scene->addItem(conn);
    return conn;
}

Simulation *CircuitBuilder::simulation() const
{
    if (!m_scene) {
        qWarning("WARNING in CircuitBuilder::simulation(): Scene is nullptr");
        return nullptr;
    }
    auto *sim = m_scene->simulation();
    if (!sim) {
        qWarning("WARNING in CircuitBuilder::simulation(): Scene has no simulation");
    }
    return sim;
}

Simulation *CircuitBuilder::initSimulation()
{
    if (!m_scene) {
        qFatal("FATAL ERROR in CircuitBuilder::initSimulation(): Scene is nullptr");
    }
    auto *sim = m_scene->simulation();
    if (!sim) {
        qFatal("FATAL ERROR in CircuitBuilder::initSimulation(): Scene has no simulation");
    }
    sim->initialize();
    return sim;
}

int CircuitBuilder::inputPortByLabel(GraphicElement *element, const QString &label)
{
    return portByLabelImpl<InputPortTraits>(element, label);
}

int CircuitBuilder::outputPortByLabel(GraphicElement *element, const QString &label)
{
    return portByLabelImpl<OutputPortTraits>(element, label);
}

Connection *CircuitBuilder::connect(GraphicElement *from, const QString &fromLabel,
                                        GraphicElement *to, int toPort)
{
    int fromPort = outputPortByLabel(from, fromLabel);
    return connect(from, fromPort, to, toPort);
}

Connection *CircuitBuilder::connect(GraphicElement *from, int fromPort,
                                        GraphicElement *to, const QString &toLabel)
{
    int toPortIndex = inputPortByLabel(to, toLabel);
    return connect(from, fromPort, to, toPortIndex);
}

Connection *CircuitBuilder::connect(GraphicElement *from, const QString &fromLabel,
                                        GraphicElement *to, const QString &toLabel)
{
    int fromPort = outputPortByLabel(from, fromLabel);
    int toPort = inputPortByLabel(to, toLabel);
    return connect(from, fromPort, to, toPort);
}
