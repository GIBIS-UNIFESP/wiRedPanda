// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICDefinition.h"

#include <QDataStream>
#include <QFile>

#include "App/IO/Serialization.h"

ICDefinition ICDefinition::fromFile(const QString &filePath, const QString &contextDir)
{
    Q_UNUSED(contextDir)
    ICDefinition def;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return def;
    }

    // Read entire file into memory so QDataStream and byte slicing work on the same buffer
    QByteArray fileData = file.readAll();
    file.close();

    QDataStream stream(&fileData, QIODevice::ReadOnly);
    const auto preamble = Serialization::readPreamble(stream);

    def.m_inputCount = preamble.metadata.value("inputCount").toInt();
    def.m_outputCount = preamble.metadata.value("outputCount").toInt();

    const QStringList inLabels = preamble.metadata.value("inputLabels").toStringList();
    const QStringList outLabels = preamble.metadata.value("outputLabels").toStringList();
    def.m_inputLabels = QVector<QString>(inLabels.begin(), inLabels.end());
    def.m_outputLabels = QVector<QString>(outLabels.begin(), outLabels.end());

    // Store the full .panda file as the blob
    def.m_blobBytes = fileData;
    def.m_valid = !def.m_blobBytes.isEmpty();

    return def;
}

