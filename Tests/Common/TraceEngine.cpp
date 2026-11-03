// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/TraceEngine.h"

#include "App/Element/Element.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationThrottleDisabler.h"
#include "App/Wiring/Port.h"

namespace {

/// master's simulation, stepped by hand: one update() per step, every phase complete whatever the interactive mode
/// says.
class MasterTraceEngine final : public TraceEngine
{
public:
    explicit MasterTraceEngine(Scene *scene)
        : m_simulation(scene->simulation())
        , m_noThrottle(m_simulation)
    {
    }

    void step() override { m_simulation->update(); }

    [[nodiscard]] bool timerRuns() const override { return m_simulation->isRunning(); }

    /**
     * \brief What \a element drives, read from its output ports, or, when it drives nothing, what its input ports
     * carry, which is what a display or an Led paints from: the 7-, 14- and 16-segment displays have no logic of their
     * own, so nothing refreshes their simulation inputs.
     */
    [[nodiscard]] QVector<Status> observed(const Element *element) const override
    {
        QVector<Status> values;
        if (element->outputSize() > 0) {
            for (int index = 0; index < element->outputSize(); ++index) {
                values << element->outputPort(index)->status();
            }
            return values;
        }
        for (int index = 0; index < element->inputSize(); ++index) {
            values << element->inputPort(index)->status();
        }
        return values;
    }

private:
    Simulation *m_simulation;
    SimulationThrottleDisabler m_noThrottle;
};

} // namespace

std::unique_ptr<TraceEngine> makeMasterTraceEngine(Scene *scene)
{
    return std::make_unique<MasterTraceEngine>(scene);
}
