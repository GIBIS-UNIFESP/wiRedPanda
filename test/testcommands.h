/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TESTCOMMANDS_H
#define TESTCOMMANDS_H

#include <QObject>
#include <QTest>

#include "editor.h"

class TestCommands : public QObject
{
    Q_OBJECT
    Editor *editor;

private slots:
    void init();
    void cleanup();

    void testAddDeleteCommands();
};

#endif /* TESTCOMMANDS_H */