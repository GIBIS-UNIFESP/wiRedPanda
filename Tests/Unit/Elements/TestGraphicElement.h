// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

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

    // Regression coverage for the inline double-click rename/edit feature (#7/#38).
    void testInlineDoubleClickRenamesLabelWithUndo();
    void testInlineDoubleClickEscapeCancelsWithoutUndo();
    void testInlineDoubleClickIgnoredWithoutLabel();
    void testTextEmptyStateHintTogglesWithLabelContent();
};
