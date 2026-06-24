// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Owns a GraphicElement's input/output port vectors and their creation/resize lifecycle.
 */

#pragma once

#include <QString>
#include <QVector>

class GraphicElement;
class QNEPort;
class QNEInputPort;
class QNEOutputPort;

/**
 * \class ElementPorts
 * \brief Owns the input and output QNEPort vectors of a GraphicElement and the primitives
 * that create, resize and trim them.
 *
 * \details Extracted from GraphicElement, mirroring the ElementAppearance / ElementSimState /
 * ElementOrientation collaborators. Storage and port creation live here; the owning element
 * keeps the polymorphic geometry layout (updatePortsProperties is virtual and orientation-
 * coupled) and the min/max size policy, driving this type's resize primitives. New ports are
 * parented to the owner element so Qt's scene graph cleans them up with it.
 */
class ElementPorts
{
public:
    /// Constructs the port store bound to its owning \a owner element.
    explicit ElementPorts(GraphicElement *owner);

    // --- Read access ---

    /// Returns the input port vector (the pointers are non-const so callers can drive the ports).
    const QVector<QNEInputPort *> &inputs() const { return m_inputPorts; }

    /// Returns the output port vector.
    const QVector<QNEOutputPort *> &outputs() const { return m_outputPorts; }

    /// Returns the input port at \a index, or nullptr if out of range.
    QNEInputPort *inputPort(int index) const;

    /// Returns the output port at \a index, or nullptr if out of range.
    QNEOutputPort *outputPort(int index) const;

    /// Returns the current number of input ports.
    int inputSize() const;

    /// Returns the current number of output ports.
    int outputSize() const;

    /// Returns all input and output ports as a combined QNEPort list.
    QVector<QNEPort *> allPorts() const;

    // --- Mutation ---

    /// Replaces the input port vector with \a inputs.
    void setInputs(const QVector<QNEInputPort *> &inputs) { m_inputPorts = inputs; }

    /// Appends a new port (parented to the owner) with optional \a name; \a isOutput selects direction.
    void addPort(const QString &name, bool isOutput);

    /// Appends a new input port with optional \a name.
    void addInputPort(const QString &name = {}) { addPort(name, false); }

    /// Appends a new output port with optional \a name.
    void addOutputPort(const QString &name = {}) { addPort(name, true); }

    /// Grows (with default-named ports) or shrinks (deleting the trailing ports) the input
    /// list to exactly \a size. The caller re-lays-out the ports afterwards.
    void resizeInputs(int size);

    /// Grows or shrinks the output list to exactly \a size.
    void resizeOutputs(int size);

    /// Removes and returns the last input port WITHOUT deleting it (the serializer deletes it
    /// after cleaning the deserialization port map). Returns nullptr if empty.
    QNEInputPort *takeLastInput();

    /// Removes and returns the last output port WITHOUT deleting it. Returns nullptr if empty.
    QNEOutputPort *takeLastOutput();

private:
    GraphicElement *m_owner = nullptr;

    QVector<QNEInputPort *> m_inputPorts;   ///< All input ports owned by the element (ordered by index).
    QVector<QNEOutputPort *> m_outputPorts; ///< All output ports owned by the element (ordered by index).
};
