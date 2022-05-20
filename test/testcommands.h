/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>

class TestCommands : public QObject
{
    Q_OBJECT

private slots:
    void testAddDeleteCommands();
};
