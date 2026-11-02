// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/KernelLogic.h"

#include <utility>

#include <QBitArray>

#include "App/Core/StatusOps.h"
#include "App/Element/Element.h"

namespace KernelLogicDetail {

// The gates fold every input, an Unknown or Error one included, so a dominating input decides (AND(0, X) = 0)

void andGate(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }
    element.setOutputValue(StatusOps::statusAndAll(element.simInputs()));
}

void orGate(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }
    element.setOutputValue(StatusOps::statusOrAll(element.simInputs()));
}

void nandGate(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }
    element.setOutputValue(StatusOps::statusNot(StatusOps::statusAndAll(element.simInputs())));
}

void norGate(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }
    element.setOutputValue(StatusOps::statusNot(StatusOps::statusOrAll(element.simInputs())));
}

void xorGate(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }
    element.setOutputValue(StatusOps::statusXorAll(element.simInputs()));
}

void xnorGate(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }
    element.setOutputValue(StatusOps::statusNot(StatusOps::statusXorAll(element.simInputs())));
}

void notGate(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }
    element.setOutputValue(StatusOps::statusNot(element.simInputs().at(0)));
}

// The memory kernels sample their data at their control inputs' levels or edges. A flip-flop's history
// (Element::kernelState()) holds its last clock, then the data inputs it last sampled: an edge latches what the data
// were at the previous evaluation, which gives the zero setup time.

/// Sets the asynchronous overrides, the active-low ~Preset and ~Clear, on \a q0 and \a q1: either one low decides.
void applyOverrides(const Status preset, const Status clear, Status &q0, Status &q1)
{
    if ((preset == Status::Inactive) || (clear == Status::Inactive)) {
        q0 = (preset == Status::Inactive) ? Status::Active : Status::Inactive;
        q1 = (clear == Status::Inactive) ? Status::Active : Status::Inactive;
    }
}

void dFlipFlop(Element &element)
{
    QVector<Status> &kept = element.kernelState(); // last clock, last D
    if (!element.simUpdateInputs()) {
        // Forget the last clock level while inputs are invalid: edge detection requires the last clock Inactive, so
        // recovery with the clock already high cannot fabricate a rising edge from stale state.
        kept[0] = Status::Unknown;
        return;
    }
    Status q0 = element.simOutputs().at(0);
    Status q1 = element.simOutputs().at(1);
    const Status d = element.simInputs().at(0);
    const Status clk = element.simInputs().at(1);

    if ((clk == Status::Active) && (kept[0] == Status::Inactive)) {
        q0 = kept[1];
        q1 = (kept[1] == Status::Active) ? Status::Inactive : Status::Active;
    }
    applyOverrides(element.simInputs().at(2), element.simInputs().at(3), q0, q1);
    kept[0] = clk;
    kept[1] = d;
    element.setOutputValue(0, q0);
    element.setOutputValue(1, q1);
}

void tFlipFlop(Element &element)
{
    QVector<Status> &kept = element.kernelState(); // last clock, last T
    if (!element.simUpdateInputs()) {
        kept[0] = Status::Unknown; // as the D flip-flop's
        return;
    }
    Status q0 = element.simOutputs().at(0);
    Status q1 = element.simOutputs().at(1);
    const Status t = element.simInputs().at(0);
    const Status clk = element.simInputs().at(1);

    if ((clk == Status::Active) && (kept[0] == Status::Inactive) && (kept[1] == Status::Active)) {
        q0 = (q0 == Status::Active) ? Status::Inactive : Status::Active;
        q1 = (q1 == Status::Active) ? Status::Inactive : Status::Active;
    }
    applyOverrides(element.simInputs().at(2), element.simInputs().at(3), q0, q1);
    kept[0] = clk;
    kept[1] = t;
    element.setOutputValue(0, q0);
    element.setOutputValue(1, q1);
}

