// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Shared string and circuit-topology utilities used by the code generators.
 */

#pragma once

#include <algorithm>

#include <QHash>
#include <QRegularExpression>
#include <QString>
#include <QVector>

#include "App/Core/Enums.h"
#include "App/Core/Priorities.h"
#include "App/Element/GraphicElement.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace CodeGenUtils {

/// Strips Unicode diacritic marks (accents) from \a input using NFC → NFD decomposition.
inline QString stripAccents(const QString &input)
{
    QString normalized = input.normalized(QString::NormalizationForm_D);
    static const QRegularExpression diacriticMarks("[\\p{Mn}]");
    return normalized.remove(diacriticMarks);
}

/**
 * \brief Converts \a input into a legal language identifier.
 *
 * Lowercases, trims whitespace, replaces spaces and dashes with underscores,
 * removes all remaining non-word characters, prepends '_' if the result starts
 * with a digit, and returns "_unnamed" for empty results.
 *
 * \param input       The raw string to sanitise.
 * \param stripFirst  If \c true, accent-strips \a input before processing
 *                    (required for Arduino C++ identifiers; not needed for SystemVerilog).
 */
inline QString removeForbiddenChars(const QString &input, const bool stripFirst = false)
{
    QString result = (stripFirst ? stripAccents(input) : input).toLower().trimmed().replace(' ', '_').replace('-', '_');
    result.erase(std::remove_if(result.begin(), result.end(),
                                [](const QChar c) { return !QChar::isLetterOrNumber(c.unicode()) && c != QLatin1Char('_'); }),
                 result.end());

    if (result.isEmpty()) {
        result = "_unnamed";
    } else if (result[0].isDigit()) {
        result.prepend('_');
    }

    return result;
}

/**
 * \brief Makes \a input safe to embed in a single-line "//" comment.
 *
 * Neutralizes embedded line breaks. Without this, a label containing "\n" (settable via a
 * crafted .panda file's label field, or the MCP create_element/set_element_properties "label"
 * parameter — neither restricts content beyond length) could break out of the comment and
 * inject an arbitrary line into the generated Arduino/SystemVerilog file.
 */
inline QString sanitizeComment(const QString &input)
{
    QString result = input;
    result.replace(QLatin1Char('\r'), QLatin1Char(' '));
    result.replace(QLatin1Char('\n'), QLatin1Char(' '));
    return result;
}

/**
 * \brief Orders \a elements by simulation-priority (sources before sinks), highest first.
 *
 * \details Moved here from Scene::sortByTopology() when GraphicElement/Port/Connection
 * stopped being QGraphicsItems -- the algorithm itself was already Widgets-free, only its
 * old home wasn't. Used by ArduinoCodeGen to emit update statements in dependency order.
 */
inline QVector<GraphicElement *> sortByTopology(QVector<GraphicElement *> elements)
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

/**
 * \brief Maps each wireless Tx node's channel label to its (single) input port.
 *
 * \details Moved here from Scene::wirelessTxInputPorts() -- see sortByTopology()'s doc
 * comment for why. Used by ArduinoCodeGen/SystemVerilogCodeGen to route a wireless Rx
 * node's generated signal to the matching Tx node's input by channel name.
 */
inline QHash<QString, InputPort *> wirelessTxInputPorts(const QVector<GraphicElement *> &elements)
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
}

}  // namespace CodeGenUtils
