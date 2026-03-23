// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ElementTabNavigator event filter for Tab/Backtab cycling between scene elements.
 */

#pragma once

#include <QObject>

class ElementEditor;
class Scene;

/**
 * \class ElementTabNavigator
 * \brief Event filter that cycles scene element selection on Tab / Shift+Tab.
 *
 * \details Install this on every interactive editor widget (label field, trigger
 * field, etc.) via QWidget::installEventFilter().  When the user presses Tab or
 * Backtab while one of those widgets has focus and exactly one element is
 * selected, the navigator advances the selection to the next (or previous)
 * visible element in reading order (left-to-right, top-to-bottom) that exposes
 * the same type of field.
 *
 * Scene access and element-jump callbacks are obtained through the ElementEditor
 * pointer; the navigator is declared a friend of ElementEditor so it can call
 * the private setCurrentElements() helper.
 */
class ElementTabNavigator : public QObject
{
    Q_OBJECT

public:
    /**
     * \brief Constructs the navigator.
     * \param editor The owning ElementEditor (provides scene access and selection control).
     * \param parent Optional QObject parent.
     */
    explicit ElementTabNavigator(ElementEditor *editor, QObject *parent = nullptr);

    /// \reimp
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    // --- Members ---

    ElementEditor *m_editor; ///< The owning ElementEditor; provides scene access and selection control.
};

