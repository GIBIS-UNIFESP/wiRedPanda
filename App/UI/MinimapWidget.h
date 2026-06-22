// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <QPixmap>

class Scene;
class GraphicsView;

class MinimapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MinimapWidget(Scene *scene, GraphicsView *view, QWidget *parent = nullptr);

    QSize sizeHint() const override;

public slots:
    void invalidateCache();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override; // swallow wheel events so main view doesn't zoom
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Scene *m_scene = nullptr;
    GraphicsView *m_view = nullptr;
    QPixmap m_cache;           ///< cached rendering of the scene for fast repaints
    bool m_cacheDirty = true;  ///< whether cache needs to be regenerated
    bool m_dragging = false;
    QPoint m_lastPos;
};
