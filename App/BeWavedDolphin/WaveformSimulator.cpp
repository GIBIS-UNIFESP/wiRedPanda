// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/WaveformSimulator.h"

#include <utility>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Simulation/SimulationRecordingSuspender.h"
#include "App/Simulation/SimulationThrottleDisabler.h"

WaveformSimulator::WaveformSimulator(Scene *externalScene, Simulation *simulation)
    : m_externalScene(externalScene)
    , m_simulation(simulation)
{
}

QVector<Status> WaveformSimulator::captureInputs(const QVector<GraphicElementInput *> &inputs, const int inputPorts)
{
    QVector<Status> oldValues(inputPorts);
    int oldIndex = 0;

    for (auto *input : std::as_const(inputs)) {
        for (int port = 0; port < input->outputSize(); ++port) {
            // Snapshot the live port state before the simulation sweep overwrites it
            oldValues[oldIndex] = input->outputPort(port)->status();
            ++oldIndex;
        }
    }

    return oldValues;
}

void WaveformSimulator::restoreInputs(const QVector<GraphicElementInput *> &inputs, const QVector<Status> &saved)
{
    qCDebug(zero) << "Restoring old values to inputs, prior to simulation.";

    // `saved` is indexed by PORT (one entry per output port of each input element),
    // not by element. A separate port-level index is required to avoid reading the wrong
    // saved value for multi-port inputs (e.g. InputRotary).
    int portIndex = 0;
    for (auto *input : std::as_const(inputs)) {
        for (int port = 0; port < input->outputSize(); ++port) {
            const bool oldValue = static_cast<bool>(saved.at(portIndex++));
            if (input->outputSize() > 1) {
                input->setOn(oldValue, port);
            } else {
                input->setOn(oldValue);
            }
        }
    }
}

void WaveformSimulator::sweep(const QVector<GraphicElementInput *> &inputs,
                              const QVector<GraphicElement *> &outputs,
                              const int inputPorts, const int columns,
                              const std::function<bool(int row, int col)> &readInput,
                              const std::function<void(int row, int col, int value)> &writeOutput) const
{
    // Block the live simulation timer while we drive the circuit manually column by column
    qCDebug(zero) << "Creating class to pause main window simulator while creating waveform.";
    SimulationBlocker simulationBlocker(m_simulation);
    // Disable the visual refresh throttle so phases 3–4 (port-status updates) run on every
    // update() call. Without this, output port statuses are stale for most columns and the
    // waveform shows incorrect values.
    SimulationThrottleDisabler throttleDisabler(m_simulation);
    // The sweep drives m_simulation with synthetic test-vector inputs, not the live circuit
    // state. The beginTimedRun()/endTimedRun() bracket around this sweep already suspends
    // the shared WaveformRecorder itself (and preserves the live clock and recorded
    // history — the stimulus editor and the Live Analyzer share one Simulation per scene),
    // so this suspender is defense-in-depth for any caller driving sweep() outside such a
    // bracket; nested save/restore is harmless.
    SimulationRecordingSuspender recordingSuspender(m_simulation);

    // Reset every element's sequential state (Q/~Q outputs, edge-detection variables) to
    // power-on defaults before the sweep so results are reproducible regardless of any
    // prior simulation run that may have left flip-flops in a different state.
    qCDebug(zero) << "Resetting simulation state of all elements.";
    for (auto *elm : m_externalScene->elements()) {
        if (elm && elm->type() == GraphicElement::Type) {
            elm->resetSimState();
        }
    }
    // The event-driven engine tracks "what changed since last tick" via each element's
    // outputChanged() flag, set only on a value transition. resetSimState() above just
    // forced every element back to a baseline value without going through that change
    // detection, so the engine's incremental tracking no longer reflects reality — without
    // this, the next update() would only re-seed from inputs/clocks and silently miss
    // internal elements left stale by the reset.
    m_simulation->resetEventTracking();

    // --- Step through each time column and compute circuit outputs ---
    for (int column = 0; column < columns; ++column) {
        qCDebug(four) << "Itr: " << column << ", inputs: " << inputs.size();
        int row = 0;

        for (auto *input : std::as_const(inputs)) {
            for (int port = 0; port < input->outputSize(); ++port) {
                // Each input applies the cell value its own way (e.g. a rotary ignores lows).
                input->setWaveformValue(readInput(row++, column), port);
            }
        }

        qCDebug(four) << "Updating the values of the circuit logic based on current input values.";
        m_simulation->update();

        // Write computed output port states back into the model's output rows
        qCDebug(four) << "Setting the computed output values to the waveform results.";
        row = inputPorts;

        for (auto *output : std::as_const(outputs)) {
            for (int port = 0; port < output->inputSize(); ++port) {
                const int value = static_cast<int>(output->inputPort(port)->status());
                writeOutput(row, column, value);
                ++row;
            }
        }
    }
}
