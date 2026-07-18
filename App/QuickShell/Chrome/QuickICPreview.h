// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickICPreview: QML-facing presenter driving the IC hover-preview popup.
 */

#pragma once

#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QQmlEngine>
#include <QString>
#include <QTimer>

class CanvasItem;
class GraphicElement;

/**
 * \class QuickICPreview
 * \brief Copy-and-adapted, Widgets-free port of App/Element/ICPreviewPopup's show/hide-delay
 * state machine for the Quick chrome.
 *
 * \details Drives ICPreviewPopup.qml's visible/title/imageUrl/screenPos bindings the way
 * ICPreviewPopup itself drives a QWidget's show()/move()/hide() calls. setCanvas() connects to
 * CanvasItem's icPreviewRequested()/icPreviewMoved()/icPreviewHideRequested()/
 * icPreviewCancelRequested() signals -- CanvasItem emits these directly from
 * hoverMoveEvent()/hoverLeaveEvent()/mouseDoubleClickEvent(), the same call sites that already
 * (redundantly, but harmlessly) call IC's own real previewRequested()/previewMoved()/
 * previewHideRequested() signals, which nothing in production's SceneUiBinder-based UI needs
 * this class to touch.
 *
 * \details The show/hide-delay/already-pending-for-this-IC state machine
 * (showForIC()/executeShow()/scheduleHide()/cancelHide()/updatePendingPos()/isShowActiveFor())
 * is a direct, line-for-line port of ICPreviewPopup's own logic, not a simplified
 * reinvention -- see each private method's doc comment for the production method it mirrors.
 * The one real difference: instead of rendering IC::previewPixmap() (always empty here --
 * ICRenderer::generatePreviewPixmap() guards on a real QApplication existing, which
 * wiredpanda_quick never constructs), executeShow() calls CanvasItem::renderICPreviewImage()
 * and encodes the result as a data: URL for imageUrl -- see that method's own doc comment for
 * why (internalElements() instead of the pre-substitution designed circuit) and why a data:
 * URL (simplest way to hand a dynamically-generated image to a QML Image.source without a
 * QQuickImageProvider request-id/lookup scheme for what is otherwise a single popup).
 */
class QuickICPreview : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("QuickICPreview is only ever exposed via AppController.icPreview")

    // FINAL: QuickICPreview is never subclassed, matching every other presenter in this Quick
    // chrome since the AOT-compilation refactor (see project_qml_aot_compilation_fusion_style_pin
    // memory).
    Q_PROPERTY(bool visible READ isVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString title READ title NOTIFY refreshed FINAL)
    Q_PROPERTY(QString imageUrl READ imageUrl NOTIFY refreshed FINAL)
    Q_PROPERTY(QPoint screenPos READ screenPos NOTIFY refreshed FINAL)

public:
    explicit QuickICPreview(QObject *parent = nullptr);

    /// Binds this presenter to \a canvas's icPreview*() signals; passing nullptr unbinds and
    /// hides any pending/visible popup. Mirrors QuickElementEditor::setCanvas()'s shape.
    void setCanvas(CanvasItem *canvas);

    [[nodiscard]] bool isVisible() const { return m_visible; }
    [[nodiscard]] QString title() const { return m_title; }
    /// A "data:image/png;base64,..." URL directly usable as a QML Image.source, or empty if
    /// the pending IC has no renderable preview (title-only display).
    [[nodiscard]] QString imageUrl() const { return m_imageUrl; }
    /// Global screen position the popup should appear near -- ICPreviewPopup.qml converts this
    /// to its parent's local coordinates via Window.contentItem.mapFromGlobal() before
    /// positioning itself.
    [[nodiscard]] QPoint screenPos() const { return m_screenPos; }

signals:
    void refreshed();

private:
    void onPreviewRequested(GraphicElement *ic, QPoint screenPos);
    /// Mirrors SceneUiBinder::bind()'s icPreviewMoved handler: if the popup is already
    /// showing/pending for this exact \a ic, just updates the pending position; otherwise
    /// behaves like a fresh onPreviewRequested().
    void onPreviewMoved(GraphicElement *ic, QPoint screenPos);
    void onPreviewHideRequested();
    /// Mirrors SceneUiBinder::bind()'s icPreviewCancelRequested handler: unconditionally
    /// cancels any pending show and hides the popup outright (no delay), regardless of which
    /// IC \a ic is.
    void onPreviewCancelRequested(GraphicElement *ic);

    /// Mirrors ICPreviewPopup::showForIC(): schedules the popup to appear near \a screenPos
    /// after the show delay, or updates it immediately if already visible.
    void showForIC(GraphicElement *ic, const QPoint &screenPos);
    /// Mirrors ICPreviewPopup::executeShow(): renders the preview image, sets
    /// title/imageUrl/screenPos, and marks the popup visible -- or hides it if the pending IC
    /// has neither a title nor a renderable image.
    void executeShow();
    /// Mirrors ICPreviewPopup::scheduleHide(): starts the hide-delay timer, cancelling any
    /// pending show.
    void scheduleHide();
    /// Mirrors ICPreviewPopup::cancelHide(): stops both timers.
    void cancelHide();
    /// Mirrors ICPreviewPopup::updatePendingPos(): updates the position the popup will appear
    /// at when the show timer fires. No effect if the popup is already visible.
    void updatePendingPos(const QPoint &screenPos);
    /// Mirrors ICPreviewPopup::isShowActiveFor(): true while a show is pending (timer running)
    /// or the popup is already visible for \a ic.
    [[nodiscard]] bool isShowActiveFor(const GraphicElement *ic) const;
    /// Marks the popup hidden and notifies, if it wasn't already.
    void hidePopup();

    CanvasItem *m_canvas = nullptr;

    QTimer m_showTimer;
    QTimer m_hideTimer;
    /// Auto-nulled if the pending IC is deleted while a timer is running -- mirrors
    /// ICPreviewPopup::m_pendingIC's own QPointer<IC> guard.
    QPointer<GraphicElement> m_pendingIc;
    QPoint m_pendingScreenPos;

    bool m_visible = false;
    QString m_title;
    QString m_imageUrl;
    QPoint m_screenPos;
};
