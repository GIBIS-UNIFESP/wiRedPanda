// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief TrashButton: a push-button that accepts drag-and-drop of IC files for deletion.
 */

#pragma once

#include <QPushButton>

/**
 * \class TrashButton
 * \brief Push-button that accepts IC file drag-and-drop to trigger IC removal.
 *
 * \details When an IC file is dragged from the IC list and dropped onto this button,
 * it emits removeICFile() with the IC's file path so the caller can delete it.
 */
class TrashButton : public QPushButton
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the trash button with optional \a parent.
    explicit TrashButton(QWidget *parent = nullptr);

signals:
    // --- Signals ---

    /// Removes the IC file \a icFileName from disk after user confirmation.
    void removeICFile(const QString &icFileName);

protected:
    // --- Drag and Drop ---

    /// \reimp
    void dragEnterEvent(QDragEnterEvent *event) override;
    /// \reimp
    void dropEvent(QDropEvent *event) override;
};
