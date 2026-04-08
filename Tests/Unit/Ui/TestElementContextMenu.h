// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestElementContextMenu : public QObject
{
    Q_OBJECT

private slots:

    // Context menu action tests (3 tests)
    void testRotateRightAction();
    void testMorphMenuAction();
    void testICSubcircuitAction();
};

