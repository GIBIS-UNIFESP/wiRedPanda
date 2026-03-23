// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVector>

class InputVcc;

class TestNodeLogic : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();
    void init();
    void testNodeChainPropagation();
    void testNodeFanOut();

private:
    QVector<InputVcc *> m_inputs{8};
};

