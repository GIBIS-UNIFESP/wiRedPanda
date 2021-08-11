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

#ifndef QNEPORT_H
#define QNEPORT_H

#include "qpen.h"

#include <QGraphicsPathItem>

class QNEBlock;
class QNEConnection;
class GraphicElement;

class QNEPort;
typedef QVector<QNEPort *> QNEPortVector;

class QNEPort : public QGraphicsPathItem
{
public:
    enum { Type = QGraphicsItem::UserType + 1 };
    enum { NamePort = 1, TypePort = 2 };

    explicit QNEPort(QGraphicsItem *parent = nullptr);

    void setNEBlock(QNEBlock *);
    void setName(const QString &n);
    int radius() const;
    const QList<QNEConnection *> &connections() const;
    void connect(QNEConnection *conn);
    void disconnect(QNEConnection *conn);
    bool isConnected(QNEPort *);
    void setPortFlags(int);

    virtual bool isOutput() const = 0;

    const QString &portName() const;
    int portFlags() const;

    int type() const override;

    quint64 ptr();
    void setPtr(quint64);

    GraphicElement *graphicElement() const;
    void setGraphicElement(GraphicElement *graphicElement);

    void updateConnections();
    signed char value() const;
    virtual void setValue(signed char value) = 0;

    bool isRequired() const;
    void setRequired(bool required);

    QBrush currentBrush() const;
    void setCurrentBrush(const QBrush &currentBrush);

    int defaultValue() const;
    void setDefaultValue(int defaultValue);

    QString getName() const;

    virtual bool isValid() const = 0;

    void hoverLeave();
    void hoverEnter();

    int index() const;
    void setIndex(int index);

    void setRemoteId(int id) { m_remoteId = id; }
    int getRemoteId() { return m_remoteId; }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    int m_defaultValue;
    int m_index;
    QNEBlock *m_block;
    QString m_name;
    QGraphicsTextItem *m_label;
    int m_radius;
    int m_margin;
    QList<QNEConnection *> m_connections;
    int m_portFlags;
    quint64 m_ptr;

    bool m_required;

    /* WPanda */
    GraphicElement *m_graphicElement;
    QBrush m_currentBrush;
    int m_remoteId;

    /* QGraphicsItem interface */
    signed char m_value;

    virtual void updateTheme() = 0;
};

class QNEInputPort : public QNEPort
{
public:
    explicit QNEInputPort(QGraphicsItem *parent);
    ~QNEInputPort() override;
    /* QNEPort interface */
    void setValue(signed char value) override;
    bool isOutput() const override;
    bool isValid() const override;
    void updateTheme() override;
};

class QNEOutputPort : public QNEPort
{
public:
    explicit QNEOutputPort(QGraphicsItem *parent);
    ~QNEOutputPort() override;
    /* QNEPort interface */
    void setValue(signed char value) override;
    bool isOutput() const override;
    bool isValid() const override;
    void updateTheme() override;
};

#endif /* QNEPORT_H */
