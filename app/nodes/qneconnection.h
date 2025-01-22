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

#pragma once

#include "enums.h"
#include "itemwithid.h"

#include <QCoreApplication>
#include <QGraphicsPathItem>

class QNEInputPort;
class QNEOutputPort;
class QNEPort;

class QNEConnection : public QGraphicsPathItem, public ItemWithId
{
    Q_DECLARE_TR_FUNCTIONS(QNEConnection)

public:
    enum { Type = QGraphicsItem::UserType + 2 };
    int type() const override { return Type; }

    explicit QNEConnection(QGraphicsItem *parent = nullptr);
    ~QNEConnection() override;
    QNEConnection(const QNEConnection &other) : QNEConnection(other.parentItem()) {}

    QNEInputPort *endPort() const;
    QNEOutputPort *startPort() const;
    QNEPort *otherPort(const QNEPort *port) const;
    QRectF boundingRect() const override;
    Status status() const;
    bool highLight();
    double angle();
    void load(QDataStream &stream, const QMap<quint64, QNEPort *> &portMap = {});
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void save(QDataStream &stream) const;
    void setEndPort(QNEInputPort *port);
    void setEndPos(const QPointF point);
    void setHighLight(const bool highLight);
    void setStartPort(QNEOutputPort *port);
    void setStartPos(const QPointF point);
    void setStatus(const Status status);
    void updatePath();
    void updatePosFromPorts();
    void updateTheme();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    bool sceneEvent(QEvent *event) override;

private:
    QColor m_activeColor;
    QColor m_inactiveColor;
    QColor m_invalidColor;
    QColor m_selectedColor;
    QNEInputPort *m_endPort = nullptr;
    QNEOutputPort *m_startPort = nullptr;
    QPointF m_endPos;
    QPointF m_startPos;
    Status m_status = Status::Invalid;
    bool m_highLight = false;
};

Q_DECLARE_METATYPE(QNEConnection)

QDataStream &operator<<(QDataStream &stream, const QNEConnection *conn);
