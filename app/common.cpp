// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"
#include "graphicelement.h"
#include "qneport.h"
#include "qneconnection.h"

Q_LOGGING_CATEGORY(zero,  "0")
Q_LOGGING_CATEGORY(one,   "1")
Q_LOGGING_CATEGORY(two,   "2")
Q_LOGGING_CATEGORY(three, "3")
Q_LOGGING_CATEGORY(four,  "4")
Q_LOGGING_CATEGORY(five,  "5")

void Comment::setVerbosity(const int verbosity)
{
    QString rules;

    switch (verbosity) {
    default:                         [[fallthrough]];
    case 0:  rules += "0 = false\n"; [[fallthrough]];
    case 1:  rules += "1 = false\n"; [[fallthrough]];
    case 2:  rules += "2 = false\n"; [[fallthrough]];
    case 3:  rules += "3 = false\n"; [[fallthrough]];
    case 4:  rules += "4 = false\n"; [[fallthrough]];
    case 5:  rules += "5 = false\n";
    }

    QLoggingCategory::setFilterRules(rules);

    qSetMessagePattern("%{if-debug}%{line}: %{function} => %{endif}%{message}");
}

Pandaception::Pandaception(const QString &message)
    : std::runtime_error(message.toStdString())
{
}

QVector<GraphicElement *> Common::sortGraphicElements(QVector<GraphicElement *> elements)
{
    QHash<GraphicElement *, bool> beingVisited;
    QHash<GraphicElement *, int> priorities;

    for (auto *elm : elements) {
        calculatePriority(elm, beingVisited, priorities);
    }

    std::sort(elements.begin(), elements.end(), [priorities](const auto &e1, const auto &e2) {
        return priorities.value(e1) > priorities.value(e2);
    });

    return elements;
}

int Common::calculatePriority(GraphicElement *elm, QHash<GraphicElement *, bool> &beingVisited, QHash<GraphicElement *, int> &priorities)
{
    if (!elm) {
        return 0;
    }

    if (beingVisited.contains(elm) && beingVisited.value(elm)) {
        return 0;
    }

    if (priorities.contains(elm)) {
        return priorities.value(elm);
    }

    beingVisited[elm] = true;
    int max = 0;

    for (auto *port : elm->outputs()) {
        for (auto *conn : port->connections()) {
            if (auto *successor = conn->endPort()) {
                max = qMax(calculatePriority(successor->graphicElement(), beingVisited, priorities), max);
            }
        }
    }

    int priority = max + 1;
    priorities[elm] = priority;
    beingVisited[elm] = false;
    return priority;
}
