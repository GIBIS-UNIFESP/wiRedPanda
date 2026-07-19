// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/DolphinModelBuilder.h"

#include <algorithm>

#include <QCoreApplication>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Scene/Scene.h"

namespace DolphinModelBuilder {

namespace {

/// Builds per-port row labels for \a elements: the element's label (or its translated type
/// name when blank), suffixed with "[port]" when the element exposes more than one port.
template <typename Elements, typename PortCount>
QStringList buildLabels(const Elements &elements, const PortCount &portCount)
{
    QStringList labels;

    for (auto *elm : elements) {
        QString label = elm->label();

        // Fall back to the element type name when the user hasn't given it a label
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(elm->elementType());
        }

        const int ports = portCount(elm);
        for (int port = 0; port < ports; ++port) {
            // Multi-port elements (e.g. bus inputs, multi-bit displays) get indexed labels
            if (ports > 1) {
                labels.append(label + "[" + QString::number(port) + "]");
            } else {
                labels.append(label);
            }
        }
    }

    return labels;
} // LCOV_EXCL_LINE -- compiler-generated QStringList cleanup for an exception path buildLabels() never takes (both template instantiations)

} // namespace

Signals collect(Scene *scene)
{
    Signals result;

    const auto elements = scene->elements();

    if (elements.isEmpty()) {
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "The circuit is empty. Add input and output elements to generate a waveform.");
    }

    for (auto *elm : elements) {
        // Not reachable: Scene::elements() (the only real caller) already filters to items
        // whose type() == GraphicElement::Type via qgraphicsitem_cast, so this can't be null
        // or the wrong type here — kept as a defensive guard against a future caller passing
        // a hand-built element list.
        if (!elm || (elm->type() != GraphicElement::Type)) {
            continue; // LCOV_EXCL_LINE
        }

        if (elm->elementGroup() == ElementGroup::Input) {
            result.inputs.append(qobject_cast<GraphicElementInput *>(elm));
            // Multi-bit inputs (e.g. rotary encoder) contribute multiple port rows
            result.inputPorts += elm->outputSize();
        }

        if (elm->elementGroup() == ElementGroup::Output) {
            result.outputs.append(elm);
        }
    }

    // Stable sort by label so the waveform table ordering is deterministic and
    // matches what the user expects from the label names they assigned
    std::stable_sort(result.inputs.begin(), result.inputs.end(), [](const auto &elm1, const auto &elm2) {
        return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) < 0;
    });

    std::stable_sort(result.outputs.begin(), result.outputs.end(), [](const auto &elm1, const auto &elm2) {
        return QString::compare(elm1->label(), elm2->label(), Qt::CaseInsensitive) < 0;
    });

    if (result.inputs.isEmpty() && result.outputs.isEmpty()) {
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "The circuit has no input or output elements. Add at least one input (e.g. Switch) and one output (e.g. LED) to generate a waveform.");
    }

    if (result.inputs.isEmpty()) {
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "The circuit has no input elements. Add at least one input (e.g. Switch, Button, or Clock) to generate a waveform.");
    }

    if (result.outputs.isEmpty()) {
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "The circuit has no output elements. Add at least one output (e.g. LED or Display) to generate a waveform.");
    }

    result.inputLabels  = buildLabels(result.inputs,  [](const GraphicElementInput *e) { return e->outputSize(); });
    result.outputLabels = buildLabels(result.outputs, [](const GraphicElement *e) { return e->inputSize(); });

    return result;
}

} // namespace DolphinModelBuilder
