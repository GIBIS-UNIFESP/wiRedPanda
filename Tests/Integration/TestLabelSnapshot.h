// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief Pins where each element's label sits and how its glyphs are turned, independent of fonts.
 * \details Every type that shows a label, at its fewest and its most ports, and the full-adder IC, placed off the
 * origin in the sixteen orientations, records its label's scene transform; each orientation is applied after the
 * ports and, written only where it differs, before them. The types without a label are listed as such. The label's
 * size depends on the machine's fonts and is not recorded.
 */
class TestLabelSnapshot : public QObject
{
    Q_OBJECT

private slots:
    void testLabelGeometryMatchesGolden();
};
