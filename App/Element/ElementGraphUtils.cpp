// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementGraphUtils.h"

#include <algorithm>

#include "App/Core/Enums.h"
#include "App/Core/Priorities.h"
#include "App/Element/GraphicElement.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

QVector<GraphicElement *> sortByTopology(QVector<GraphicElement *> elements)
{
    QHash<GraphicElement *, QVector<GraphicElement *>> successors;
    for (auto *elm : elements) {
        for (auto *port : elm->outputs()) {
            for (auto *conn : port->connections()) {
                if (auto *endPort = conn->endPort()) {
                    if (auto *successor = endPort->graphicElement()) {
                        auto &vec = successors[elm];
                        if (!vec.contains(successor)) {
                            vec.append(successor);
                        }
                    }
                }
            }
        }
    }

    QHash<GraphicElement *, int> priorities;
    calculatePriorities(elements, successors, priorities);

    std::stable_sort(elements.begin(), elements.end(), [&priorities](const auto &e1, const auto &e2) {
        return priorities.value(e1) > priorities.value(e2);
    });

    return elements;
}

QHash<QString, InputPort *> wirelessTxInputPorts(const QVector<GraphicElement *> &elements)
{
    QHash<QString, InputPort *> txMap;
    for (auto *elm : elements) {
        if (elm->wirelessMode() == WirelessMode::Tx && !elm->label().isEmpty() && elm->inputPort(0)) {
            if (!txMap.contains(elm->label())) {
                txMap.insert(elm->label(), elm->inputPort(0));
            }
        }
    }
    return txMap;
} // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned QHash<QString, InputPort *>, never reached after the return above.
