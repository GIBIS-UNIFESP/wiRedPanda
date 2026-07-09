// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QDialog>
#include <QString>

#include "App/UI/ContentBrowserDialogUI.h"

/// Dialog for browsing and launching available circuit exercises or guided tours, selected
/// by \a category — the two shared an ~identical implementation (ExerciseBrowserDialog and
/// TourBrowserDialog) differing only in a handful of translated strings and which Settings/
/// ExerciseTourResources category key they use.
class ContentBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ContentBrowserDialog(ContentCategory category, QWidget *parent = nullptr);
    ~ContentBrowserDialog() override;

    /// Returns the resource path of the selected item, or empty if cancelled.
    QString selectedPath() const;

private:
    void populateList();
    void onSelectionChanged();

    /// "Exercises" or "Tours" — the discover()/preferredContentDir() key.
    QString categoryKey() const;
    /// Settings::completedExercises()/completedTours().
    QStringList completedItems() const;

    ContentCategory m_category;
    std::unique_ptr<ContentBrowserDialogUi> m_ui;
    QString m_selectedPath;
};
