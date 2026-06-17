// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

struct ExerciseElementRequirement {
    QString typeName;
    int     minCount = 1;
};

struct ExerciseConnectionRequirement {
    QString fromTypeName;
    QString toTypeName;
    int     minCount = 1;
};

/// One step in a circuit exercise.
/// A step whose requiredElements AND requiredConnections are both empty is an
/// "observe" step: the engine never auto-advances it; the user must click Next/Finish.
///
/// \a show, \a hide, and \a click list widget/action IDs handled by MainWindow
/// before the step is presented.  Execution order: click → hide → show.
struct ExerciseStep {
    QString instruction;
    QString hint;
    QVector<ExerciseElementRequirement>    requiredElements;
    QVector<ExerciseConnectionRequirement> requiredConnections;
    QStringList show;    ///< Widget IDs to make visible on step enter.
    QStringList hide;    ///< Widget IDs to hide on step enter.
    QStringList click;   ///< Widget/action IDs to activate on step enter.
    QString context;     ///< "" or "circuit" → overlay on WorkSpace; "bwd" → overlay on BeWavedDolphin.
};
