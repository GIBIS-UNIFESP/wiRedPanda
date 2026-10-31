// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/// ElementPainter: an element's selection drawn under its body, wholly in the theme handed, and an element drawing
/// itself as the painter draws it in the current theme.
class TestElementPainter : public QObject
{
    Q_OBJECT

private slots:
    void testASelectedElementHasItsSelectionUnderItsBody();
    void testAnElementIsDrawnWhollyInTheThemeHanded();
    void testAnElementDrawsItselfWithThePainter();
};
