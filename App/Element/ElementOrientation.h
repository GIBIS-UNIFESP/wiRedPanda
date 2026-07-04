// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Owns a GraphicElement's rotation angle and flip (mirror) state.
 */

#pragma once

#include <QtGlobal>

class GraphicElement;
class Port;

/**
 * \class ElementOrientation
 * \brief Owns the rotation angle and flip flags of a GraphicElement, and the transform
 * math that applies them to the item and its ports.
 *
 * \details Extracted from GraphicElement, mirroring the ElementAppearance / ElementSimState
 * pattern (owner back-pointer, used through the element's public interface). Rotatable
 * elements rotate the whole QGraphicsItem; non-rotatable elements keep their pixmap fixed
 * and instead re-transform each port individually — both paths are implemented here.
 */
class ElementOrientation
{
public:
    /// Constructs the orientation state bound to its owning \a owner element.
    explicit ElementOrientation(GraphicElement *owner)
        : m_owner(owner)
    {
    }

    /// Returns the current rotation angle in degrees.
    qreal angle() const { return m_angle; }

    /// Rotates to \a angle degrees (normalised to [0, 360)) and updates port positions.
    void setRotation(qreal angle);

    /// Returns \c true if this element is mirrored along the X axis (horizontal flip).
    bool isFlippedX() const { return m_flippedX; }

    /// Returns \c true if this element is mirrored along the Y axis (vertical flip).
    bool isFlippedY() const { return m_flippedY; }

    /// Sets the horizontal mirror state and updates the item transform.
    void setFlippedX(bool flipped);

    /// Sets the vertical mirror state and updates the item transform.
    void setFlippedY(bool flipped);

    /// Re-applies the combined rotation + flip orientation to every port. Used by
    /// non-rotatable elements, which keep their pixmap fixed and move only their
    /// ports around the centre.
    void rotatePorts();

    /**
     * \brief Orients \a port for the current rotation + flip state.
     * \details Used by non-rotatable elements, which keep their pixmap fixed. Applies
     * Rotate(centre, angle) then Flip about the pixmap centre to the port, so the port
     * moves to the mirrored/rotated side while the element graphic stays upright.
     * Recomputed from the flags each call, so it is involutive.
     */
    void orientPort(Port *port);

    // --- Raw setters: load-time only, no side effects (caller applies afterwards) ---

    /// Sets the angle without re-applying the transform. Load-time only.
    void setAngleRaw(qreal angle) { m_angle = angle; }
    /// Sets the horizontal flip flag without re-applying the transform. Load-time only.
    void setFlippedXRaw(bool flipped) { m_flippedX = flipped; }
    /// Sets the vertical flip flag without re-applying the transform. Load-time only.
    void setFlippedYRaw(bool flipped) { m_flippedY = flipped; }

    /// Applies the raw-loaded angle/flip state to the item, reproducing the historical
    /// setRotation()+applyFlipTransform() load sequence byte-for-byte.
    void applyLoadedOrientation()
    {
        setRotation(m_angle);
        applyFlipTransform();
    }

private:
    /// Recomputes the QGraphicsItem transform from the current flip flags.
    void applyFlipTransform();

    GraphicElement *m_owner;

    qreal m_angle = 0;
    bool m_flippedX = false;
    bool m_flippedY = false;
};
