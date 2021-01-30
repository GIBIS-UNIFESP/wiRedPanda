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

#ifndef QNECONNECTION_H
#define QNECONNECTION_H

#include "itemwithid.h"

#include <QGraphicsPathItem>

class QNEPort;
class QNEInputPort;
class QNEOutputPort;

class QNEConnection : public QGraphicsPathItem, public ItemWithId
{
public:
    enum : uint32_t { Type = QGraphicsItem::UserType + 2 };
    enum class Status { Invalid = -1, Inactive = 0, Active = 1 };

    explicit QNEConnection(QGraphicsItem *parent = nullptr);
    virtual ~QNEConnection();

    void setStartPos(const QPointF &p);
    void setEndPos(const QPointF &p);
    void setStart(QNEOutputPort *p);
    void setEnd(QNEInputPort *p);
    void updatePosFromPorts();
    void updatePath();
    QNEOutputPort *start() const;
    QNEInputPort *end() const;

    double angle();

    void save(QDataStream &) const;
    bool load(QDataStream &, const QMap<quint64, QNEPort *> &portMap = QMap<quint64, QNEPort *>());

    int type() const
    {
        return (Type);
    }
    QNEPort *otherPort(const QNEPort *port) const;
    QNEOutputPort *otherPort(const QNEInputPort *) const;
    QNEInputPort *otherPort(const QNEOutputPort *) const;
    Status status() const;
    void setStatus(const Status &status);

    void updateTheme();

private:
    QPointF startPos;
    QPointF endPos;
    QNEOutputPort *m_start;
    QNEInputPort *m_end;
    Status m_status;

    QColor m_invalidClr;
    QColor m_activeClr;
    QColor m_inactiveClr;
    QColor m_selectedClr;
    /* QGraphicsItem interface */
public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif /* QNECONNECTION_H */
