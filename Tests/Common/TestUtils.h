// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>

#include <QApplication>
#include <QDragEnterEvent>
#include <QImage>
#include <QMessageBox>
#include <QPixmap>
#include <QPoint>
#include <QTest>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
#  define QVERIFY_THROWS(exType, ...) QVERIFY_THROWS_EXCEPTION(exType, __VA_ARGS__)
#else
#  define QVERIFY_THROWS(exType, ...) QVERIFY_EXCEPTION_THROWN(__VA_ARGS__, exType)
#endif

/// QDragEnterEvent's QPoint-taking constructor is deprecated in favour of a new QPointF-taking
/// overload -- which doesn't exist on older Qt (confirmed against the actual installed headers:
/// absent in 6.9.3, present -- and the QPoint overload deprecated -- in 6.12.0), so neither type
/// works unmodified across the versions this project supports. The deprecation attribute names
/// "6.16" as its target release, but that label doesn't track when the QPointF overload actually
/// shipped in this toolchain, so the version gate below is pinned to the verified boundary
/// instead of that label.
#if QT_VERSION >= QT_VERSION_CHECK(6, 12, 0)
inline QDragEnterEvent makeDragEnterEvent(QPoint pos, Qt::DropActions actions, const QMimeData *data,
                                           Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
    return QDragEnterEvent(QPointF(pos), actions, data, buttons, modifiers);
}
#else
inline QDragEnterEvent makeDragEnterEvent(QPoint pos, Qt::DropActions actions, const QMimeData *data,
                                           Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
{
    return QDragEnterEvent(pos, actions, data, buttons, modifiers);
}
#endif

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"

class InputSwitch;
class Led;
class Connection;
class Simulation;
class QGraphicsScene;

/**
 * @brief Macro for accessing test data directory
 */
#define TEST_DATA_DIR (QString(QUOTE(CURRENTDIR)) + "/../Examples/")

#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string

namespace TestUtils {

/**
 * @brief Set up the test environment (headless mode on Linux, register types)
 */
void setupTestEnvironment();

/**
 * @brief Configure QApplication settings
 */
void configureApp();

/**
 * @brief Create a workspace with an initialized scene
 * @return Unique pointer to the workspace
 */
std::unique_ptr<WorkSpace> createWorkspace();

/**
 * @brief Get the path to the examples directory
 * @return QString path to examples
 */
QString examplesDir();

/**
 * @brief Get the path to the backward compatibility test directory
 * @return QString path to Tests/BackwardCompatibility/
 */
QString backwardCompatibilityDir();

/**
 * @brief Get the path to CPU test components directory
 * @return QString path to Integration/IC/Components/
 */
QString cpuComponentsDir();

/**
 * @brief Get the path to expected Arduino output directory
 * @return QString path to Integration/Arduino/
 */
QString arduinoExpectedDir();

/**
 * @brief Get the path to expected SystemVerilog output directory
 * @return QString path to Integration/SystemVerilog/
 */
QString systemVerilogExpectedDir();

/**
 * @brief Helper to get input status as bool
 * @param elm Element to check
 * @param port Port index (default 0)
 * @return true if port status is Active
 */
bool inputStatus(GraphicElement *elm, int port = 0);

/**
 * @brief Helper to get output status as bool
 * @param elm Element to check
 * @param port Port index (default 0)
 * @return true if port status is Active
 */
bool outputStatus(GraphicElement *elm, int port = 0);

/**
 * @brief Count the number of connections in a scene
 * @param scene Scene to count connections in
 * @return Number of Connection items in the scene
 */
int countConnections(Scene *scene);

/**
 * @brief Get all connections in a scene
 * @param scene Scene to get connections from
 * @return List of all Connection items in the scene
 */
QList<Connection *> sceneConnections(Scene *scene);

/**
 * @brief Set multiple input bits from an integer value
 *
 * Useful for sequential and memory tests where you need to set N bits at once.
 * Example: setMultiBitInput({sw0, sw1, sw2, sw3}, 0b1010) sets:
 *   sw0=0, sw1=1, sw2=0, sw3=1 (bit order matches array order)
 *
 * @param inputs Vector of InputSwitch pointers in bit order (index 0 = LSB)
 * @param value Integer value to decompose into bits
 */
void setMultiBitInput(const QVector<InputSwitch *> &inputs, int value);

/**
 * @brief Read multiple output bits from graphic elements
 *
 * Reads bit values from multiple elements' input ports and combines into
 * a single integer. Useful for reading results from multi-bit outputs.
 * Example: readMultiBitOutput({led0, led1, led2, led3}, 0) reads 4 bits
 *   Result bit pattern: led3|led2|led1|led0 (MSB to LSB)
 *
 * @param elements Vector of GraphicElement pointers in bit order (index 0 = LSB)
 * @param port Input port index to read from each element (default 0)
 * @return Combined integer value from all bits
 */
template<typename T>
inline int readMultiBitOutput(const QVector<T *> &elements, int port = 0)
{
    int result = 0;
    for (int i = 0; i < elements.size(); ++i) {
        if (!elements[i]) {
            qFatal("FATAL ERROR in readMultiBitOutput(): Element at index %d is nullptr", i);
        }
        if (inputStatus(static_cast<GraphicElement *>(elements[i]), port)) {
            result |= (1 << i);
        }
    }
    return result;
}

/**
 * @brief Execute a complete clock pulse (rising edge + falling edge)
 *
 * For sequential logic testing, this performs a complete clock cycle:
 * LOW → HIGH (rising edge, triggers latch) → LOW (falling edge, ready for next)
 *
 * Usage:
 *   InputSwitch clk;
 *   clockCycle(sim, &clk);  // Complete pulse: 0→1→0
 *   // Flip-flop has latched on the rising edge
 *
 * @param simulation Simulation to run
 * @param clk Clock input switch
 */
void clockCycle(Simulation *simulation, InputSwitch *clk);

/**
 * @brief Toggle the clock signal once (single edge)
 *
 * For cases that need fine-grained clock control (multi-phase clocking,
 * level-sensitive latches, testing specific clock states).
 *
 * Usage:
 *   InputSwitch clk;  // starts LOW
 *   clockToggle(sim, &clk);  // LOW → HIGH (rising edge)
 *   // ... do something while clock is HIGH ...
 *   clockToggle(sim, &clk);  // HIGH → LOW (falling edge)
 *
 * @param simulation Simulation to run
 * @param clk Clock input switch to toggle
 */
void clockToggle(Simulation *simulation, InputSwitch *clk);
/**
 * @brief Initialize a source element for direct-logic testing (0 inputs, 1 output)
 */
void initSrc(GraphicElement &elm);

/**
 * @brief Initialize an element for direct-logic testing using its own port counts
 */
void initElm(GraphicElement &elm);

/**
 * @brief True if any pixel of @a pixmap has non-zero alpha (i.e. paint() drew something)
 */
bool pixmapHasInk(const QPixmap &pixmap);

/**
 * @brief Waits (pumping the event loop) until @a pred returns true or @a timeoutMs elapses.
 *
 * Thin wrapper over QTest::qWaitFor. Preferred over the QTRY_* macros, whose
 * Qt 6.9 chrono-default internals trip this project's -Werror=conversion.
 */
bool waitFor(const std::function<bool()> &pred, int timeoutMs = 5000);

/**
 * @brief RAII poller that dismisses modal dialogs/message boxes for as long as it lives.
 *
 * Replaces the one-shot `QTimer::singleShot(0, ...)` + `activeModalWidget()`
 * dismissal pattern, which silently no-ops when the dialog isn't active yet —
 * leaving the dialog's exec() loop blocked until Qt Test's function watchdog
 * kills the binary (the TestDialogs hang on the Intel macOS runner). A
 * repeating timer polls every 10 ms and scans QApplication::topLevelWidgets()
 * for visible modal widgets, so it neither depends on window-activation timing
 * (asynchronous on native macOS) nor on being scheduled after the dialog opens.
 *
 * dismissCount() lets tests assert the dialog actually appeared:
 *
 *     TestUtils::AutoDismisser dismisser = TestUtils::AutoDismisser::closeAnyModal();
 *     triggerActionThatShowsDialog();
 *     QTRY_VERIFY(dismisser.dismissCount() >= 1);   // when the dialog is mandatory
 */
class AutoDismisser
{
public:
    /// @param handler Called for each visible modal widget found; return true if handled
    ///                (counts toward dismissCount()).
    explicit AutoDismisser(std::function<bool(QWidget *)> handler);

    /// Number of widgets the handler reported as handled so far.
    int dismissCount() const { return m_dismissCount; }

    // --- Convenience factories ---

    /// Dismisses any visible modal widget via close().
    static AutoDismisser closeAnyModal();
    /// Accepts any visible modal QMessageBox.
    static AutoDismisser acceptMessageBox();
    /// Clicks the button with the given text on any visible modal QMessageBox.
    static AutoDismisser clickMessageBoxButton(const QString &text);
    /// Clicks the given standard button on any visible modal QMessageBox.
    static AutoDismisser answerMessageBox(QMessageBox::StandardButton answer);

private:
    QTimer m_timer;
    int m_dismissCount = 0;
};

/**
 * @brief Renders @a elm's scene footprint into an image; @a centerOut receives the centre of
 * the element's body (where drawBody() centres its logo/icon) in image coordinates.
 */
QImage renderElementForComparison(QGraphicsScene *scene, GraphicElement *elm, QPoint &centerOut);

/**
 * @brief Compares @a halfSize-radius crops of @a a around @a ca and @a b around @a cb, trying
 * small alignment offsets so sub-pixel crop rounding can't fail the comparison. Returns the
 * smallest count of pixels whose channels differ by more than @a tolerance.
 */
int alignedMismatch(const QImage &a, const QPoint ca, const QImage &b, const QPoint cb,
                     int halfSize, int tolerance);

/**
 * @brief One step of an engine-differential test vector: the inputs driven and
 *        the outputs the wiRedPanda engine produced (the oracle).
 */
struct DiffStep {
    QVector<int> inputBits;   ///< Value driven on each input switch (caller order).
    QVector<int> expectedOut; ///< Engine output on each LED (caller order) — the reference.
};

/**
 * @brief Seeded-random differential stimulus driven through the engine oracle.
 *
 * Backend-agnostic: the returned steps feed either the SystemVerilog (iverilog)
 * or Arduino (simavr) testbench, which must reproduce @c expectedOut for every
 * step. Deterministic for a given @a seed (uses a local QRandomGenerator, not the
 * global one).
 *
 * For clocked circuits (@a clockIdx >= 0) each step randomizes the non-clock
 * inputs while the clock is LOW, then pulses the clock, sampling the engine three
 * times: with the new data (clock LOW), at the rising edge (clock HIGH, mid-pulse
 * — deterministically exercises clock-edge polarity), and at the falling edge
 * (clock LOW). The engine is settled to a fixed point before each sample. For
 * latches (@a clockIdx < 0) each step randomizes all inputs and samples once.
 *
 * Inputs listed in @a asyncLowIdx are treated as active-low async controls
 * (preset/clear). A cross-coupled flip-flop powers on into a free-running
 * bistable state the engine and a faithful gate-level export resolve oppositely,
 * so the run opens with a RECORDED reset preamble: assert ALL async controls
 * (the input events drive the export's seeded latch off its metastable (0,0)
 * point — a single assert can't, and a clock pulse there only corrupts it), then
 * release all but one, leaving one control asserted to force a defined state both
 * agree on. During random stimulus at most ONE of them is asserted per step —
 * never the forbidden both-low contention, whose release is undefined. The
 * synchronous data path (where edge polarity matters) is fully exercised.
 *
 * @param switches     Input switches (the IC's inputs, in codegen order).
 * @param leds         Output LEDs (the IC's outputs, in codegen order).
 * @param sim          Initialised simulation to drive.
 * @param clockIdx     Index of the clock input in @a switches, or -1 if none.
 * @param asyncLowIdx  Indices of active-low async controls (preset/clear).
 * @param asyncHighIdx Indices of active-high reset/seed controls (reset/init): a
 *                     boundary input that drives the embedded flip-flops to a
 *                     defined state when asserted HIGH. The preamble pulses them
 *                     to establish that state (which the engine and a faithful
 *                     gate-level export agree on) before the checked stimulus,
 *                     then holds them inactive (LOW) through the random body.
 * @param seed         RNG seed for reproducibility.
 * @param numSteps     Number of stimulus steps (each clocked step emits three samples).
 */
QVector<DiffStep> generateDifferentialVectors(
    const QVector<InputSwitch *> &switches,
    const QVector<Led *> &leds,
    Simulation *sim,
    int clockIdx,
    const QVector<int> &asyncLowIdx,
    const QVector<int> &asyncHighIdx,
    quint32 seed,
    int numSteps);

} // namespace TestUtils

/**
 * @brief Helper class for building test circuits with less boilerplate
 *
 * Usage:
 *   WorkSpace workspace;
 *   CircuitBuilder builder(workspace.scene());
 *
 *   InputSwitch sw1, sw2;
 *   And andGate;
 *   Led led;
 *
 *   builder.add(&sw1, &sw2, &andGate, &led);
 *   builder.connect(&sw1, 0, &andGate, 0);
 *   builder.connect(&sw2, 0, &andGate, 1);
 *   builder.connect(&andGate, 0, &led, 0);
 */
class CircuitBuilder
{
public:
    explicit CircuitBuilder(Scene *scene);

    /**
     * @brief Add a single element to the scene
     */
    void addElement(GraphicElement *elm);

    /**
     * @brief Add multiple elements to the scene (variadic template)
     */
    template<typename... Args>
    void add(Args *...elements)
    {
        (addElement(elements), ...);
    }

    /**
     * @brief Create a connection between two elements and add to scene
     * @param from Source element
     * @param fromPort Output port index
     * @param to Destination element
     * @param toPort Input port index
     * @return Pointer to created connection
     */
    Connection *connect(GraphicElement *from, int fromPort,
                           GraphicElement *to, int toPort);

    /**
     * @brief Get the scene
     */
    Scene *scene() const { return m_scene; }

    /**
     * @brief Get the simulation from the scene
     */
    Simulation *simulation() const;

    /**
     * @brief Initialize and return simulation
     */
    Simulation *initSimulation();

    /**
     * @brief Create a connection using output port label
     * @param from Source element
     * @param fromLabel Output port label on source element
     * @param to Destination element
     * @param toPort Input port index on destination element
     * @return Pointer to created connection
     */
    Connection *connect(GraphicElement *from, const QString &fromLabel,
                           GraphicElement *to, int toPort);

    /**
     * @brief Create a connection using input port label
     * @param from Source element
     * @param fromPort Output port index on source element
     * @param to Destination element
     * @param toLabel Input port label on destination element
     * @return Pointer to created connection
     */
    Connection *connect(GraphicElement *from, int fromPort,
                           GraphicElement *to, const QString &toLabel);

    /**
     * @brief Create a connection using both port labels
     * @param from Source element
     * @param fromLabel Output port label on source element
     * @param to Destination element
     * @param toLabel Input port label on destination element
     * @return Pointer to created connection
     */
    Connection *connect(GraphicElement *from, const QString &fromLabel,
                           GraphicElement *to, const QString &toLabel);

private:
    Scene *m_scene;

    /**
     * @brief Port direction traits for template-based port access
     */
    struct InputPortTraits
    {
        static int count(GraphicElement *elm);
        static class Port *port(GraphicElement *elm, int idx);
        static const char *portType();
        static const char *noPortsMessage();
    };

    struct OutputPortTraits
    {
        static int count(GraphicElement *elm);
        static class Port *port(GraphicElement *elm, int idx);
        static const char *portType();
        static const char *noPortsMessage();
    };

    /**
     * @brief Generic implementation for finding port by label
     * @tparam PortTraits Traits specifying input or output port access
     */
    template<typename PortTraits>
    int portByLabelImpl(GraphicElement *element, const QString &label);

    /**
     * @brief Generic implementation for getting available ports list
     * @tparam PortTraits Traits specifying input or output port access
     */
    template<typename PortTraits>
    QString availablePortsImpl(GraphicElement *element) const;

    /**
     * @brief Find input port index by label (for IC elements)
     * @param element The IC element
     * @param label Port label to search for
     * @return Port index (0-based)
     * @throws std::runtime_error if port not found
     */
    int inputPortByLabel(GraphicElement *element, const QString &label);

    /**
     * @brief Find output port index by label (for IC elements)
     * @param element The IC element
     * @param label Port label to search for
     * @return Port index (0-based)
     * @throws std::runtime_error if port not found
     */
    int outputPortByLabel(GraphicElement *element, const QString &label);
};
