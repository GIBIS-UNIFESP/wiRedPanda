/* Copyright (c) 2012, STANISLAW ADASZEWSKI
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * Neither the name of STANISLAW ADASZEWSKI nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL STANISLAW ADASZEWSKI BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "qneconnection.h"

#include "common.h"
#include "qneport.h"
#include "thememanager.h"

#include <QBrush>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

namespace
{
    int id = qRegisterMetaType<QNEConnection>();
}

QNEConnection::QNEConnection(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setZValue(-1);

    updateTheme();
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

    if (oldPort && (oldPort != port)) {
        oldPort->disconnect(this);
    }

    if (port) {
        port->connect(this);
        setStartPos(port->scenePos());
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

void QNEConnection::setWireless(const bool isWireless){
    m_wireless = isWireless;
}

void QNEConnection::updatePath()
{

    if(m_wireless) return;
    QPainterPath path;

    path.moveTo(m_startPos);

    qreal dx = m_endPos.x() - m_startPos.x();
    qreal dy = m_endPos.y() - m_startPos.y();

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
        if (port2->isOutput()) {
            std::swap(port1, port2);
        }

        return QLineF(port1->scenePos(), port2->scenePos()).angle();
    }

    return 0.0;
}

void QNEConnection::save(QDataStream &stream) const
{
    stream << reinterpret_cast<quint64>(m_startPort);
    stream << reinterpret_cast<quint64>(m_endPort);
    stream << m_wireless;
    stream << this->mapId();
}

void QNEConnection::load(QDataStream &stream, const QMap<quint64, QNEPort *> &portMap)
{
    quint64 ptr1; stream >> ptr1;
    quint64 ptr2; stream >> ptr2;
    bool isWireless; stream >> isWireless;
    int mapId; stream >> mapId;
    m_wireless = isWireless;
    m_mapId = mapId;
    if (portMap.isEmpty()) {
        qCDebug(three) << "Empty port map.";
        auto *port1 = reinterpret_cast<QNEPort *>(ptr1);
        auto *port2 = reinterpret_cast<QNEPort *>(ptr2);

        if (port1 && port2) {
            if (port1->isInput() && port2->isOutput()) {
                setStartPort(dynamic_cast<QNEOutputPort *>(port2));
                setEndPort(dynamic_cast<QNEInputPort *>(port1));
            } else if (port1->isOutput() && port2->isInput()) {
                setStartPort(dynamic_cast<QNEOutputPort *>(port1));
                setEndPort(dynamic_cast<QNEInputPort *>(port2));
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
                setStartPort(dynamic_cast<QNEOutputPort *>(port2));
                qCDebug(three) << "Setting end 1.";
                setEndPort(dynamic_cast<QNEInputPort *>(port1));
            } else if (port1->isOutput() && port2->isInput()) {
                qCDebug(three) << "Setting start 2.";
                setStartPort(dynamic_cast<QNEOutputPort *>(port1));
                qCDebug(three) << "Setting end 2.";
                setEndPort(dynamic_cast<QNEInputPort *>(port2));
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

    switch (status) {
    case Status::Invalid:  setPen(QPen(m_invalidColor,  5)); break;
    case Status::Inactive: setPen(QPen(m_inactiveColor, 3)); break;
    case Status::Active:   setPen(QPen(m_activeColor,   3)); break;
    }

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
}

void QNEConnection::setMapId(const int mapId){
    m_mapId = mapId;
}

int QNEConnection::mapId() const{
    if(m_mapId == -1) return this->id();
    return m_mapId;
}

void QNEConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)
    if(m_wireless) return;
    if (m_highLight) {
        painter->save();
        painter->setPen(QPen(Qt::blue, 10));
        painter->drawPath(path());
        painter->restore();
    }

    painter->setPen(isSelected() ? QPen(m_selectedColor, 5) : pen());
    painter->drawPath(path());
}

QVariant QNEConnection::itemChange(GraphicsItemChange change, const QVariant &value)
{
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
    return path().boundingRect().adjusted(-10, -10, 10, 10);
}

bool QNEConnection::sceneEvent(QEvent *event)
{
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
