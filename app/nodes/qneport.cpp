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

#include "qneport.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "thememanager.h"

#include <QCursor>
#include <QDebug>
#include <QFontMetrics>
#include <QGraphicsScene>
#include <QPen>
#include <iostream>

QNEPort::QNEPort(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    QPainterPath p;
    p.addPolygon(QRectF(QPointF(-m_radius, -m_radius), QPointF(m_radius, m_radius)));
    setPath(p);

    setPen(QPen(Qt::darkRed));
    setCurrentBrush(Qt::red);

    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
}

void QNEPort::setName(const QString &n)
{
    m_name = n;
    // m_label->setPlainText(n);
    setToolTip(n);
}

int QNEPort::radius() const
{
    return m_radius;
}

const QList<QNEConnection *> &QNEPort::connections() const
{
    return m_connections;
}

void QNEPort::connect(QNEConnection *conn)
{
    if (conn) {
        if (!m_connections.contains(conn)) {
            m_connections.append(conn);
        }
        updateConnections();
        // if (graphicElement()) {
        //     graphicElement()->updatePorts();
        //     if (isOutput()) {
        //         graphicElement()->updateLogic();
        //     }
        // }
    }
}

void QNEPort::disconnect(QNEConnection *conn)
{
    m_connections.removeAll(conn);
    if (conn->start() == this) {
        conn->setStart(nullptr);
    }
    if (conn->end() == this) {
        conn->setEnd(nullptr);
    }

    updateConnections();
}

void QNEPort::setPortFlags(const int f)
{
    m_portFlags = f;
    if (m_portFlags & TypePort) {
        QFont font(scene()->font());
        font.setItalic(true);
        m_label->setFont(font);
        setPath(QPainterPath());
    } else if (m_portFlags & NamePort) {
        QFont font(scene()->font());
        font.setBold(true);
        m_label->setFont(font);
        setPath(QPainterPath());
    }
}

const QString &QNEPort::portName() const
{
    return m_name;
}

int QNEPort::portFlags() const
{
    return m_portFlags;
}

quint64 QNEPort::ptr() const
{
    return m_ptr;
}

void QNEPort::setPtr(const quint64 p)
{
    m_ptr = p;
}

bool QNEPort::isConnected(QNEPort *other)
{
    return std::any_of(m_connections.cbegin(), m_connections.cend(), [&](auto *conn) { return conn->start() == other || conn->end() == other; });
}

void QNEPort::updateConnections()
{
    for (auto *conn : qAsConst(m_connections)) {
        conn->updatePosFromPorts();
        conn->updatePath();
    }
    if (isValid()) {
        if (m_connections.empty() && !isOutput()) {
            setValue(static_cast<signed char>(defaultValue()));
        }
    } else {
        setValue(-1);
    }
}

QVariant QNEPort::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemScenePositionHasChanged) {
        updateConnections();
    }
    return value;
}

int QNEPort::index() const
{
    return m_index;
}

void QNEPort::setIndex(const int index)
{
    m_index = index;
}

QString QNEPort::name() const
{
    return m_name;
}

int QNEPort::defaultValue() const
{
    return m_defaultValue;
}

void QNEPort::setDefaultValue(const int defaultValue)
{
    m_defaultValue = defaultValue;
    setValue(static_cast<signed char>(defaultValue));
}

QBrush QNEPort::currentBrush() const
{
    return m_currentBrush;
}

void QNEPort::setCurrentBrush(const QBrush &currentBrush)
{
    m_currentBrush = currentBrush;
    if (brush().color() != Qt::yellow) {
        setBrush(currentBrush);
    }
}

bool QNEPort::isRequired() const
{
    return m_required;
}

void QNEPort::setRequired(const bool required)
{
    m_required = required;
    setDefaultValue(-1 * required);
}

signed char QNEPort::value() const
{
    return m_value;
}

GraphicElement *QNEPort::graphicElement() const
{
    return m_graphicElement;
}

void QNEPort::setGraphicElement(GraphicElement *graphicElement)
{
    m_graphicElement = graphicElement;
}

void QNEPort::hoverLeave()
{
    setBrush(currentBrush());
    update();
}

void QNEPort::hoverEnter()
{
    setBrush(QBrush(Qt::yellow));
    update();
}

QNEInputPort::QNEInputPort(QGraphicsItem *parent)
    : QNEPort(parent)
{
    m_label->setPos(-m_radius - m_margin - m_label->boundingRect().width(), -m_label->boundingRect().height() / 2);
}

QNEInputPort::~QNEInputPort()
{
    while (!m_connections.isEmpty()) {
        QNEConnection *conn = m_connections.last();
        m_connections.removeAll(conn);
        conn->setEnd(nullptr);
        delete conn;
    }
}

void QNEInputPort::setValue(signed char value)
{
    m_value = value;
    if (!isValid()) {
        m_value = -1;
    }
    const auto theme = ThemeManager::attributes();
    if (m_value == -1) {
        setPen(theme.m_qnePortInvalidPen);
        setCurrentBrush(theme.m_qnePortInvalidBrush);
    } else if (m_value == 1) {
        setPen(theme.m_qnePortTruePen);
        setCurrentBrush(theme.m_qnePortTrueBrush);
    } else {
        setPen(theme.m_qnePortFalsePen);
        setCurrentBrush(theme.m_qnePortFalseBrush);
    }
}

bool QNEInputPort::isOutput() const
{
    return false;
}

bool QNEInputPort::isValid() const
{
    if (m_connections.isEmpty()) {
        return !isRequired();
    }
    return m_connections.size() == 1;
}

void QNEInputPort::updateTheme()
{
}

QNEOutputPort::QNEOutputPort(QGraphicsItem *parent)
    : QNEPort(parent)
{
    m_label->setPos(m_radius + m_margin, -m_label->boundingRect().height() / 2);
    QNEOutputPort::updateTheme();
}

QNEOutputPort::~QNEOutputPort()
{
    while (!m_connections.isEmpty()) {
        auto *conn = m_connections.last();
        m_connections.removeAll(conn);
        conn->setStart(nullptr);
        delete conn;
    }
}

void QNEOutputPort::setValue(signed char value)
{
    m_value = value;
    for (auto *conn : connections()) {
        if (value == -1) {
            conn->setStatus(QNEConnection::Status::Invalid);
        } else if (value == 0) {
            conn->setStatus(QNEConnection::Status::Inactive);
        } else {
            conn->setStatus(QNEConnection::Status::Active);
        }
        auto *port = conn->otherPort(this);
        if (port) {
            port->setValue(value);
        }
    }
}

bool QNEOutputPort::isOutput() const
{
    return true;
}

bool QNEOutputPort::isValid() const
{
    return m_value != -1;
}

void QNEOutputPort::updateTheme()
{
    const auto theme = ThemeManager::attributes();
    setPen(theme.m_qnePortOutputPen);
    setCurrentBrush(theme.m_qnePortOutputBrush);
}
