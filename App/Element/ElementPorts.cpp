// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementPorts.h"

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Wiring/Port.h"

ElementPorts::~ElementPorts()
{
    // Empty the member vectors *before* deleting anything: a port's destructor drains its
    // connections, which can re-enter GraphicElement::portsBoundingRect() (detaching a
    // connection recomputes the connection's path, which reads both endpoints' scenePos(),
    // which calls back into the owning element's boundingRect()) -- and portsBoundingRect()
    // iterates inputs()/outputs(), i.e. these very vectors. Deleting in place (qDeleteAll(m_inputPorts)
    // then qDeleteAll(m_outputPorts), each left populated with now-dangling pointers until the
    // whole call returns) lets that reentrant portsBoundingRect() dereference an
    // already-freed port -- a real heap-use-after-free, ASan-confirmed. Swapping each vector
    // to empty first means any such reentrant call sees no ports at all instead of dangling
    // ones; the resulting empty-rect answer is harmless since nothing durable depends on a
    // mid-teardown geometry query.
    QVector<InputPort *> inputs;
    inputs.swap(m_inputPorts);
    QVector<OutputPort *> outputs;
    outputs.swap(m_outputPorts);
    qDeleteAll(inputs);
    qDeleteAll(outputs);
}

InputPort *ElementPorts::inputPort(const int index) const
{
    if (index < 0 || index >= m_inputPorts.size()) {
        return nullptr;
    }
    return m_inputPorts.at(index);
}

OutputPort *ElementPorts::outputPort(const int index) const
{
    if (index < 0 || index >= m_outputPorts.size()) {
        return nullptr;
    }
    return m_outputPorts.at(index);
}


void ElementPorts::setInputs(const QVector<InputPort *> &inputs)
{
    m_inputPorts = inputs;

    // Keep index() == vector position: Connection::save() derives
    // connection serial IDs from port->index() while GraphicElement::save()
    // numbers ports by position. A caller that permutes the vector (the
    // Display7 legacy pin remap) would otherwise cross-wire the pins on the
    // next save/load round trip. Load-time connection resolution is pointer-
    // keyed, so re-indexing here is safe.
    for (int i = 0; i < m_inputPorts.size(); ++i) {
        if (auto *port = m_inputPorts.at(i)) {
            port->setIndex(i);
        }
    }
}

void ElementPorts::addPort(const QString &name, const bool isOutput)
{
    // No max-size guard here — setInputSize()/setOutputSize() already enforce
    // min/max constraints before calling this method.  The serializer also
    // calls addPort() and needs to create whatever ports the file contains.
    qCDebug(four) << "New port.";
    Port *port = nullptr;

    if (isOutput) {
        m_outputPorts.push_back(new OutputPort());
        port = m_outputPorts.constLast();
        port->setIndex(outputSize() - 1);
    } else {
        m_inputPorts.push_back(new InputPort());
        port = m_inputPorts.constLast();
        port->setIndex(inputSize() - 1);
    }

    port->setGraphicElement(m_owner);
    port->setName(name);
    port->setVisible(true);
}

void ElementPorts::resizeInputs(const int size)
{
    if (size > inputSize()) {
        while (size > inputSize()) { addInputPort(); }
    } else {
        qDeleteAll(m_inputPorts.begin() + size, m_inputPorts.end());
        m_inputPorts.resize(size);
    }
}

void ElementPorts::resizeOutputs(const int size)
{
    if (size > outputSize()) {
        while (size > outputSize()) { addOutputPort(); }
    } else {
        qDeleteAll(m_outputPorts.begin() + size, m_outputPorts.end());
        m_outputPorts.resize(size);
    }
}

InputPort *ElementPorts::takeLastInput()
{
    // Unreachable: GraphicElementSerializer::removeSurplusInputs() is the sole caller, and its
    // own while-loop condition (element.inputSize() > targetSize) can never be true once
    // m_inputPorts (== inputSize()) is already empty, so this guard is never reached from it.
    if (m_inputPorts.isEmpty()) { // LCOV_EXCL_LINE
        return nullptr; // LCOV_EXCL_LINE
    }
    auto *port = m_inputPorts.constLast();
    m_inputPorts.removeLast();
    return port;
}

OutputPort *ElementPorts::takeLastOutput()
{
    // Unreachable for the same reason as takeLastInput() above (removeSurplusOutputs()'s
    // own loop condition already prevents calling this once m_outputPorts is empty).
    if (m_outputPorts.isEmpty()) { // LCOV_EXCL_LINE
        return nullptr; // LCOV_EXCL_LINE
    }
    auto *port = m_outputPorts.constLast();
    m_outputPorts.removeLast();
    return port;
}
