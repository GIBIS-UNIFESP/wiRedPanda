// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/Common.h"

// Logging categories are named "0"–"5" so they can be filtered numerically:
// level 0 = high-level events, level 5 = finest-grained tracing.
#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"

Q_LOGGING_CATEGORY(zero,  "0")
Q_LOGGING_CATEGORY(one,   "1")
Q_LOGGING_CATEGORY(two,   "2")
Q_LOGGING_CATEGORY(three, "3")
Q_LOGGING_CATEGORY(four,  "4")
Q_LOGGING_CATEGORY(five,  "5")

void Comment::setVerbosity(const int verbosity)
{
    QString rules;

    // Fall-through cascade: each case disables that level and all finer ones.
    // verbosity=5 → nothing disabled (all categories active);
    // verbosity=0 → all six categories disabled (silence everything);
    // negative / default → also disables all (used for production builds).
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

    // Include file line and function name in debug output for faster navigation.
    qSetMessagePattern("%{if-debug}%{line}: %{function} => %{endif}%{message}");
}

Pandaception::Pandaception(const QString &translatedMessage, const QString &englishMessage)
    // std::runtime_error carries the translated message so that what() shown in
    // the UI respects the current locale.  The English copy is kept separately
    // for crash-reporting backends (Sentry) that need a language-stable string.
    : std::runtime_error(translatedMessage.toStdString())
    , m_englishMessage(englishMessage)
{
}

QString Pandaception::englishMessage() const
{
    return m_englishMessage;
}

QVector<GraphicElement *> Common::sortGraphicElements(QVector<GraphicElement *> elements)
{
    QMap<GraphicElement *, bool> beingVisited;
    QMap<GraphicElement *, int> priorities;

    for (auto *elm : elements) {
        calculatePriority(elm, beingVisited, priorities);
    }

    // Higher priority = closer to a circuit input (no predecessors) = must be updated first.
    // std::stable_sort preserves the original relative order of equal-priority elements,
    // which avoids non-deterministic behaviour for unconnected or parallel elements.
    std::stable_sort(elements.begin(), elements.end(), [priorities](const auto &e1, const auto &e2) {
        return priorities.value(e1) > priorities.value(e2);
    });

    return elements;
}

int Common::calculatePriority(GraphicElement *elm, QMap<GraphicElement *, bool> &beingVisited, QMap<GraphicElement *, int> &priorities)
{
    if (!elm) {
        return 0;
    }

    // Cycle detection: if we encounter an element that is currently on the DFS stack,
    // we have a feedback loop. Return 0 to break the recursion; the element will be
    // assigned its priority from the non-cyclic part of its subgraph on the way back up.
    if (beingVisited.contains(elm) && beingVisited.value(elm)) {
        return 0;
    }

    // Memoisation: if the priority was already computed during a previous DFS branch,
    // return it immediately to avoid redundant recomputation in diamond-shaped topologies
    if (priorities.contains(elm)) {
        return priorities.value(elm);
    }

    beingVisited[elm] = true;
    int max = 0;

    // Recurse into each downstream (successor) element; the deepest successor determines
    // this element's priority so that inputs always have a higher number than outputs
    for (auto *port : elm->outputs()) {
        for (auto *conn : port->connections()) {
            if (auto *successor = conn->endPort()) {
                auto *successorElement = successor->graphicElement();
                if (successorElement) {
                    max = qMax(calculatePriority(successorElement, beingVisited, priorities), max);
                }
            }
        }
    }

    // Priority = depth from this element to the furthest output in the circuit + 1.
    // Elements with no outputs (leaf outputs) get priority 1; inputs get the highest value.
    int priority = max + 1;
    priorities[elm] = priority;
    beingVisited[elm] = false; // clear the "in-stack" flag before returning
    return priority;
}
