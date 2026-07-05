// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QStringList>

/// One step in a guided interface tour.
/// \a target is a string key resolved by MainWindow::resolveTourTarget() to
/// the widget that should be spotlighted.  An empty or "none" target shows
/// a centered callout with the whole window dimmed uniformly.
///
/// \a click lists widget/action IDs handled by MainWindow before the spotlight is painted.
struct TourStep {
    QString title;
    QString body;
    QString target;
    QStringList click;  ///< Widget/action IDs to activate on step enter.
};
