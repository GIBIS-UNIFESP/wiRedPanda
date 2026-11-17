// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QWidget>

class QLabel;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class Scene;

/**
 * \class WarningsPanel
 * \brief Static sidebar panel displaying elements with port issues in the active scene.
 *
 * \details Derives all warning information on the fly from Port::isValid().
 * No warning state is stored on elements or translated ahead of display time.
 * Sits as the right-hand pane in MainWindow's central QSplitter.
 * Component rows are collapsible with details hidden by default to avoid clutter.
 */
class WarningsPanel : public QWidget
{
    Q_OBJECT
public:
    explicit WarningsPanel(QWidget *parent = nullptr);

    /// Binds the panel to \a scene, or clears the display when \a scene is null.
    void setScene(Scene *scene);

    /// Re-evaluates port validity for all elements in the bound scene and updates rows in place.
    void refresh();

    /// Re-applies translated titles and issue descriptions when the UI language changes.
    void retranslateUi();

    /// Total number of invalid ports across all elements in the bound scene.
    int issueCount() const { return m_issueCount; }

    /// Number of elements currently listed with warnings.
    int elementCount() const { return static_cast<int>(m_items.size()); }

    /// Underlying tree widget displaying the issue rows and expandable details.
    QTreeWidget *warningsTree() const { return m_warnings; }

    /// Header label displaying total issue count.
    QLabel *countLabel() const { return m_count; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    /// Emitted when the user activates an element row in the panel.
    void warningElementActivated(int elementId);

    /// Emitted when the user clicks the close button in the panel header.
    void closeRequested();

private:
    QLabel *m_header = nullptr;
    QLabel *m_count = nullptr;
    QLabel *m_description = nullptr;
    QLabel *m_emptyState = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QTreeWidget *m_warnings = nullptr;
    Scene *m_scene = nullptr;
    int m_issueCount = 0;
    QHash<int, QTreeWidgetItem *> m_items;
};
