// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Shared, immutable IC sub-circuit template with port metadata and cached pixmap.
 */

#pragma once

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
    /// Constructs an empty (invalid) definition.
    ICDefinition() = default;

    /**
     * \brief Constructs a definition by reading metadata from a .panda file.
     * \param filePath Path to the .panda file.
     * \return A valid ICDefinition on success, or an invalid one on failure.
     */
    static ICDefinition fromFile(const QString &filePath);

    /// Returns the full .panda file bytes (header + metadata + elements + connections).
    const QByteArray &blobBytes() const { return m_blobBytes; }

    /// Returns the number of input ports.
    int inputCount() const { return m_inputCount; }
    /// Returns the number of output ports.
    int outputCount() const { return m_outputCount; }

    /// Returns input port display labels (sorted by position, matching IC port order).
    const QVector<QString> &inputLabels() const { return m_inputLabels; }
    /// Returns output port display labels (sorted by position, matching IC port order).
    const QVector<QString> &outputLabels() const { return m_outputLabels; }

    /// Returns true if this definition was successfully loaded.
    bool isValid() const { return m_valid; }

private:
    QByteArray m_blobBytes;            ///< Raw serialized .panda file content.
    int m_inputCount = 0;              ///< Number of input ports in the sub-circuit.
    int m_outputCount = 0;             ///< Number of output ports in the sub-circuit.
    QVector<QString> m_inputLabels;    ///< Input port display labels.
    QVector<QString> m_outputLabels;   ///< Output port display labels.
    bool m_valid = false;              ///< True if the definition was successfully loaded.
};

