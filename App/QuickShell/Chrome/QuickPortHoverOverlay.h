// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickPortHoverOverlay: QML-facing presenter driving the port-hover highlight/label chips.
 */

#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QVariant>

class CanvasItem;

/**
 * \class QuickPortHoverOverlay
 * \brief Thin QML-property wrapper around CanvasItem::portHoverChanged(), driving
 * PortHoverOverlay.qml's Repeater the way QuickICPreview drives ICPreviewPopup.qml.
 *
 * \details Ports the Widgets-side ConnectionManager::setHoverPort()/showHoverLabels()/
 * releaseHoverPort() feature (hovering a port highlights it and every port connected to it via
 * a wire, with in-situ name-chip labels for the named ones) -- see
 * project_quick_real_missing_features_found_in_phase7 project memory for why it was deferred
 * out of the original Phase 7 test-writing pass. Unlike QuickICPreview, there's no
 * show/hide-delay state machine to port: CanvasItem::portHoverChanged() already carries the
 * final chip list (or an empty one to hide), computed fresh on every hover-port change, so this
 * class only needs to store and re-expose it.
 */
class QuickPortHoverOverlay : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("QuickPortHoverOverlay is only ever exposed via AppController.portHoverOverlay")

    // FINAL: matches every other presenter in this Quick chrome (see
    // project_qml_aot_compilation_fusion_style_pin memory).
    Q_PROPERTY(QVariantList chips READ chips NOTIFY refreshed FINAL)

public:
    explicit QuickPortHoverOverlay(QObject *parent = nullptr);

    /// Binds this presenter to \a canvas's portHoverChanged() signal; passing nullptr unbinds
    /// and clears any chips currently shown. Mirrors QuickICPreview::setCanvas()'s shape.
    void setCanvas(CanvasItem *canvas);

    /// Each entry is a QVariantMap with screenX/screenY/radius/side/text -- see
    /// CanvasItem::portHoverChanged()'s own doc comment for the exact fields.
    [[nodiscard]] QVariantList chips() const { return m_chips; }

signals:
    void refreshed();

private:
    void onPortHoverChanged(const QVariantList &chips);

    // QPointer, not a raw CanvasItem*: see QuickElementEditor::m_canvas's identical doc comment
    // -- the previously-bound canvas may already be destroyed by the time setCanvas() runs again.
    QPointer<CanvasItem> m_canvas;
    QVariantList m_chips;
};
