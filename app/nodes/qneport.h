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

#include "common.h"

#include <QBrush>
#include <QGraphicsPathItem>

class GraphicElement;
class LogicElement;
class QNEConnection;
class QNEPort;

class QNEPort : public QGraphicsPathItem
{
public:
    enum { Type = QGraphicsItem::UserType + 1 };
    int type() const override { return Type; }

    enum { NamePort = 1, TypePort = 2 };

    explicit QNEPort(QGraphicsItem *parent = nullptr);

    GraphicElement *graphicElement() const;
    LogicElement *logicElement() const;
    QBrush currentBrush() const;
    QString name() const;
    Status defaultValue() const;
    Status value() const;
    bool isConnected(QNEPort *);
    bool isRequired() const;
    const QList<QNEConnection *> &connections() const;
    const QString &portName() const;
    int index() const;
    int portFlags() const;
    int radius() const;
    quint64 ptr() const;
    virtual bool isInput() const = 0;
    virtual bool isOutput() const = 0;
    virtual bool isValid() const = 0;
    virtual void setValue(Status value) = 0;
    void connect(QNEConnection *conn);
    void disconnect(QNEConnection *conn);
    void hoverEnter();
    void hoverLeave();
    void setCurrentBrush(const QBrush &currentBrush);
    void setDefaultValue(const Status defaultValue);
    void setGraphicElement(GraphicElement *graphicElement);
    void setIndex(const int index);
    void setName(const QString &name);
    void setPortFlags(const int flags);
    void setPtr(const quint64 pointer);
    void setRequired(const bool required);
    void updateConnections();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    virtual void updateTheme() = 0;

    GraphicElement *m_graphicElement = nullptr;
    QBrush m_currentBrush;
    QGraphicsTextItem *m_label = new QGraphicsTextItem(this);
    QList<QNEConnection *> m_connections;
    QString m_name;
    Status m_defaultValue = Status::Invalid;
    Status m_value = Status::Inactive;
    bool m_required = true;
    int m_index;
    int m_margin = 2;
    int m_portFlags = 0;
    int m_radius = 5;
    quint64 m_ptr;
};

class QNEInputPort : public QNEPort
{
public:
    explicit QNEInputPort(QGraphicsItem *parent);
    ~QNEInputPort() override;

    bool isInput() const override;
    bool isOutput() const override;
    bool isValid() const override;
    void setValue(Status value) override;
    void updateTheme() override;
};

class QNEOutputPort : public QNEPort
{
public:
    explicit QNEOutputPort(QGraphicsItem *parent);
    ~QNEOutputPort() override;

    bool isInput() const override;
    bool isOutput() const override;
    bool isValid() const override;
    void setValue(Status value) override;
    void updateTheme() override;
};
