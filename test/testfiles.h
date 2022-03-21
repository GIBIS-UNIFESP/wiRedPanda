/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TESTFILES_H
#define TESTFILES_H

#include <QObject>
#include <QTest>

#include "editor.h"

class TestFiles : public QObject
{
    Q_OBJECT
    Editor *editor;
private slots:

    /* functions executed by QtTest before and after each test */
    void init();
    void cleanup();

    void testFiles();
};

#endif /* TESTFILES_H */
