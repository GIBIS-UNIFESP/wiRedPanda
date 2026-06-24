// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Owns a GraphicElement's rotation + flip orientation and the transform math.
 */

#pragma once

#include <QtGlobal>

class GraphicElement;
class Port;

/**
 * \class ElementOrientation
 * \brief Owns the rotation angle and horizontal/vertical flip flags of a GraphicElement,
 * plus the transform math that applies them to the item and its ports.
 *
 * \details Extracted from GraphicElement (the only heavy inline geometry it carried),
 * mirroring the ElementAppearance / ElementSimState collaborators. Rotatable elements spin
 * the whole QGraphicsItem and swap in a counter-oriented pixmap; non-rotatable elements keep
 * the pixmap fixed and orient only their ports about the element centre. The owning element
 * forwards its rotation()/setRotation()/isFlippedX()/... API here and reads orientation back
 * through the owner's public getters where needed (e.g. ElementAppearance).
 */
class ElementOrientation
{
public:
    /// Constructs the orientation bound to its owning \a owner element.
    explicit ElementOrientation(GraphicElement *owner);

    /// Returns the current rotation angle in degrees, kept in [0, 360).
    qreal angle() const { return m_angle; }

    /// Rotates the element to \a angle degrees and updates the item/ports accordingly.
    void setRotation(qreal angle);

    /// Returns true if the element is mirrored along the X axis (horizontal flip).
    bool isFlippedX() const { return m_flippedX; }

    /// Returns true if the element is mirrored along the Y axis (vertical flip).
    bool isFlippedY() const { return m_flippedY; }

    /// Sets the horizontal mirror state and reapplies the transform.
    void setFlippedX(bool flipped);

    /// Sets the vertical mirror state and reapplies the transform.
    void setFlippedY(bool flipped);

    /// Re-applies the combined rotation + flip orientation to every owner port (non-rotatable
    /// elements keep their pixmap fixed and move only their ports around the centre).
    void rotatePorts();

    /// Orients a single \a port for the current rotation + flip state. Applies Rotate(centre,
    /// angle) then Flip about the pixmap centre, so the port moves to the mirrored/rotated side
    /// while the element graphic stays upright. Recomputed from the flags each call (involutive).
    void orientPort(Port *port);

    // --- Deserialization (raw state, applied in one pass by applyLoadedOrientation) ---

    /// Sets the rotation angle without applying any transform (load path).
    void setAngleRaw(qreal angle) { m_angle = angle; }

    /// Sets the horizontal flip flag without applying any transform (load path).
    void setFlippedXRaw(bool flipped) { m_flippedX = flipped; }

    /// Sets the vertical flip flag without applying any transform (load path).
    void setFlippedYRaw(bool flipped) { m_flippedY = flipped; }

    /// Applies the full current orientation (rotation then flip) to the item and its ports.
    /// Called by the serializer after the raw angle/flip state has been restored and the ports
    /// repositioned, reproducing the historical setRotation()+applyFlipTransform() load sequence.
    void applyLoadedOrientation();

private:
    /// Recomputes the owner's QGraphicsItem transform from the current flip flags.
    void applyFlipTransform();

    GraphicElement *m_owner = nullptr;

    qreal m_angle = 0;    ///< Rotation angle in degrees, kept in [0, 360).
    bool m_flippedX = false; ///< Mirrored along the X axis.
    bool m_flippedY = false; ///< Mirrored along the Y axis.
};
