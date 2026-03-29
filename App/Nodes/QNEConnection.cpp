// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Nodes/QNEConnection.h"

#include <QBrush>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

#include "App/Core/Common.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Nodes/QNEPort.h"

static const int s_connectionMetatypeId = qRegisterMetaType<QNEConnection>();

QNEConnection::QNEConnection(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    // Draw wires behind elements so port dots and element bodies always render on top
    setZValue(-1);

    updateTheme();
    // Start in the Unknown visual state; the wire colour is updated once both ports are attached
    setPen(QPen(m_unknownColor, 3));
}

QNEConnection::~QNEConnection()
{
    if (m_startPort) {
        m_startPort->disconnect(this);
    }

    if (m_endPort) {
        m_endPort->disconnect(this);
    }
}

void QNEConnection::setStartPos(const QPointF point)
{
    m_startPos = point;
}

void QNEConnection::setEndPos(const QPointF point)
{
    m_endPos = point;
}

void QNEConnection::setStartPort(QNEOutputPort *port)
{
    auto *oldPort = m_startPort;
    m_startPort = port;

    // Detach from the previous port before attaching to the new one to avoid
    // dangling entries in the old port's connection list (e.g. during IC hot-reload)
    if (oldPort && (oldPort != port)) {
        oldPort->disconnect(this);
    }

    if (port) {
        port->connect(this);
        setStartPos(port->scenePos());
        // Inherit the source port's signal status so the wire colour is correct immediately
        setStatus(port->status());
    }
}

void QNEConnection::setEndPort(QNEInputPort *port)
{
    auto *oldPort = m_endPort;
    m_endPort = port;

    if (oldPort && (oldPort != port)) {
        oldPort->disconnect(this);
    }

    if (port) {
        port->connect(this);
        setEndPos(port->scenePos());
        // Push the current wire status into the destination port so it displays correctly
        // even before the next simulation step
        port->setStatus(status());
    }
}

void QNEConnection::updatePosFromPorts()
{
    if (m_startPort) {
        m_startPos = m_startPort->scenePos();
    }

    if (m_endPort) {
        m_endPos = m_endPort->scenePos();
    }

    updatePath();
}

void QNEConnection::updatePath()
{
    QPainterPath path;

    path.moveTo(m_startPos);

    qreal dx = m_endPos.x() - m_startPos.x();
    qreal dy = m_endPos.y() - m_startPos.y();

    // Cubic Bézier control points chosen so the wire leaves/arrives roughly horizontally
    // (small Y fraction) and curves gently in the middle (0.25/0.75 X fraction).
    // This gives an S-curve appearance typical of schematic wire routing tools.
    QPointF ctr1(m_startPos.x() + dx * 0.25, m_startPos.y() + dy * 0.1);
    QPointF ctr2(m_startPos.x() + dx * 0.75, m_startPos.y() + dy * 0.9);

    path.cubicTo(ctr1, ctr2, m_endPos);

    setPath(path);
}

QNEOutputPort *QNEConnection::startPort() const
{
    return m_startPort;
}

QNEInputPort *QNEConnection::endPort() const
{
    return m_endPort;
}

double QNEConnection::angle()
{
    QNEPort *port1 = m_startPort;
    QNEPort *port2 = m_endPort;

    if (port1 && port2) {
        // Normalise: port1 = output, port2 = input regardless of which end was set first.
        // QLineF::angle() measures from positive X axis counter-clockwise, so the
        // returned angle always describes the direction from output to input.
        if (port2->isOutput()) {
            std::swap(port1, port2);
        }

        return QLineF(port1->scenePos(), port2->scenePos()).angle();
    }

    return 0.0;
}

void QNEConnection::save(QDataStream &stream) const
{
    // Calculate and save port serial IDs deterministically
    // Serial ID format: (elementId << 16) | portIndex
    // For output ports: portIndex = inputSize + outputIndex

    auto calculateSerialId = [](QNEPort *port) -> quint64 {
        if (!port) return 0;

        GraphicElement *elem = port->graphicElement();
        if (!elem) return 0;

        quint64 elementId = static_cast<quint64>(elem->id());
        int portIndex = port->index();

        // For output ports, offset by the number of input ports
        if (port->isOutput()) {
            portIndex += elem->inputSize();
        }

        return (elementId << 16) | (portIndex & 0xFFFF);
    };

    QMap<QString, QVariant> map;
    map.insert("startPortId", calculateSerialId(m_startPort));
    map.insert("endPortId", calculateSerialId(m_endPort));
    stream << map;
}

