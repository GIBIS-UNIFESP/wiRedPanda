// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief Pins master's IC components as an IC loads them: the hover preview and the pins.
 * \details For each of the 73 components: whether it has a hover preview and whether the preview draws anything on its
 * background, its port counts, and its pins in order, each by the inner element behind it (that element's id when the
 * component is loaded as a circuit), the element's port and the pin's label. The preview's size is not pinned, since
 * labels drawn in the platform's sans serif reach the circuit's bounds in most components; one component whose labels
 * stay inside them has its preview's pixels pinned, labels masked.
 */
class TestIcPreviewSnapshot : public QObject
{
    Q_OBJECT

private slots:
    void testComponentMatchesGolden_data();
    void testComponentMatchesGolden();
    void testEveryGoldenHasAComponent();
    void testPreviewPixelsMatchGolden();
};
