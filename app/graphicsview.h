// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QGraphicsView>

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphicsView(QWidget *parent = nullptr);

    bool canZoomIn() const;
    bool canZoomOut() const;
    void resetZoom();
    void setFastMode(const bool fastMode);
    void setRedirectZoom(const bool value);
    void zoomIn();
    void zoomOut();

signals:
    void scaleIn();
    void scaleOut();
    void zoomChanged();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    bool m_pan = false;
    bool m_redirectZoom = false;
    bool m_space = false;
    int m_panStartX = 0;
    int m_panStartY = 0;
    int m_zoomLevel = 0;
};
