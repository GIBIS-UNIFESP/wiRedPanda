// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestICUnit : public QObject
{
    Q_OBJECT

private slots:

    void testICLoadFromFile();
    void testICPortLabelResolution();
    void testICNestedSaveLoad();
    void testICInvalidFile();
};

