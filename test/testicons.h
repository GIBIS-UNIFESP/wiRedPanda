/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef TESTICONS_H
#define TESTICONS_H

#include <QObject>
#include <QTest>

class TestIcons : public QObject
{
    Q_OBJECT
private slots:
    void testIcons();
};

#endif /* TESTICONS_H */