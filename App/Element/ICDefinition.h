// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPixmap>
#include <QString>
#include <QVector>

/**
 * \class ICDefinition
 * \brief Shared, immutable template for an IC sub-circuit type.
 *
 * \details Holds port metadata (count, labels) and the raw serialized bytes
 * (elements+connections) of the sub-circuit. Multiple IC instances referencing
 * the same file share one ICDefinition. The metadata is read from the .panda
 * file's port metadata section (V_4_5+) without deserializing elements.
 *
 * Each IC instance still deserializes its own copy of the sub-circuit elements
 * for simulation, since simulation state is mutable and per-instance.
 */
class ICDefinition
{
public:
    ICDefinition() = default;

    /// Construct from a .panda file. Reads metadata from the port metadata
    /// section (V_4_5+) without deserializing elements.
    static ICDefinition fromFile(const QString &filePath, const QString &contextDir);

    /// The full .panda file bytes (header + metadata + elements + connections).
    const QByteArray &blobBytes() const { return m_blobBytes; }

    /// Port counts.
    int inputCount() const { return m_inputCount; }
    int outputCount() const { return m_outputCount; }

    /// Port display labels (sorted by position, matching IC port order).
    const QVector<QString> &inputLabels() const { return m_inputLabels; }
    const QVector<QString> &outputLabels() const { return m_outputLabels; }

    /// Cached pixmap generated from port count. Lazy, thread-safe via mutable.
    const QPixmap &pixmap() const;

    /// Returns true if this definition was successfully loaded.
    bool isValid() const { return m_valid; }

private:
    void generatePixmap() const;

    QByteArray m_blobBytes;
    int m_inputCount = 0;
    int m_outputCount = 0;
    QVector<QString> m_inputLabels;
    QVector<QString> m_outputLabels;
    bool m_valid = false;

    mutable QPixmap m_pixmap;
    mutable bool m_pixmapValid = false;
};

