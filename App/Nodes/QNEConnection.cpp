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
#include "App/Nodes/QNEPort.h"

QNEConnection::QNEConnection(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    // Draw wires behind elements so port dots and element bodies always render on top
    setZValue(-1);

    updateTheme();
    // Start in the Invalid visual state; the wire colour is updated once both ports are attached
    setPen(QPen(m_invalidColor, 5));
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
    // Persist the raw pointer addresses as opaque 64-bit IDs.  They are meaningless
    // as pointers after a reload but serve as unique tokens during the same session,
    // and load() uses a portMap to translate them back to the correct QNEPort objects.
    stream << reinterpret_cast<quint64>(m_startPort);
    stream << reinterpret_cast<quint64>(m_endPort);
}

void QNEConnection::load(QDataStream &stream, const QMap<quint64, QNEPort *> &portMap)
{
    quint64 ptr1; stream >> ptr1;
    quint64 ptr2; stream >> ptr2;

    if (portMap.isEmpty()) {
        // No portMap means this is an in-process clipboard paste: the stored integers
        // ARE still valid pointer addresses because no process restart occurred.
        // Casting them back is safe here, but would be undefined behaviour after reload.
        qCDebug(three) << "Empty port map.";
        auto *port1 = reinterpret_cast<QNEPort *>(ptr1);
        auto *port2 = reinterpret_cast<QNEPort *>(ptr2);

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
    }

    if (!portMap.isEmpty()) {
        if (!portMap.contains(ptr1) || !portMap.contains(ptr2)) {
            return;
        }

        qCDebug(three) << "Port map with elements: ptr1(" << ptr1 << "), ptr2(" << ptr2 << ")";
        auto *port1 = portMap.value(ptr1);
        auto *port2 = portMap.value(ptr2);
        qCDebug(three) << "Before if 1.";

        if (port1 && port2) {
            qCDebug(three) << "Before if 2.";

            if (port1->isInput() && port2->isOutput()) {
                qCDebug(three) << "Setting start 1.";
                auto *outputPort = dynamic_cast<QNEOutputPort *>(port2);
                auto *inputPort = dynamic_cast<QNEInputPort *>(port1);
                if (outputPort && inputPort) {
                    setStartPort(outputPort);
                    qCDebug(three) << "Setting end 1.";
                    setEndPort(inputPort);
                }
            } else if (port1->isOutput() && port2->isInput()) {
                qCDebug(three) << "Setting start 2.";
                auto *outputPort = dynamic_cast<QNEOutputPort *>(port1);
                auto *inputPort = dynamic_cast<QNEInputPort *>(port2);
                if (outputPort && inputPort) {
                    setStartPort(outputPort);
                    qCDebug(three) << "Setting end 2.";
                    setEndPort(inputPort);
                }
            }

            qCDebug(three) << "After ifs.";
        }
    }

    qCDebug(three) << "Updating pos from ports.";
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

    // Invalid wires are drawn thicker (5 px) to draw attention to the problem;
    // active/inactive wires are thinner (3 px) to reduce visual clutter during simulation
    switch (status) {
    case Status::Invalid:  setPen(QPen(m_invalidColor,  5)); break;
    case Status::Inactive: setPen(QPen(m_inactiveColor, 3)); break;
    case Status::Active:   setPen(QPen(m_activeColor,   3)); break;

    default:
        // Handle unexpected status values gracefully
        setPen(QPen(m_invalidColor, 5));
        break;
    }

    // Propagate to the destination port so its fill colour also reflects the signal state
    if (endPort()) {
        endPort()->setStatus(status);
    }
}

void QNEConnection::updateTheme()
{
    const ThemeAttributes theme = ThemeManager::attributes();
    m_invalidColor = theme.m_connectionInvalid;
    m_inactiveColor = theme.m_connectionInactive;
    m_activeColor = theme.m_connectionActive;
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
    qCDebug(zero) << "Writing Connection.";
    stream << conn->type();
    conn->save(stream);
    return stream;
}

