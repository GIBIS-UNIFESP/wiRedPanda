// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementSimState.h"

#include "App/Element/GraphicElement.h"
#include "App/Wiring/Port.h"

void ElementSimState::reset(const QVector<OutputPort *> &outputPorts)
{
    // Reset each output slot to the port's power-on default so that the next
    // BeWavedDolphin sweep starts from a known, reproducible state.
    // Subclasses with internal edge-detection variables (flip-flops) override
    // GraphicElement::resetSimState() to also clear those fields.
    for (int i = 0; i < m_outputs.size(); ++i) {
        const Status def = (i < outputPorts.size())
                               ? outputPorts.at(i)->defaultValue()
                               : Status::Inactive;
        m_outputs[i] = (def == Status::Unknown) ? Status::Inactive : def;
    }
    m_outputChanged = true;
}

void ElementSimState::connectPredecessor(int inputIndex, GraphicElement *source, int outputPort)
{
    if (inputIndex >= m_connections.size()) {
        return;
    }
    m_connections[inputIndex] = {source, outputPort};
}

void ElementSimState::initVectors(int inputCount, int outputCount, const QVector<OutputPort *> &outputPorts)
{
    m_connections.fill({}, inputCount);
    m_inputs.fill(Status::Inactive, inputCount);
    m_outputs.resize(outputCount);
    m_staged.resize(outputCount);
    m_deferCommit = false;
    // Initialize outputs from port default statuses when they're explicitly set
    // (e.g., flip-flop Q'=Active), otherwise default to Inactive.
    // Using Inactive (not Unknown) ensures gate-level feedback loops can settle.
    for (int i = 0; i < outputCount; ++i) {
        if (i < outputPorts.size()) {
            const Status def = outputPorts.at(i)->defaultValue();
            m_outputs[i] = (def == Status::Unknown) ? Status::Inactive : def;
        } else {
            m_outputs[i] = Status::Inactive;
        }
    }
    m_outputChanged = false;
}

bool ElementSimState::updateInputs(bool allowUnknown, const QVector<InputPort *> &inputPorts)
{
    for (int index = 0; index < m_connections.size(); ++index) {
        auto *pred = m_connections.at(index).sourceElement;
        Status val;
        if (pred) {
            val = pred->outputValue(m_connections.at(index).sourceOutputIndex);
        } else {
            if (index < inputPorts.size() && inputPorts.at(index)->connections().size() > 1) {
                val = Status::Error;  // multi-driver bus conflict
            } else {
                // Unconnected input: use port's default status (replaces global GND/VCC).
                val = (index < inputPorts.size()) ? inputPorts.at(index)->defaultValue() : Status::Unknown;
            }
        }

        // allowUnknown: only fail for truly unconnected inputs that return Unknown.
        // !allowUnknown: fail for any Unknown or Error value.
        const bool shouldFail = allowUnknown ? (val == Status::Unknown && !pred)
                                             : (val == Status::Unknown || val == Status::Error);
        if (shouldFail) {
            for (auto &out : m_outputs) {
                if (out != Status::Unknown) {
                    m_outputChanged = true;
                }
                out = Status::Unknown;
            }
            return false;
        }
        m_inputs[index] = val;
    }
    return true;
}

int ElementSimState::decodeSelectValue(int offset, int count) const
{
    int selectValue = 0;
    for (int i = 0; i < count; i++) {
        if (m_inputs.at(offset + i) == Status::Active) {
            selectValue |= (1 << i);
        }
    }
    return selectValue;
}
