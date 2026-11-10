// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDockWidget>
#include <QHash>
#include <QString>

class QLabel;
class QListWidget;
class QListWidgetItem;
class Scene;

class WarningsDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit WarningsDock(QWidget *parent = nullptr);

    /// Binds to \a scene and recomputes the warning list.
    void setScene(Scene *scene);

    /// Returns the scene currently observed.
    Scene *scene() const { return m_scene; }

    /// Re-derives all warnings from the active scene and updates rows in place.
    void refresh();

    /// Re-derives displayed texts for runtime language changes.
    void retranslateUi();

    /// Total count of invalid input ports across all elements.
    int issueCount() const { return m_issueCount; }

    /// Internal list widget displaying issue rows.
    QListWidget *warningsList() const { return m_warnings; }

    /// Header label displaying total issue count.
    QLabel *countLabel() const { return m_count; }

signals:
    /// Emitted when the user activates an element row in the dock.
    void warningElementActivated(int elementId);

private:
    QLabel *m_count = nullptr;
    QListWidget *m_warnings = nullptr;
    Scene *m_scene = nullptr;
    int m_issueCount = 0;
    QHash<int, QListWidgetItem *> m_items;
};
