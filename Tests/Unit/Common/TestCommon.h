// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCommon : public QObject
{
    Q_OBJECT

private slots:

    // Exception Tests (2 tests)
    void testPandaceptionMessage();
    void testPandaceptionEnglishMessage();

    // Regression: F8 — verbosity 5 enables category five; >5 doesn't silence
    void testSetVerbosityCategoryScale();
};
