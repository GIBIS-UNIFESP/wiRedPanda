// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/KernelLogic.h"

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

    // Computed by their families' subclasses until those go (P3.23)
    case KernelKind::Node:
    case KernelKind::Mux:
    case KernelKind::Demux:
    case KernelKind::TruthTable:
    case KernelKind::DLatch:
    case KernelKind::SRLatch:
    case KernelKind::DFlipFlop:
    case KernelKind::TFlipFlop:
    case KernelKind::JKFlipFlop:
    case KernelKind::SRFlipFlop:
        return nullptr;

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
