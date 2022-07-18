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
    setBrush(Qt::NoBrush);
    setZValue(-1);
    updateTheme();
}

QNEConnection::~QNEConnection()
{
    if (m_start) {
        m_start->disconnect(this);
    }

    if (m_end) {
        m_end->disconnect(this);
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

void QNEConnection::setStart(QNEOutputPort *port)
{
    QNEOutputPort *old = m_start;
    m_start = port;

    if (old && (old != port)) {
        old->disconnect(this);
    }

    if (port) {
        port->connect(this);
        setStartPos(port->scenePos());
        setStatus(port->status());
    }
}

void QNEConnection::setEnd(QNEInputPort *port)
{
    QNEInputPort *old = m_end;
    m_end = port;

    if (old && (old != port)) {
        old->disconnect(this);
    }

    if (port) {
        port->connect(this);
        setEndPos(port->scenePos());
        port->setStatus(status());
    }
}

void QNEConnection::updatePosFromPorts()
{
    if (m_start) {
        m_startPos = m_start->scenePos();
    }

    if (m_end) {
        m_endPos = m_end->scenePos();
    }

    updatePath();
}

void QNEConnection::updatePath()
{
    QPainterPath path;

    path.moveTo(m_startPos);

    qreal dx = m_endPos.x() - m_startPos.x();
    qreal dy = m_endPos.y() - m_startPos.y();

    QPointF ctr1(m_startPos.x() + dx * 0.25, m_startPos.y() + dy * 0.1);
    QPointF ctr2(m_startPos.x() + dx * 0.75, m_startPos.y() + dy * 0.9);

    path.cubicTo(ctr1, ctr2, m_endPos);

    // path.lineTo(pos2);
    setPath(path);
}

QNEOutputPort *QNEConnection::start() const
{
    return m_start;
}

QNEInputPort *QNEConnection::end() const
{
    return m_end;
}

double QNEConnection::angle()
{
    QNEPort *p1 = m_start;
    QNEPort *p2 = m_end;

    if (p1 && p2) {
        if (p2->isOutput()) {
            std::swap(p1, p2);
        }

        QLineF line(p1->scenePos(), p2->scenePos());
        return line.angle();
    }

    return 0.0;
}

void QNEConnection::save(QDataStream &stream) const
{
    stream << reinterpret_cast<quint64>(m_start);
    stream << reinterpret_cast<quint64>(m_end);
}

void QNEConnection::load(QDataStream &stream, const QMap<quint64, QNEPort *> &portMap)
{
    quint64 ptr1;
    quint64 ptr2;
    stream >> ptr1;
    stream >> ptr2;

    if (portMap.isEmpty()) {
        qCDebug(three) << tr("Empty port map.");
        auto *port1 = reinterpret_cast<QNEPort *>(ptr1);
        auto *port2 = reinterpret_cast<QNEPort *>(ptr2);

        if (port2 && port1) {
            if (!port1->isOutput() && port2->isOutput()) {
                setStart(dynamic_cast<QNEOutputPort *>(port2));
                setEnd(dynamic_cast<QNEInputPort *>(port1));
            } else if (port1->isOutput() && !port2->isOutput()) {
                setStart(dynamic_cast<QNEOutputPort *>(port1));
                setEnd(dynamic_cast<QNEInputPort *>(port2));
            }
        }
    }

    if (!portMap.isEmpty()) {
        if (!portMap.contains(ptr1) || !portMap.contains(ptr2)) {
            return;
        }

        qCDebug(three) << tr("Port map with elements: ptr1:") << ptr1 << tr(", ptr2:") << ptr2;
        QNEPort *port1 = portMap.value(ptr1);
        QNEPort *port2 = portMap.value(ptr2);
        qCDebug(three) << tr("Before if 1.");
        if (port1 && port2) {
            qCDebug(three) << tr("Before if 2.");
            if (port1->isInput() && port2->isOutput()) {
                qCDebug(three) << tr("Setting start 1.");
                setStart(dynamic_cast<QNEOutputPort *>(port2));
                qCDebug(three) << tr("Setting end 1.");
                setEnd(dynamic_cast<QNEInputPort *>(port1));
            } else if (port1->isOutput() && port2->isInput()) {
                qCDebug(three) << tr("Setting start 2.");
                setStart(dynamic_cast<QNEOutputPort *>(port1));
                qCDebug(three) << tr("Setting end 2.");
                setEnd(dynamic_cast<QNEInputPort *>(port2));
            }
            qCDebug(three) << tr("After ifs.");
        }
    }

    qCDebug(three) << tr("Updating pos from ports.");
    updatePosFromPorts();
    qCDebug(three) << tr("Updating path.");
    updatePath();
}

QNEPort *QNEConnection::otherPort(const QNEPort *port) const
{
    if (port == m_start) {
        return m_end;
    }

    return m_start;
}

QNEOutputPort *QNEConnection::otherPort(const QNEInputPort *port) const
{
    Q_UNUSED(port)
    return m_start;
}

QNEInputPort *QNEConnection::otherPort(const QNEOutputPort *port) const
{
    Q_UNUSED(port)
    return m_end;
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
    case Status::Active:   setPen(QPen(m_activeColor,   3)); break;
    case Status::Inactive: setPen(QPen(m_inactiveColor, 3)); break;
    case Status::Invalid:  setPen(QPen(m_invalidColor,  5)); break;
    }
}

void QNEConnection::updateTheme()
{
    const ThemeAttributes theme = ThemeManager::attributes();
    m_inactiveColor = theme.m_qneConnectionFalse;
    m_activeColor = theme.m_qneConnectionTrue;
    m_invalidColor = theme.m_qneConnectionInvalid;
    m_selectedColor = theme.m_qneConnectionSelected;
}

void QNEConnection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)
    painter->setPen(m_highLight ? QPen(Qt::blue, 10) : pen());
    painter->drawPath(path());
    painter->setPen(isSelected() ? QPen(m_selectedColor, 5) : pen());
    painter->drawPath(path());
}

QDataStream &operator<<(QDataStream &stream, const QNEConnection *item)
{
    qCDebug(zero) << QObject::tr("Writing Connection.");
    stream << QNEConnection::Type;
    const auto *conn = qgraphicsitem_cast<const QNEConnection *>(item);
    conn->save(stream);
    return stream;
}

QVariant QNEConnection::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedChange) {
        if (value.toBool()) {
            start()->hoverEnter();
            end()->hoverEnter();
        } else {
            start()->hoverLeave();
            end()->hoverLeave();
        }
    }

    return QGraphicsPathItem::itemChange(change, value);
}

bool QNEConnection::highLight()
{
    return m_highLight;
}

void QNEConnection::setHighLight(bool newHighLight)
{
    m_highLight = newHighLight;
    update();
}
