// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Wiring/ConnectionValidity.h"

#include <QCoreApplication>

#include "App/Core/Enums.h"
#include "App/Element/GraphicElement.h"
#include "App/Wiring/Port.h"

bool isConnectionAllowed(OutputPort *startPort, InputPort *endPort)
{
    return connectionRejectionReason(startPort, endPort).isEmpty();
}

QString connectionRejectionReason(OutputPort *startPort, InputPort *endPort)
{
    // Same translation context ("ConnectionManager") as before this was extracted, so the
    // .ts catalogs keep the same source-string keys instead of churning on the move.
    if (!startPort || !endPort) {
        return QCoreApplication::translate("ConnectionManager", "This connection is not allowed.");
    }
    if (startPort->graphicElement() == endPort->graphicElement()) {
        return QCoreApplication::translate("ConnectionManager", "Can't connect an element to itself.");
    }
    if (startPort->isConnected(endPort)) {
        return QCoreApplication::translate("ConnectionManager", "These ports are already connected.");
    }
    // Rx nodes receive their signal over the air; a physical wire on the input
    // port would be silently overridden by the simulation (tunnel convention).
    if (auto *elm = endPort->graphicElement(); elm && elm->wirelessMode() == WirelessMode::Rx) {
        return QCoreApplication::translate("ConnectionManager", "This element receives wirelessly — no input wire needed.");
    }
    // Tx nodes are dead-end transmitters; their output port drives the wireless
    // channel only — no physical wire should bypass the channel (tunnel convention).
    if (auto *elm = startPort->graphicElement(); elm && elm->wirelessMode() == WirelessMode::Tx) {
        return QCoreApplication::translate("ConnectionManager", "This element transmits wirelessly — no output wire needed.");
    }
    return {};
}
