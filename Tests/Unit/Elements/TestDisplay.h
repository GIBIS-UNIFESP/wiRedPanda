// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestDisplay : public QObject
{
    Q_OBJECT

private slots:

    void testDisplay14AllColors();
    void testDisplay16AllColors();
    void testDisplay14NextPrevColor();
    void testDisplay16NextPrevColor();
    void testDisplay14Save();
    void testDisplay16Save();
};

