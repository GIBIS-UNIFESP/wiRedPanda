// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestElementEditor : public QObject
{
    Q_OBJECT

private slots:

    void testCreation();
    void testSetScene();
    void testRetranslateUi();
    void testSetCurrentElementsEmpty();
    void testSetCurrentElementsGate();
    void testSetCurrentElementsLed();
    void testFillColorComboBox();
};

