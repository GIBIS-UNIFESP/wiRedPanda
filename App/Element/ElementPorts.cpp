// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementPorts.h"

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEPort.h"

ElementPorts::ElementPorts(GraphicElement *owner)
    : m_owner(owner)
{
}

QNEInputPort *ElementPorts::inputPort(const int index) const
{
    if (index < 0 || index >= m_inputPorts.size()) {
        return nullptr;
    }
    return m_inputPorts.at(index);
}

QNEOutputPort *ElementPorts::outputPort(const int index) const
{
    if (index < 0 || index >= m_outputPorts.size()) {
        return nullptr;
    }
    return m_outputPorts.at(index);
}

int ElementPorts::inputSize() const
{
    return static_cast<int>(m_inputPorts.size());
}

int ElementPorts::outputSize() const
{
    return static_cast<int>(m_outputPorts.size());
}

QVector<QNEPort *> ElementPorts::allPorts() const
{
    QVector<QNEPort *> result;
    result.reserve(m_inputPorts.size() + m_outputPorts.size());
    for (auto *p : m_inputPorts)  { result.append(p); }
    for (auto *p : m_outputPorts) { result.append(p); }
    return result;
}

void ElementPorts::addPort(const QString &name, const bool isOutput)
{
    // No max-size guard here — setInputSize()/setOutputSize() already enforce
    // min/max constraints before calling this method.  The serializer also
    // calls addPort() and needs to create whatever ports the file contains.
    qCDebug(four) << "New port.";
    QNEPort *port = nullptr;

    if (isOutput) {
        m_outputPorts.push_back(new QNEOutputPort(m_owner));
        port = m_outputPorts.constLast();
        port->setIndex(outputSize() - 1);
    } else {
        m_inputPorts.push_back(new QNEInputPort(m_owner));
        port = m_inputPorts.constLast();
        port->setIndex(inputSize() - 1);
    }

    port->setGraphicElement(m_owner);
    port->setName(name);
    port->show();
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

QNEInputPort *ElementPorts::takeLastInput()
{
    if (m_inputPorts.isEmpty()) {
        return nullptr;
    }
    auto *port = m_inputPorts.constLast();
    m_inputPorts.removeLast();
    return port;
}

QNEOutputPort *ElementPorts::takeLastOutput()
{
    if (m_outputPorts.isEmpty()) {
        return nullptr;
    }
    auto *port = m_outputPorts.constLast();
    m_outputPorts.removeLast();
    return port;
}
