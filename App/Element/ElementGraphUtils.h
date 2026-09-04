// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Pure, Widgets-free graph algorithms over element vectors -- extracted from Scene so
 * portable consumers (ArduinoCodeGen, SystemVerilogCodeGen) don't need to link against it.
 */

#pragma once

#include <QHash>
#include <QString>
#include <QVector>

class GraphicElement;
class InputPort;

/// Returns \a elements sorted in topological dependency order (inputs first).
QVector<GraphicElement *> sortByTopology(QVector<GraphicElement *> elements);

/**
 * \brief Returns a map from wireless channel label to the Tx node's input port.
 * \details Scans \a elements for nodes in WirelessMode::Tx, keyed by label.
 * If two Tx nodes share the same label the first one wins. Used by codegens
 * to resolve Rx node signals without duplicating the wireless scan.
 */
QHash<QString, InputPort *> wirelessTxInputPorts(const QVector<GraphicElement *> &elements);
