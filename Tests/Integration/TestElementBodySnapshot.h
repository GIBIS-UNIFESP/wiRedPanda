// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief Pins every element type's body: where it is, and what it draws.
 * \details Each type is put through the states that change its picture or its bounds (port counts, on and off,
 * colours, segment patterns, positions, wireless modes and signal colours, playing audio, custom appearances, an IC
 * from a file and embedded), each in the sixteen orientations. The geometry (bounds, pivot, item transform, displayed
 * picture, the child items that draw) goes to a text golden; the pixels the element and its child items paint,
 * unselected and selected, go to one PNG golden per type, an atlas with a row per state and selection and a column per
 * orientation, compared within TestUtils::kPixelChannelTolerance. What a Text element's hint and label decide is
 * masked, since their glyphs fall where this platform's font puts them; the bundled pictures draw their letters as
 * outlines, compared like every other pixel. The pictures that follow the theme are pinned in both themes. Goldens live
 * in Tests/Integration/BodySnapshot, with the few atlases 32-bit x86's rasteriser draws apart in its i386 directory;
 * GENERATE_EXPECTED_SNAPSHOTS=1 regenerates them.
 */
class TestElementBodySnapshot : public QObject
{
    Q_OBJECT

private slots:
    void testEveryTypeHasACase();
    void testGeometryMatchesGolden();
    void testPixelsMatchGoldens_data();
    void testPixelsMatchGoldens();
};
