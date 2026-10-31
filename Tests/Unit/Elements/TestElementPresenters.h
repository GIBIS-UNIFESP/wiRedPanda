// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/// The presenters: every type has one, and an element's body and label are the ones its type's presenter measures and
/// places.
class TestElementPresenters : public QObject
{
    Q_OBJECT

private slots:
    void testEveryTypeHasAPresenter();
    void testAnElementsBodyIsItsPresentersMeasure();
    void testTextDrawsItsHintAsASimpleTextItemDid();
    void testEachTypePlacesItsLabel();
    void testTheLabelBoxIsTheTextAsDrawn_data();
    void testTheLabelBoxIsTheTextAsDrawn();
    void testTheLabelBoxFollowsTheTrigger();
    void testTextsBodyHoldsItsLabel();
};
