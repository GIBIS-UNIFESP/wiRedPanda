// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief ICDropZone: drop target widget for cross-section IC palette drops.

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
    enum class Section { FileBased, Embedded };

    explicit ICDropZone(Section section, QWidget *parent = nullptr);

signals:
    /// Emitted when a file-based IC is dropped onto the embedded section.
    void embedByFileRequested(const QString &fileName);

    /// Emitted when an embedded IC is dropped onto the file-based section.
    void extractByBlobNameRequested(const QString &blobName);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Section m_section;
};

