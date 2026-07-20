// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Quick-native, Scene-free replacement for the portable subset of Tests/Common/
 * TestUtils.h's `TestUtils` namespace, plus Tests/Integration/IC/Tests/CpuTestUtils.h's
 * `CPUTestUtils` namespace -- used by the ported Level/CPU regression suite
 * (Tests/QuickShell/IC/), which cannot include Tests/Common/TestUtils.h itself: that header
 * pulls in <QApplication>/<QMessageBox> (Qt6::Widgets), and test_wiredpanda_quick deliberately
 * links zero Qt6::Widgets (see the qtquick-rewrite plan's "Current phase" section, Task 107).
 *
 * Deliberately reuses the `TestUtils`/`CPUTestUtils` namespace names: every call site in the
 * ported Level/CPU files stays textually identical to its Widgets-side original (only the
 * #include line and the CircuitBuilder/WorkSpace construction pattern change) -- safe because
 * test_wiredpanda and test_wiredpanda_quick are disjoint binaries that never both include this
 * file and the real Tests/Common/TestUtils.h in the same translation unit.
 *
 * Only the genuinely portable functions are here -- confirmed via grep across the whole
 * Tests/Integration/IC/ tree for which TestUtils::/CPUTestUtils:: symbols it actually calls.
 * countConnections()/sceneConnections() (take Scene*), AutoDismisser/configureApp()/
 * setupTestEnvironment() (QWidget/QMessageBox-based; QuickRunnerUtils.h's runQuickTestSuite()
 * already covers the equivalent env setup once per binary), renderElementForComparison()/
 * generateDifferentialVectors()/DiffStep (unused by this suite) are NOT ported here.
 */

#pragma once

#include <memory>

#include <QFileInfo>
#include <QTest>
#include <QVector>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Port.h"

#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string

namespace TestUtils {

/// Path to Tests/Integration/IC/Components/ -- identical construction to the Widgets-side
/// original, and CURRENTDIR is already defined identically for test_wiredpanda_quick (App/
/// QuickShell/CMakeLists.txt's own target_compile_definitions), so the .panda component files
/// don't need moving.
inline QString cpuComponentsDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/IC/Components/";
}

/// Path to Examples/ -- identical construction to Tests/Common/TestUtils.h's own examplesDir().
inline QString examplesDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/../Examples/";
}

inline bool inputStatus(GraphicElement *elm, int port = 0)
{
    auto *inputPort = elm->inputPort(port);
    if (!inputPort) {
        qFatal("FATAL ERROR in inputStatus(): element %s has no input port %d",
               qPrintable(elm->objectName()), port);
    }
    return inputPort->status() == Status::Active;
}

inline bool outputStatus(GraphicElement *elm, int port = 0)
{
    auto *outputPort = elm->outputPort(port);
    if (!outputPort) {
        qFatal("FATAL ERROR in outputStatus(): element %s has no output port %d",
               qPrintable(elm->objectName()), port);
    }
    return outputPort->status() == Status::Active;
}

inline void setMultiBitInput(const QVector<InputSwitch *> &inputs, int value)
{
    for (int i = 0; i < inputs.size(); ++i) {
        inputs[i]->setOn((value >> i) & 1);
    }
}

/// Reads N bits (LSB = elements[0]) from each element's input port \a port and combines them
/// into one integer -- identical semantics to the Widgets-side original.
template<typename T>
inline int readMultiBitOutput(const QVector<T *> &elements, int port = 0)
{
    int result = 0;
    for (int i = 0; i < elements.size(); ++i) {
        if (!elements[i]) {
            qFatal("FATAL ERROR in readMultiBitOutput(): element at index %d is nullptr", i);
        }
        if (inputStatus(static_cast<GraphicElement *>(elements[i]), port)) {
            result |= (1 << i);
        }
    }
    return result;
}

inline void clockCycle(Simulation *simulation, InputSwitch *clk)
{
    Q_ASSERT(simulation && clk);
    clk->setOn(true);
    simulation->update();
    clk->setOff();
    simulation->update();
}

inline void clockToggle(Simulation *simulation, InputSwitch *clk)
{
    Q_ASSERT(simulation && clk);
    clk->setOn(!clk->isOn());
    simulation->update();
}

/// New (no Widgets-side equivalent): under the old Scene-backed CircuitBuilder, every element a
/// test heap-`new`'d and passed into a CpuHelpers.h build*() function was implicitly
/// Scene-owned (QGraphicsScene::addItem() semantics), so the test itself never needed to track
/// its own cleanup. QuickCircuitBuilder::add() is deliberately non-owning (it also has to accept
/// stack-allocated elements -- see its own class doc comment), so a build*() function's own
/// internal add(param) call does NOT take ownership of caller-supplied elements either. A test
/// that still heap-`new`'s its InputSwitch/Led fixtures before calling into a build*() function
/// needs its own RAII for them -- this pool is that, kept independent of the builder's own
/// ownership tracking (calling both add() and addOwnedElement() on the same element would
/// double-register it in QuickCircuitBuilder::m_elements, corrupting simulationItems()).
class OwnedElementPool
{
public:
    template<typename T>
    T *make()
    {
        auto elm = std::make_unique<T>();
        T *raw = elm.get();
        m_pool.push_back(std::move(elm));
        return raw;
    }

private:
    std::vector<std::unique_ptr<GraphicElement>> m_pool;
};

} // namespace TestUtils

namespace CPUTestUtils {

inline constexpr int PATTERN_ALL_ZEROS = 0x00;
inline constexpr int PATTERN_ALL_ONES = 0xFF;
inline constexpr int PATTERN_ALTERNATING_1 = 0xAA;
inline constexpr int PATTERN_ALTERNATING_2 = 0x55;
inline constexpr int PATTERN_LOWER_NIBBLE = 0x0F;
inline constexpr int PATTERN_UPPER_NIBBLE = 0xF0;
inline constexpr int PATTERN_EVEN_BITS = 0x55;
inline constexpr int PATTERN_ODD_BITS = 0xAA;

template<size_t N>
inline int get8BitValue(Led *(&leds)[N])
{
    QVector<GraphicElement *> elements;
    for (size_t i = 0; i < N; ++i) {
        elements.append(leds[i]);
    }
    return TestUtils::readMultiBitOutput(elements, 0);
}

/// Loads a CPU building-block IC from Tests/Integration/IC/Components/ -- caller owns the
/// returned pointer (matches the Widgets-side original's contract exactly, including wrapping
/// a load failure as std::runtime_error rather than letting Pandaception propagate).
inline IC *loadBuildingBlockIC(const QString &filename)
{
    auto ic = std::make_unique<IC>();
    const QString absoluteIcPath = TestUtils::cpuComponentsDir() + filename;
    const QString icDirectory = QFileInfo(absoluteIcPath).absolutePath();
    try {
        ic->loadFile(absoluteIcPath, icDirectory);
    } catch (const Pandaception &e) {
        throw std::runtime_error(
            QString("Failed to load IC %1: %2").arg(filename, e.what()).toStdString());
    }
    return ic.release();
}

} // namespace CPUTestUtils
