// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "node.h"

#include "common.h"
#include "globalproperties.h"
#include "qneport.h"
#include "scene.h"
#include "wirelessconnectionmanager.h"

#include <QDataStream>

Node::Node(QGraphicsItem *parent)
    : GraphicElement(ElementType::Node, ElementGroup::Gate, ":/basic/node.svg", tr("NODE"), tr("Node"), 1, 1, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    m_label->setPos(0, 32);

    setCanChangeSkin(true);
    setHasLabel(true);
    inputPort()->setRequired(true);

    Node::updatePortsProperties();
}

void Node::updatePortsProperties()
{
    inputPort()->setPos(  0, 16);
    outputPort()->setPos(32, 16);
}

void Node::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("wirelessLabel", m_wirelessLabel);

    stream << map;
}

void Node::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    if (version >= VERSION("4.3")) {
        QMap<QString, QVariant> map;
        stream >> map;

        if (map.contains("wirelessLabel")) {
            m_wirelessLabel = map.value("wirelessLabel").toString();
            qCDebug(zero) << "Node" << id() << "loaded wireless label:" << m_wirelessLabel;

            // Note: Wireless connection will be restored later when node is added to scene
            // The scene() is null during deserialization, so we defer restoration
        }
    }
}

void Node::setLabel(const QString &label)
{
    const QString trimmedLabel = label.trimmed();

    // Store the old wireless label for change notification
    const QString oldWirelessLabel = m_wirelessLabel;

    // Call parent implementation to set the visual label
    GraphicElement::setLabel(trimmedLabel);

    // Update wireless label if it changed
    if (m_wirelessLabel != trimmedLabel) {
        m_wirelessLabel = trimmedLabel;

        // Notify the scene's wireless manager
        if (scene()) {
            if (auto *s = qobject_cast<Scene *>(scene())) {
                if (s->wirelessManager()) {
                    s->wirelessManager()->setNodeWirelessLabel(this, trimmedLabel);
                }
            }
        }

        emit wirelessLabelChanged(oldWirelessLabel, trimmedLabel);
        qCDebug(zero) << "Node" << id() << "wireless label changed from" << oldWirelessLabel << "to" << trimmedLabel;
    }
}

QString Node::getWirelessLabel() const
{
    return m_wirelessLabel;
}

bool Node::hasWirelessLabel() const
{
    return !m_wirelessLabel.isEmpty();
}

bool Node::hasInputConnection()
{
    return inputPort() && !inputPort()->connections().isEmpty();
}

bool Node::hasOutputConnection()
{
    return outputPort() && !outputPort()->connections().isEmpty();
}
