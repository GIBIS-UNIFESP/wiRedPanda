// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICDefinition.h"

#include <QDataStream>
#include <QFile>
#include <QPainter>

#include "App/IO/Serialization.h"
#include "App/Scene/Scene.h"
#include "App/Versions.h"

ICDefinition ICDefinition::fromFile(const QString &filePath, const QString &contextDir)
{
    Q_UNUSED(contextDir)

    ICDefinition def;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return def;
    }

    QDataStream stream(&file);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    Serialization::loadDolphinFileName(stream, version);
    Serialization::loadRect(stream, version);

    if (version >= Versions::V_4_5) {
        QMap<QString, QVariant> portMeta;
        stream >> portMeta;

        def.m_inputCount = portMeta.value("inputCount").toInt();
        def.m_outputCount = portMeta.value("outputCount").toInt();

        const QStringList inLabels = portMeta.value("inputLabels").toStringList();
        const QStringList outLabels = portMeta.value("outputLabels").toStringList();
        def.m_inputLabels = QVector<QString>(inLabels.begin(), inLabels.end());
        def.m_outputLabels = QVector<QString>(outLabels.begin(), outLabels.end());
    }

    // Read remaining bytes as blob (elements+connections)
    def.m_blobBytes = file.readAll();
    def.m_valid = !def.m_blobBytes.isEmpty();

    return def;
}

const QPixmap &ICDefinition::pixmap() const
{
    if (!m_pixmapValid) {
        generatePixmap();
        m_pixmapValid = true;
    }
    return m_pixmap;
}

void ICDefinition::generatePixmap() const
{
    // Compute size from port count using the same layout as GraphicElement::updatePortsProperties.
    // Ports are spaced 16px apart (2 * gridSize/2), centred around y=32.
    // The box is at least 64x64.
    const int step = Scene::gridSize / 2;
    const int maxPorts = qMax(m_inputCount, m_outputCount);
    const int portsHeight = (maxPorts > 0) ? (maxPorts - 1) * step * 2 + step * 2 : 0;
    const int height = qMax(64, portsHeight);
    const int width = 64;

    QPixmap tempPixmap(width, height);
    tempPixmap.fill(Qt::transparent);

    QPainter painter(&tempPixmap);

    const QColor bodyColor(126, 126, 126);
    const QColor outlineColor(78, 78, 78);

    painter.setBrush(bodyColor);
    painter.setPen(QPen(QBrush(outlineColor), 0.5, Qt::SolidLine));

    // Draw package
    QPoint topLeft(7, 0);
    QSize finalSize(width - 14, height);
    QRectF finalRect(topLeft, finalSize);
    painter.drawRoundedRect(finalRect, 3, 3);

    QPixmap panda(":/Components/Logic/ic-panda2.svg");
    QPointF pandaOrigin = finalRect.center();
    pandaOrigin.setX(pandaOrigin.x() - panda.width() / 2);
    pandaOrigin.setY(pandaOrigin.y() - panda.height() / 2);
    painter.drawPixmap(pandaOrigin, panda);

    // Draw shadow
    painter.setBrush(outlineColor);
    painter.setPen(QPen(QBrush(outlineColor), 0.5, Qt::SolidLine));

    QRectF shadowRect(finalRect.bottomLeft(), finalRect.bottomRight());
    shadowRect.adjust(0, -3, 0, 0);
    painter.drawRoundedRect(shadowRect, 3, 3);

    // Draw semicircle
    QRectF topCenter(finalRect.topLeft() + QPointF(18, -12), QSize(24, 24));
    painter.drawChord(topCenter, 0, -180 * 16);

    m_pixmap = tempPixmap;
}

