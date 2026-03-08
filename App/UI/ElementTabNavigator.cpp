// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ElementTabNavigator.h"

#include <algorithm>

#include <QEvent>
#include <QKeyEvent>
#include <QWidget>

#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/UI/ElementEditor.h"

ElementTabNavigator::ElementTabNavigator(ElementEditor *editor, QObject *parent)
    : QObject(parent)
    , m_editor(editor)
{
}

bool ElementTabNavigator::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() != QEvent::KeyPress) {
        return false;
    }

    auto *keyEvent = dynamic_cast<QKeyEvent *>(event);

    if (!keyEvent || (m_editor->m_elements.size() != 1)) {
        return false;
    }

    const bool moveFwd  = (keyEvent->key() == Qt::Key_Tab);
    const bool moveBack = (keyEvent->key() == Qt::Key_Backtab);

    if (!moveFwd && !moveBack) {
        return false;
    }

    Scene *scene = m_editor->m_scene;

    if (!scene) {
        return false;
    }

    auto *elm      = m_editor->m_elements.constFirst();
    auto  elements = scene->visibleElements();

    // Sort in reading order: left-to-right within each row, row-by-row top-to-bottom.
    std::stable_sort(elements.begin(), elements.end(), [](const auto &a, const auto &b) {
        return a->pos().ry() < b->pos().ry();
    });
    std::stable_sort(elements.begin(), elements.end(), [](const auto &a, const auto &b) {
        return a->pos().rx() < b->pos().rx();
    });

    const int elmPos = elements.indexOf(elm);
    const int step   = moveFwd ? 1 : -1;
    const int total  = elements.size();

    auto *widget = qobject_cast<QWidget *>(obj);
    int pos = (total + elmPos + step) % total;

    // Advance until we find an element whose editor widget is actually enabled
    // (some elements don't expose the currently focused field, so we skip them).
    for (; pos != elmPos; pos = (total + pos + step) % total) {
        elm = elements.at(pos);
        m_editor->setCurrentElements({elm});

        if (widget->isEnabled()) {
            break;
        }
    }

    scene->clearSelection();

    if (!widget->isEnabled()) {
        elm = elements.at(elmPos);
    }

    elm->setSelected(true);
    elm->ensureVisible();
    widget->setFocus();
    event->accept();
    return true;
}
