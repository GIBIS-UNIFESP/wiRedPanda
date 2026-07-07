// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

#include <QRect>
#include <QWidget>

class QFrame;

#include "App/Tour/TourStep.h"

class QLabel;
class QPushButton;
class TourEngine;

/// Full-window overlay for the guided interface tour.
///
/// Dims the entire centralWidget area and punches a spotlight hole over the
/// target widget.  A floating callout panel shows the step title, body text,
/// and Prev/Next navigation buttons.
class TourOverlay : public QWidget
{
    Q_OBJECT

public:
    using TargetResolver = std::function<QRect(const QString &targetId)>;

    explicit TourOverlay(TourEngine *engine, QWidget *parent = nullptr);

    /// Registers a callback that maps a target-id string to a highlight rect
    /// in the overlay's local coordinate space.  Called once after construction.
    void setTargetResolver(TargetResolver resolver);

    /// Re-parents the overlay to \a newParent and reinstalls the resize event filter.
    void setParentWindow(QWidget *newParent);

public slots:
    void onStepChanged(int step, int total, const TourStep &stepData);
    void onTourFinished();
    void onRetranslated();

signals:
    void closeRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUi();
    void repositionCallout();

    TourEngine     *m_engine          = nullptr;
    TargetResolver  m_resolver;
    QString         m_currentTarget;
    QRect           m_highlightRect;
    QColor          m_dimColor;
    QColor          m_spotlightColor;

    // Callout panel (child of this overlay)
    QFrame      *m_callout        = nullptr;
    QLabel      *m_stepCounter    = nullptr;
    QLabel      *m_titleLabel     = nullptr;
    QLabel      *m_bodyLabel      = nullptr;
    QPushButton *m_prevButton     = nullptr;
    QPushButton *m_nextButton     = nullptr;
    QPushButton *m_closeButton    = nullptr;

    void applyTheme();
};