void jkFlipFlop(Element &element)
{
    QVector<Status> &kept = element.kernelState(); // last clock, last J, last K
    if (!element.simUpdateInputs()) {
        kept[0] = Status::Unknown; // as the D flip-flop's
        return;
    }
    Status q0 = element.simOutputs().at(0);
    Status q1 = element.simOutputs().at(1);
    const Status j = element.simInputs().at(0);
    const Status clk = element.simInputs().at(1);
    const Status k = element.simInputs().at(2);

    if ((clk == Status::Active) && (kept[0] == Status::Inactive)) {
        if ((kept[1] == Status::Active) && (kept[2] == Status::Active)) {
            std::swap(q0, q1);
        } else if (kept[1] == Status::Active) {
            q0 = Status::Active;
            q1 = Status::Inactive;
        } else if (kept[2] == Status::Active) {
            q0 = Status::Inactive;
            q1 = Status::Active;
        }
    }
    applyOverrides(element.simInputs().at(3), element.simInputs().at(4), q0, q1);
    kept[0] = clk;
    kept[1] = j;
    kept[2] = k;
    element.setOutputValue(0, q0);
    element.setOutputValue(1, q1);
}

void srFlipFlop(Element &element)
{
    QVector<Status> &kept = element.kernelState(); // last clock, last S, last R
    if (!element.simUpdateInputs()) {
        kept[0] = Status::Unknown; // as the D flip-flop's
        return;
    }
    Status q0 = element.simOutputs().at(0);
    Status q1 = element.simOutputs().at(1);
    const Status s = element.simInputs().at(0);
    const Status clk = element.simInputs().at(1);
    const Status r = element.simInputs().at(2);

    if ((clk == Status::Active) && (kept[0] == Status::Inactive)) {
        if ((kept[1] == Status::Active) && (kept[2] == Status::Active)) {
            q0 = Status::Active;
            q1 = Status::Active;
        } else if (kept[1] != kept[2]) {
            q0 = kept[1];
            q1 = kept[2];
        }
    }
    applyOverrides(element.simInputs().at(3), element.simInputs().at(4), q0, q1);
    kept[0] = clk;
    kept[1] = s;
    kept[2] = r;
    element.setOutputValue(0, q0);
    element.setOutputValue(1, q1);
}

void dLatch(Element &element)
{
    if (!element.simUpdateInputs()) {
        return;
    }
    Status q0 = element.simOutputs().at(0);
    Status q1 = element.simOutputs().at(1);
    const Status d = element.simInputs().at(0);

    // Level-sensitive: Q follows D while Enable is high
    if (element.simInputs().at(1) == Status::Active) {
        q0 = d;
        q1 = (d == Status::Active) ? Status::Inactive : Status::Active;
    }
    element.setOutputValue(0, q0);
    element.setOutputValue(1, q1);
}

void srLatch(Element &element)
{
    if (!element.simUpdateInputs()) {
        return;
    }
    Status q0 = element.simOutputs().at(0);
    Status q1 = element.simOutputs().at(1);
    const Status set = element.simInputs().at(0);
    const Status reset = element.simInputs().at(1);

    if ((set == Status::Active) && (reset == Status::Active)) {
        q0 = Status::Inactive;
        q1 = Status::Inactive;
    } else if (set == Status::Active) {
        q0 = Status::Active;
        q1 = Status::Inactive;
    } else if (reset == Status::Active) {
        q0 = Status::Inactive;
        q1 = Status::Active;
    }
    element.setOutputValue(0, q0);
    element.setOutputValue(1, q1);
}

// The routing kernels read their select lines as a number, input 0 of them the least significant bit
// (Element::decodeSelectValue()). Any select line Unknown or Error makes the routing indeterminate, and so does a value
// that addresses no data input or output, possible whenever their count is not a power of two: what is routed then is
// Unknown, not a select line read back as data or an index past the ports.

void mux(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }
    const int selectLines = KernelLogic::muxSelectLines(element.inputSize());
    const int dataInputs = element.inputSize() - selectLines;
    for (int i = 0; i < selectLines; ++i) {
        const Status select = element.simInputs().at(dataInputs + i);
        if ((select == Status::Unknown) || (select == Status::Error)) {
            element.setOutputValue(select);
            return;
        }
    }

    const int selected = element.decodeSelectValue(dataInputs, selectLines);
    element.setOutputValue((selected < dataInputs) ? element.simInputs().at(selected) : Status::Unknown);
}

void demux(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }
    const int outputs = element.outputSize();
    const int selectLines = KernelLogic::demuxSelectLines(outputs);
    for (int i = 0; i < selectLines; ++i) {
        const Status select = element.simInputs().at(1 + i);
        if ((select == Status::Unknown) || (select == Status::Error)) {
            for (int output = 0; output < outputs; ++output) {
                element.setOutputValue(output, select);
            }
            return;
        }
    }

    const int selected = element.decodeSelectValue(1, selectLines);
    const Status data = element.simInputs().at(0);
    for (int output = 0; output < outputs; ++output) {
        if (selected >= outputs) {
            element.setOutputValue(output, Status::Unknown);
        } else {
            element.setOutputValue(output, (output == selected) ? data : Status::Inactive);
        }
    }
}

