// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementOrientation.h"

#include <cmath>

#include <QTransform>

#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEPort.h"

ElementOrientation::ElementOrientation(GraphicElement *owner)
    : m_owner(owner)
{
}

void ElementOrientation::setRotation(const qreal angle)
{
    // Keep angle in [0, 360) to avoid accumulated floating-point drift across many rotations
    m_angle = std::fmod(angle, 360);
    // Rotatable elements rotate the entire QGraphicsItem (pixmap + ports move together), then
    // swap in a pixmap whose baked-in SVG <text> is counter-rotated so the labels stay upright.
    // Non-rotatable elements (inputs/outputs) keep the pixmap fixed and only spin ports
    // around the element centre so connections track the correct positions.
    if (m_owner->rotatesGraphic()) {
        m_owner->QGraphicsItem::setRotation(m_angle);
        m_owner->reapplyAppearanceOrientation();
    } else {
        rotatePorts();
    }
}

void ElementOrientation::rotatePorts()
{
    for (auto *port : m_owner->allPorts()) {
        orientPort(port);
    }
}

void ElementOrientation::orientPort(QNEPort *port)
{
    // Non-rotatable elements keep their pixmap fixed and instead transform each port about the
    // pixmap centre, so the port moves to the rotated/mirrored side while the graphic stays
    // upright. The single combined transform encodes both rotation and the flip flags, so all
    // three triggers (updatePortsProperties, setRotation, setFlippedX/Y) produce the same result.
    port->setRotation(0);
    port->setTransformOriginPoint(QPointF());

    // pixmapCenter() expressed in the port's local frame. port->pos() is the port's base
    // position — flip and rotation use the transform and never move pos — so this is stable
    // and independent of any transform already on the port (keeps the operation involutive).
    const QPointF origin = m_owner->pixmapCenter() - port->pos();

    QTransform t;
    t.translate(origin.x(), origin.y());
    t.rotate(m_angle);
    t.scale(m_flippedX ? -1 : 1, m_flippedY ? -1 : 1);
    t.translate(-origin.x(), -origin.y());
    port->setTransform(t);

    port->updateConnections();
}

void ElementOrientation::applyLoadedOrientation()
{
    // Apply the deserialized angle after ports are positioned so any non-rotatable element
    // can apply its own rotatePorts() path correctly, then apply the flip on top. This
    // reproduces the historical setRotation(m_angle) + applyFlipTransform() load sequence.
    setRotation(m_angle);
    applyFlipTransform();
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

    if (!m_flippedX && !m_flippedY) {
        m_owner->setTransform(QTransform());
    } else {
        const auto center = m_owner->pixmapCenter();
        QTransform t;
        t.translate(center.x(), center.y());
        t.scale(m_flippedX ? -1 : 1, m_flippedY ? -1 : 1);
        t.translate(-center.x(), -center.y());
        m_owner->setTransform(t);
    }

    // The item transform above mirrors the whole pixmap (and moves the ports). Swap in a pixmap
    // whose baked-in SVG <text> is pre-counter-oriented so the glyphs read upright after the
    // rotation + flip while still moving to the opposite side. No-op for an upright, unflipped or
    // non-SVG pixmap.
    m_owner->reapplyAppearanceOrientation();
    m_owner->update();
}