void QNEConnection::load(QDataStream &stream, SerializationContext &context)
{
    quint64 id1, id2;

    if (VersionInfo::hasConnectionQMap(context.version)) {
        QMap<QString, QVariant> map;
        stream >> map;

        if (stream.status() != QDataStream::Ok) {
            throw PANDACEPTION("Stream error reading connection map at offset %1",
                              stream.device()->pos());
        }

        id1 = map.value("startPortId").toULongLong();
        id2 = map.value("endPortId").toULongLong();
    } else {
        stream >> id1;
        stream >> id2;

        if (stream.status() != QDataStream::Ok) {
            throw PANDACEPTION("Stream error reading connection port IDs at offset %1",
                              stream.device()->pos());
        }

        // For backwards compatibility with old files: if ID not found and oldPtrMap is provided, try the mapping
        if (!context.portMap.contains(id1) && !context.oldPtrToSerialId.isEmpty()) {
            quint64 newId = context.oldPtrToSerialId.value(id1, 0);
            if (newId != 0) {
                id1 = newId;
            }
        }

        if (!context.portMap.contains(id2) && !context.oldPtrToSerialId.isEmpty()) {
            quint64 newId = context.oldPtrToSerialId.value(id2, 0);
            if (newId != 0) {
                id2 = newId;
            }
        }
    }

    if (!context.portMap.contains(id1) || !context.portMap.contains(id2)) {
        return;
    }

    auto *port1 = context.portMap.value(id1);
    auto *port2 = context.portMap.value(id2);

    if (port1 && port2) {
        if (port1->isInput() && port2->isOutput()) {
            auto *outputPort = dynamic_cast<QNEOutputPort *>(port2);
            auto *inputPort = dynamic_cast<QNEInputPort *>(port1);
            if (outputPort && inputPort) {
                setStartPort(outputPort);
                setEndPort(inputPort);
            }
        } else if (port1->isOutput() && port2->isInput()) {
            auto *outputPort = dynamic_cast<QNEOutputPort *>(port1);
            auto *inputPort = dynamic_cast<QNEInputPort *>(port2);
            if (outputPort && inputPort) {
                setStartPort(outputPort);
                setEndPort(inputPort);
            }
        }
    }

    updatePosFromPorts();
}

QNEPort *QNEConnection::otherPort(const QNEPort *port) const
{
    if (port == m_startPort) {
        return m_endPort;
    }

    return m_startPort;
}

Status QNEConnection::status() const
{
    return m_status;
}

void QNEConnection::setStatus(const Status status)
{
    if (status == m_status) {
        return;
    }

    m_status = status;

    // Error wires are drawn thicker (5 px) to draw attention to the problem;
    // other wires are thinner (3 px) to reduce visual clutter during simulation.
    // Unknown (undriven) wires use a distinct gray from Error (red).
    switch (status) {
    case Status::Unknown:  setPen(QPen(m_unknownColor,  3)); break;
    case Status::Inactive: setPen(QPen(m_inactiveColor, 3)); break;
    case Status::Active:   setPen(QPen(m_activeColor,   3)); break;
    case Status::Error:    setPen(QPen(m_errorColor,    5)); break;
    }

    // Propagate to the destination port so its fill colour also reflects the signal state
    if (endPort()) {
        endPort()->setStatus(status);
    }
}

void QNEConnection::updateTheme()
{
    const ThemeAttributes theme = ThemeManager::attributes();
    m_unknownColor = theme.m_connectionUnknown;
    m_inactiveColor = theme.m_connectionInactive;
    m_activeColor = theme.m_connectionActive;
    m_errorColor = theme.m_connectionError;
    m_selectedColor = theme.m_connectionSelected;
    update();
}

void QNEConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    // Highlight is drawn as a wider blue halo beneath the normal wire, so users can
    // easily see which wires belong to a selected element
    if (m_highLight) {
        painter->save();
        painter->setPen(QPen(Qt::blue, 10));
        painter->drawPath(path());
        painter->restore();
    }

    // When the wire itself is selected (clicked), switch to the selection colour;
    // otherwise use the status-driven pen set by setStatus()
    painter->setPen(isSelected() ? QPen(m_selectedColor, 5) : pen());
    painter->drawPath(path());
}

QVariant QNEConnection::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // When the wire is selected/deselected, visually highlight both endpoint ports
    // so the user can see which element pins are connected by this wire
    if (change == ItemSelectedChange) {
        if (value.toBool()) {
            if (startPort()) startPort()->hoverEnter();
            if (endPort()) endPort()->hoverEnter();
        } else {
            if (startPort()) startPort()->hoverLeave();
            if (endPort()) endPort()->hoverLeave();
        }
    }

    return QGraphicsPathItem::itemChange(change, value);
}

bool QNEConnection::highLight()
{
    return m_highLight;
}

void QNEConnection::setHighLight(const bool highLight)
{
    m_highLight = highLight;
    update();
}

QRectF QNEConnection::boundingRect() const
{
    // Expand beyond the path's tight bounding box by 10 px on all sides to ensure
    // the thick selection outline and highlight halo are fully covered during repaints
    return path().boundingRect().adjusted(-10, -10, 10, 10);
}

bool QNEConnection::sceneEvent(QEvent *event)
{
    // Swallow Ctrl+click so the scene can use Ctrl+click for multi-selection without
    // the wire consuming the event and blocking the rubber-band/deselect behaviour
    if (auto mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent *>(event)) {
        if (mouseEvent->modifiers().testFlag(Qt::ControlModifier)) {
            return true;
        }
    }

    return QGraphicsPathItem::sceneEvent(event);
}

QDataStream &operator<<(QDataStream &stream, const QNEConnection *conn)
{
    // Type tags are now written by Serialization::serialize() for symmetry
    // This is now only called from serialize(), so type is already written
    qCDebug(zero) << "Writing Connection.";
    conn->save(stream);
    return stream;
}

