// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// GraphicElement has no QGraphicsItem inheritance and no Widgets-only inline-edit mechanism
/// (a QLineEdit hosted by a QGraphicsProxyWidget); CanvasItem's own QML-based inline-edit is
/// covered separately by TestCanvasItemInteraction.
class TestGraphicElement : public QObject
{
    Q_OBJECT

private slots:

    void testElementPaintSelection();
    void testElementSaveLoad();
    void testElementSkin();
    void testElementAppearance();
    void testElementTooltip();
    void testElementDoubleClick();
    void testTextElement();
    void testLineElement();
    void testDisplay14Paint();
    void testDisplay16Paint();

    void testTextEmptyStateHintTogglesWithLabelContent();

    // previousColor()/nextColor()/colorNameToIndex()'s fallback branches, for an element type
    // that never overrides color() (e.g. And, whose base color() always returns "")
    void testColorCycleFunctionsDefaultToWhiteForNonColoredElement();
    void testColorNameToIndexUnknownColorDefaultsToZero();

    // isValid()'s propagate-invalid-downstream branch
    void testIsValidPropagatesErrorStatusToConnectedOutputs();

    // Base setColor()/setAudio()/setVolume()/setFrequency()/setDelay() no-ops, and the
    // blobName() default, for an element type that doesn't override any of them
    void testBaseSettersAreNoOpsForUnsupportedProperties();
    void testBlobNameDefaultsToEmptyForNonIcElement();

    // retranslate()'s translated-name/tooltip/port-name refresh
    void testRetranslateUpdatesTranslatedNameToolTipAndPortName();
};
