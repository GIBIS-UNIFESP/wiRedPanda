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

QVector<GraphicElement *> ElementTabNavigator::readingOrder(QVector<GraphicElement *> elements)
{
    // Reading order is row-major: top-to-bottom by Y (primary key), left-to-right by X within a
    // row (secondary key). std::stable_sort is stable, so sorting by the secondary key (X) first
    // and the primary key (Y) last leaves Y dominant while elements sharing a row keep their
    // left-to-right X order.
    std::stable_sort(elements.begin(), elements.end(), [](const auto &a, const auto &b) {
        return a->pos().rx() < b->pos().rx();
    });
    std::stable_sort(elements.begin(), elements.end(), [](const auto &a, const auto &b) {
        return a->pos().ry() < b->pos().ry();
    });
    return elements;
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

    auto *elm            = m_editor->m_elements.constFirst();
    const auto elements  = readingOrder(scene->visibleElements());

    const qsizetype elmPos = elements.indexOf(elm);
    if (elmPos < 0) {
        // The focused element isn't among the visible elements (e.g. it's hidden by a
        // visibility filter). elmPos == -1 can never be reached by the modular advance loop
        // below, so it would spin forever; elements.at(-1) would also be out of range. Let
        // the key event fall through to default handling instead.
        return false;
    }
    const qsizetype step   = moveFwd ? 1 : -1;
    const qsizetype total  = elements.size();

    auto *widget = qobject_cast<QWidget *>(obj);
    qsizetype pos = (total + elmPos + step) % total;

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
