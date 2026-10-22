// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include "App/Core/ThemeManager.h"

/**
 * \brief Builds every element type at every legal port count, and the full-adder IC, in the sixteen orientations,
 * and compares what the port layout produced with a golden.
 * \details Each orientation records the element's bounds and picture and, per port, its name, its position in the
 * element and in the scene, its transform, its required flag and its default. Each configuration is also reached by
 * orienting first and changing the count after (for the IC, loading its file after), written only where that
 * differs. The port names are listed again under a translator that tags every string. The pictures' paths name the
 * theme, so the class runs on the Light theme.
 */
class TestPortLayoutSnapshot : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testLayoutMatchesGolden();

private:
    Theme m_previousTheme = Theme::System;
};
