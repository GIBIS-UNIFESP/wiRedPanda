// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QDialog>
#include <QString>

#include "App/Tour/TourBrowserDialogUI.h"

/// Dialog for browsing and launching available guided tours.
class TourBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TourBrowserDialog(QWidget *parent = nullptr);
    ~TourBrowserDialog() override;

    /// Returns the resource path of the selected tour, or empty if cancelled.
    QString selectedTourPath() const;

private:
    void populateList();
    void onSelectionChanged();

    std::unique_ptr<TourBrowserDialogUi> m_ui;
    QString m_selectedPath;
};
