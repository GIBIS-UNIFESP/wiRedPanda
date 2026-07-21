// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementOrientation.h"

#include <cmath>

#include "App/Element/GraphicElement.h"
#include "App/Wiring/Port.h"

void ElementOrientation::setRotation(const qreal angle)
{
    // Keep angle in [0, 360) to avoid accumulated floating-point drift across many rotations
    m_angle = std::fmod(angle, 360);
    // Rotatable elements rotate the entire graphic (pixmap + ports move together, computed
    // fresh by GraphicElement::elementTransform()/Port::scenePos() -- there's no stored
    // transform to update here anymore). Non-rotatable elements (inputs/outputs) keep the
    // pixmap fixed and only spin ports around the element centre so connections track the
    // correct positions.
    if (m_owner->rotatesGraphic()) {
        m_owner->reapplyAppearanceOrientation();
        m_owner->update();
    } else {
        rotatePorts();
    }
}

void ElementOrientation::rotatePorts()
{
    for (auto *port : m_owner->inputs())  { orientPort(port); }
    for (auto *port : m_owner->outputs()) { orientPort(port); }
}

void ElementOrientation::orientPort(Port *port)
{
    // Non-rotatable elements keep their pixmap fixed and instead transform each port about
    // the pixmap centre -- but Port::scenePos() now computes that fresh from the owning
    // element's current rotation/flip state on every call (oracle-validated formula,
    // qtquick-rewrite plan Phase 8a), rather than caching a per-port transform here. The only
    // remaining real work is notifying attached wires that this port's effective position
    // just changed.
    port->updateConnections();
}

void ElementOrientation::setFlippedX(const bool flipped)
{
    m_flippedX = flipped;
    applyFlipTransform();
}

void ElementOrientation::setFlippedY(const bool flipped)
{
    m_flippedY = flipped;
    applyFlipTransform();
}

void ElementOrientation::applyFlipTransform()
{
    // Non-rotatable elements (inputs/outputs) keep their pixmap fixed and mirror only their
    // ports — exactly as rotation does for them — so a flipped pushbutton/display never renders
    // its graphic reversed. rotatePorts() re-applies the combined rotation+flip orientation.
    if (!m_owner->rotatesGraphic()) {
        rotatePorts();
        return;
    }

    // The flip state (m_flippedX/m_flippedY) is already the single source of truth
    // GraphicElement::elementTransform() reads directly -- nothing to separately "set" on the
    // owner here anymore. Swap in a pixmap whose baked-in SVG <text> is pre-counter-oriented
    // so the glyphs read upright after the rotation + flip while still moving to the opposite
    // side. No-op for an upright, unflipped or non-SVG pixmap.
    m_owner->reapplyAppearanceOrientation();
    m_owner->update();
}
