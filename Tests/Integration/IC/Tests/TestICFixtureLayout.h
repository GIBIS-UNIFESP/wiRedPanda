// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestICFixtureLayout : public QObject
{
    Q_OBJECT

private slots:
    void testLayout_data();
    void testLayout();
};
