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

/// Appends one Row per PORT of each element in \a elements to \a rows, labelling each with
/// the element's label (or its translated type name when blank), suffixed with "[port]" when
/// the element exposes more than one port. This is the only place the table's row layout is
/// decided; the sweep, the snapshot API and the header labels all read the result.
template <typename Elements, typename PortCount>
void appendRows(QVector<Row> &rows, const Elements &elements, const RowKind kind, const PortCount &portCount)
{
    for (auto *elm : elements) {
        QString label = elm->label();

        // Fall back to the element type name when the user hasn't given it a label
        if (label.isEmpty()) {
            label = ElementFactory::translatedName(elm->elementType());
        }

        const int ports = portCount(elm);
        for (int port = 0; port < ports; ++port) {
            // Multi-port elements (e.g. bus inputs, multi-bit displays) get indexed labels
            const QString rowLabel = (ports > 1) ? label + "[" + QString::number(port) + "]" : label;
            rows.append(Row{elm, port, kind, rowLabel});
        }
    }
} // LCOV_EXCL_LINE -- compiler-generated cleanup for an exception path appendRows() never takes (both template instantiations)

/// Projects the labels of every \a kind row out of \a rows, preserving order.
QStringList labelsOf(const QVector<Row> &rows, const RowKind kind)
{
    QStringList labels;
    for (const auto &row : rows) {
        if (row.kind == kind) {
            labels.append(row.label);
        }
    }
    return labels;
} // LCOV_EXCL_LINE -- compiler-generated QStringList cleanup for an exception path labelsOf() never takes

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

    // Rows first -- they are the source of truth; the label lists are projections of them.
    appendRows(result.rows, result.inputs,  RowKind::Input,  [](const GraphicElementInput *e) { return e->outputSize(); });
    appendRows(result.rows, result.outputs, RowKind::Output, [](const GraphicElement *e) { return e->inputSize(); });

    result.inputLabels  = labelsOf(result.rows, RowKind::Input);
    result.outputLabels = labelsOf(result.rows, RowKind::Output);
    // Derived, not accumulated separately: the input rows ARE inputLabels, so counting ports in
    // the collection loop would be a second computation of the same quantity by different code --
    // the duplication this row layout exists to remove.
    result.inputPorts   = static_cast<int>(result.inputLabels.size());

    return result;
}

} // namespace DolphinModelBuilder
