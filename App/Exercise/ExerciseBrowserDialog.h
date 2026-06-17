// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QDialog>
#include <QString>
#include <QVector>

#include "App/Exercise/ExerciseBrowserDialogUI.h"

/// Dialog for browsing and launching available circuit exercises.
class ExerciseBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExerciseBrowserDialog(QWidget *parent = nullptr);
    ~ExerciseBrowserDialog() override;

    /// Returns the resource path of the selected exercise, or empty if cancelled.
    QString selectedExercisePath() const;

private:
    struct ExerciseEntry {
        QString resourcePath;
        QString title;
        QString id;
        QString description;
    };

    static QVector<ExerciseEntry> availableExercises();

    void populateList();
    void onSelectionChanged();

    std::unique_ptr<ExerciseBrowserDialogUi> m_ui;
    QString m_selectedPath;
};