void truthTable(Element &element)
{
    if (!element.simUpdateInputsAllowUnknown()) {
        return;
    }

    // Any input Unknown or Error leaves the row undetermined
    for (const Status input : element.simInputs()) {
        if ((input == Status::Unknown) || (input == Status::Error)) {
            for (int output = 0; output < element.outputSize(); ++output) {
                element.setOutputValue(output, input);
            }
            return;
        }
    }

    // The row is the inputs read MSB-first (input 0 the most significant bit). The table holds a full column of rows,
    // one per pattern of the most inputs the type takes (its descriptor's, as the table's size is), for each output in
    // turn
    quint32 row = 0;
    for (const Status input : element.simInputs()) {
        row = (row << 1) | ((input == Status::Active) ? 1U : 0U);
    }
    const qsizetype rows = qsizetype{1} << element.descriptor().maxInputSize;
    const QBitArray &table = element.param<QBitArray>(ParamId::Table);
    for (int output = 0; output < element.outputSize(); ++output) {
        element.setOutputValue(output, table.at((rows * output) + static_cast<qsizetype>(row)));
    }
}

// The node passes its input on, an Unknown or Error one included; a wireless node keeps the level it carries
// (ParamId::Carried), which its indicator shows

void node(Element &element)
{
    const auto carry = [&element](const Status status) {
        if (element.wirelessMode() != WirelessMode::None) {
            element.setParam(ParamId::Carried, static_cast<int>(status));
        }
    };
    if (!element.simUpdateInputsAllowUnknown()) {
        carry(Status::Unknown);
        return;
    }
    const Status status = element.simInputs().at(0);
    element.setOutputValue(status);
    carry(status);
}

} // namespace KernelLogicDetail

KernelLogic::Logic KernelLogic::of(const KernelKind kind)
{
    switch (kind) {
    case KernelKind::And:  return &KernelLogicDetail::andGate;
    case KernelKind::Or:   return &KernelLogicDetail::orGate;
    case KernelKind::Nand: return &KernelLogicDetail::nandGate;
    case KernelKind::Nor:  return &KernelLogicDetail::norGate;
    case KernelKind::Xor:  return &KernelLogicDetail::xorGate;
    case KernelKind::Xnor: return &KernelLogicDetail::xnorGate;
    case KernelKind::Not:  return &KernelLogicDetail::notGate;

    case KernelKind::DLatch:     return &KernelLogicDetail::dLatch;
    case KernelKind::SRLatch:    return &KernelLogicDetail::srLatch;
    case KernelKind::DFlipFlop:  return &KernelLogicDetail::dFlipFlop;
    case KernelKind::TFlipFlop:  return &KernelLogicDetail::tFlipFlop;
    case KernelKind::JKFlipFlop: return &KernelLogicDetail::jkFlipFlop;
    case KernelKind::SRFlipFlop: return &KernelLogicDetail::srFlipFlop;

    case KernelKind::Mux:        return &KernelLogicDetail::mux;
    case KernelKind::Demux:      return &KernelLogicDetail::demux;
    case KernelKind::TruthTable: return &KernelLogicDetail::truthTable;

    case KernelKind::Node: return &KernelLogicDetail::node;

    // Not computed by logic: the sources drive their outputs themselves, and an IC is computed by its own subclass
    case KernelKind::None:
    case KernelKind::Instance:
    case KernelKind::Constant:
    case KernelKind::Input:
    case KernelKind::Clock:
        return nullptr;
    }
    return nullptr;
}

bool KernelLogic::isSource(const KernelKind kind)
{
    return (kind == KernelKind::Constant) || (kind == KernelKind::Input) || (kind == KernelKind::Clock);
}

int KernelLogic::muxSelectLines(const int inputs)
{
    int lines = 1;
    while ((1 << lines) < inputs - lines) {
        ++lines;
    }
    return lines;
}

int KernelLogic::demuxSelectLines(const int outputs)
{
    int lines = 1;
    while ((1 << lines) < outputs) {
        ++lines;
    }
    return lines;
}
