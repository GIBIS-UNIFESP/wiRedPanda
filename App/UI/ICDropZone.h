// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICDropZone: drop target widget for cross-section IC palette drops.
 */

#pragma once

#include <QWidget>

/**
 * \class ICDropZone
 * \brief Drop zone widget that accepts opposite-type IC label drops.
 *
 * \details Used in the IC palette to enable cross-section drag-and-drop:
 * dropping a file-based IC onto the embedded section triggers embedding,
 * and dropping an embedded IC onto the file-based section triggers extraction.
 */
class ICDropZone : public QWidget
{
    Q_OBJECT

public:
    /// Identifies which IC palette section this zone belongs to.
    enum class Section {
        FileBased, ///< The file-backed IC section (accepts embedded drops for extraction).
        Embedded,  ///< The embedded IC section (accepts file-backed drops for embedding).
    };

    /// Constructs a drop zone for the given palette \a section.
    explicit ICDropZone(Section section, QWidget *parent = nullptr);

signals:
    /// Emitted when a file-based IC is dropped onto the embedded section.
    void embedByFileRequested(const QString &fileName);

    /// Emitted when an embedded IC is dropped onto the file-based section.
    void extractByBlobNameRequested(const QString &blobName);

protected:
    /// \reimp
    void dragEnterEvent(QDragEnterEvent *event) override;
    /// \reimp
    void dragMoveEvent(QDragMoveEvent *event) override;
    /// \reimp
    void dropEvent(QDropEvent *event) override;

private:
    Section m_section; ///< Which palette section this zone represents.
};

