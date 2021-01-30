/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TESTSIMULATIONCONTROLLER_H
#define TESTSIMULATIONCONTROLLER_H

#include <QObject>
#include <QTest>

#include "editor.h"
#include "graphicelement.h"

class TestSimulationController : public QObject
{
    Q_OBJECT

    QVector<GraphicElement *> elms;
    Editor *editor;
private slots:

    /* functions executed by QtTest before and after each test */
    void init();
    void cleanup();
    void testCase1();
};

#endif /* TESTSIMULATIONCONTROLLER_H */