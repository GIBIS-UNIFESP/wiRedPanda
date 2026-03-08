// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestDecoder8To256 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // 8-to-256 Decoder Tests (Memory Address Decoder)
    void test8to256Decoder();
};

