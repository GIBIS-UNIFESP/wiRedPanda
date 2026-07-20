// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/ConnectionManager.h"

#include "App/Element/GraphicElement.h"
#include "App/Wiring/Port.h"

bool ConnectionManager::isConnectionAllowed(OutputPort *startPort, InputPort *endPort)
{
    return connectionRejectionReason(startPort, endPort).isEmpty();
}

QString ConnectionManager::connectionRejectionReason(OutputPort *startPort, InputPort *endPort)
{
    if (!startPort || !endPort) {
        return tr("This connection is not allowed.");
    }
    if (startPort->graphicElement() == endPort->graphicElement()) {
        return tr("Can't connect an element to itself.");
    }
    if (startPort->isConnected(endPort)) {
        return tr("These ports are already connected.");
    }
    // Rx nodes receive their signal over the air; a physical wire on the input
    // port would be silently overridden by the simulation (tunnel convention).
    if (auto *elm = endPort->graphicElement(); elm && elm->wirelessMode() == WirelessMode::Rx) {
        return tr("This element receives wirelessly — no input wire needed.");
    }
    // Tx nodes are dead-end transmitters; their output port drives the wireless
    // channel only — no physical wire should bypass the channel (tunnel convention).
    if (auto *elm = startPort->graphicElement(); elm && elm->wirelessMode() == WirelessMode::Tx) {
        return tr("This element transmits wirelessly — no output wire needed.");
    }
    return {};
}
