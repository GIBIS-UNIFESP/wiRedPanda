// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Draggable element icon+label widget used in the element palette.
 */

#pragma once

#include <QFrame>
#include <QLabel>
#include <QMimeData>

#include "App/Core/Enums.h"

/**
 * \class ElementLabel
 * \brief Icon and name widget shown in the left-panel element palette.
 *
 * \details Displays a small pixmap and the translated element name.
 * When dragged, it produces MIME data that the scene interprets to drop
 * a new element into the circuit.
 */
class ElementLabel : public QFrame
{
    Q_OBJECT

public:
    /**
     * \brief Constructs an ElementLabel from a pixmap pointer.
     * \param pixmap     Icon image (pointer variant).
     * \param type       Element type identifier.
     * \param icFileName IC file path (empty for built-in elements).
     * \param parent     Optional parent widget.
     */
    explicit ElementLabel(const QPixmap *pixmap, ElementType type, const QString &icFileName, QWidget *parent = nullptr);

    /**
     * \brief Constructs an ElementLabel from a pixmap.
     * \param pixmap     Icon image.
     * \param type       Element type identifier.
     * \param icFileName IC file path (empty for built-in elements).
     * \param parent     Optional parent widget.
     */
    explicit ElementLabel(const QPixmap &pixmap, ElementType type, const QString &icFileName, QWidget *parent = nullptr);

    // --- Queries ---

    /// Returns the element type identifier for this label.
    const ElementType &elementType() const;

    /// Returns the IC file path, or an empty string for built-in elements.
    QString icFileName() const;

    /// Returns the displayed element name.
    QString name() const;

    /// Returns the icon pixmap shown in this label.
    const QPixmap &pixmap() const;

    // --- Drag support ---

    /// Creates and returns the MIME data payload used for drag-and-drop.
    QMimeData *mimeData();

    /// Initiates a drag operation carrying this label's element type.
    void startDrag();

    // --- Display updates ---

    /// Updates the displayed name to match the current application locale.
    void updateName();

    /// Updates colors to match the current application theme.
    void updateTheme();

protected:
    /// Starts a drag operation when the user clicks on this label.
    void mousePressEvent(QMouseEvent *event) override;

private:
    // --- Members ---

    ElementType m_elementType = ElementType::Unknown;
    QLabel m_iconLabel;
    QLabel m_nameLabel;
    QPixmap m_pixmap;
    QString m_icFileName;
};

