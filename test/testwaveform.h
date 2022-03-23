/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TESTWAVEFORM_H
#define TESTWAVEFORM_H

#include <QObject>
#include <QTest>

#include "editor.h"

class TestWaveForm : public QObject
{
    Q_OBJECT

    Editor *editor;

private slots:

    /* functions executed by QtTest before and after each test */
    void init();
    void cleanup();
    void testDisplay4Bits();
};

#endif /* TESTWAVEFORM_H */