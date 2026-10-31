// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/// The presenters: every type has one, and an element's body is the one its type's presenter measures.
class TestElementPresenters : public QObject
{
    Q_OBJECT

private slots:
    void testEveryTypeHasAPresenter();
    void testAnElementsBodyIsItsPresentersMeasure();
    void testTextDrawsItsHintAsASimpleTextItemDid();
};
