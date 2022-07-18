// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testlogicelements.h"

#include "logicand.h"
#include "logicdemux.h"
#include "logicdflipflop.h"
#include "logicdlatch.h"
#include "logicinput.h"
#include "logicjkflipflop.h"
#include "logicmux.h"
#include "logicnode.h"
#include "logicor.h"
#include "logicsrflipflop.h"
#include "logictflipflop.h"

#include <QTest>

void TestLogicElements::init()
{
    std::generate(switches.begin(), switches.end(), [] { return new LogicInput(); });
}

void TestLogicElements::cleanup()
{
    qDeleteAll(switches);
}

void TestLogicElements::testLogicNode()
{
    LogicNode elm;
    elm.connectPredecessor(0, switches.at(0), 0);

    const QVector<QVector<bool>> truthTable{
        {1, 1},
        {0, 0},
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(0));

        elm.updateLogic();

        QCOMPARE(elm.outputValue(), test.at(1));
    }
}

void TestLogicElements::testLogicAnd()
{
    LogicAnd elm(2);
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);

    const QVector<QVector<bool>> truthTable{
        {1, 1, 1},
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 0},
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(0));
        switches.at(1)->setOutputValue(test.at(1));

        elm.updateLogic();

        QCOMPARE(elm.outputValue(), test.at(2));
    }
}

void TestLogicElements::testLogicOr()
{
    LogicOr elm(2);
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);

    const QVector<QVector<bool>> truthTable{
        {1, 1, 1},
        {1, 0, 1},
        {0, 1, 1},
        {0, 0, 0},
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(0));
        switches.at(1)->setOutputValue(test.at(1));

        elm.updateLogic();

        QCOMPARE(elm.outputValue(), test.at(2));
    }
}

void TestLogicElements::testLogicInput()
{
    LogicInput elm;
    QCOMPARE(elm.outputValue(), false);
    elm.setOutputValue(true);
    QCOMPARE(elm.outputValue(), true);
    elm.setOutputValue(false);
    QCOMPARE(elm.outputValue(), false);
}

void TestLogicElements::testLogicMux()
{
    LogicMux elm;
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);
    elm.connectPredecessor(2, switches.at(2), 0);

    const QVector<QVector<bool>> truthTable{
        {0, 0, 0, 0},
        {0, 0, 1, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 1},
        {1, 0, 0, 1},
        {1, 0, 1, 0},
        {1, 1, 0, 1},
        {1, 1, 1, 1},
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(0));
        switches.at(1)->setOutputValue(test.at(1));
        switches.at(2)->setOutputValue(test.at(2));

        elm.updateLogic();

        QCOMPARE(elm.outputValue(), test.at(3));
    }
}

void TestLogicElements::testLogicDemux()
{
    LogicDemux elm;
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);

    const QVector<QVector<bool>> truthTable{
      /* i  S  o0 o1 */
        {0, 0, 0, 0},
        {0, 1, 0, 0},
        {1, 0, 1, 0},
        {1, 1, 0, 1},
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(0));
        switches.at(1)->setOutputValue(test.at(1));

        elm.updateLogic();

        QCOMPARE(elm.outputValue(0), test.at(2));
        QCOMPARE(elm.outputValue(1), test.at(3));
    }
}

void TestLogicElements::testLogicDFlipFlop()
{
    LogicDFlipFlop elm;
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);
    elm.connectPredecessor(2, switches.at(2), 0);
    elm.connectPredecessor(3, switches.at(3), 0);

    const QVector<QVector<bool>> truthTable{
      /* L  D  C  p  c  Q ~Q */
        {0, 0, 1, 1, 1, 0, 1}, /* Clk up and D = 0 */
        {0, 1, 1, 1, 1, 1, 0}, /* Clk up and D = 1 */

        {0, 0, 0, 0, 1, 1, 0}, /* Preset = false */
        {0, 0, 1, 1, 0, 0, 1}, /* Clear = false */
        {0, 0, 1, 0, 0, 1, 1}, /* Clear and Preset = false */

        {1, 0, 0, 1, 1, 1, 0}, /* Clk dwn and D = 0 (must maintain current state)*/
        {1, 1, 0, 1, 1, 1, 0}, /* Clk dwn and D = 1 (must maintain current state)*/
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(1)); /* DATA */
        switches.at(1)->setOutputValue(test.at(0)); /*  CLK */
        switches.at(2)->setOutputValue(false);      /* PRST */
        switches.at(3)->setOutputValue(false);      /* CLR */

        elm.updateLogic();

        elm.setOutputValue(0, test.at(0));
        elm.setOutputValue(1, !test.at(0));

        for (int port = 0; port < 4; ++port) {
            switches.at(port)->setOutputValue(test.at(port + 1));
        }

        elm.updateLogic();

        QCOMPARE(elm.outputValue(0), test.at(5));
        QCOMPARE(elm.outputValue(1), test.at(6));
    }
}

void TestLogicElements::testLogicDLatch()
{
    LogicDLatch elm;
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);

    const QVector<QVector<bool>> truthTable{
      /* D  E  Q  A */
        {0, 0, 0, 0},
        {0, 0, 1, 1},
        {1, 0, 0, 0},
        {1, 0, 1, 1},
        {0, 1, 0, 1},
        {1, 1, 1, 0},
    };

    for (const auto &test : truthTable) {
        elm.setOutputValue(0, test.at(3));
        elm.setOutputValue(1, !test.at(3));

        for (int port = 0; port < 2; ++port) {
            switches.at(port)->setOutputValue(test.at(port));
        }

        elm.updateLogic();

        QCOMPARE(elm.outputValue(0), test.at(2));
        QCOMPARE(elm.outputValue(1), !test.at(2));
    }
}

void TestLogicElements::testLogicJKFlipFlop()
{
    LogicJKFlipFlop elm;
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);
    elm.connectPredecessor(2, switches.at(2), 0);
    elm.connectPredecessor(3, switches.at(3), 0);
    elm.connectPredecessor(4, switches.at(4), 0);

    const QVector<QVector<bool>> truthTable{
      /* L  J  C  K  p  c  Q  Q  A */
        {0, 0, 0, 0, 0, 1, 1, 0, 0}, /* Preset = false */
        {0, 0, 0, 0, 1, 0, 0, 1, 0}, /* Clear = false */
        {0, 0, 0, 1, 0, 0, 1, 1, 0}, /* Clear and Preset = false*/
        {1, 1, 0, 0, 1, 1, 0, 1, 0}, /* Clk dwn and J = 0 (must maintain current state)*/
        {1, 1, 0, 0, 1, 1, 0, 1, 0}, /* Clk dwn and J = 1 (must maintain current state)*/
        {0, 1, 1, 1, 1, 1, 1, 0, 0}, /* Clk up J = 1 K = 1 (must swap Q and ~Q)*/
        {0, 1, 1, 1, 1, 1, 0, 1, 1}, /* Clk up J = 1 K = 1 (must swap Q and ~Q)*/
        {0, 1, 1, 0, 1, 1, 1, 0, 0}, /* Clk up J = 1 K = 0 */
        {0, 1, 1, 0, 1, 1, 1, 0, 1}, /* Clk up J = 1 K = 0 */
        {0, 0, 1, 1, 1, 1, 0, 1, 0}, /* Clk up J = 0 K = 1 */
        {0, 0, 1, 1, 1, 1, 0, 1, 1}, /* Clk up J = 0 K = 1 */
        {0, 0, 1, 0, 1, 1, 0, 1, 0}, /* Clk up J = 0 K = 0 */
        {0, 0, 1, 0, 1, 1, 1, 0, 1}, /* Clk up J = 0 K = 0 */
    };

    elm.updateLogic();

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(1));
        switches.at(1)->setOutputValue(test.at(0));
        switches.at(2)->setOutputValue(test.at(3));
        switches.at(3)->setOutputValue(false);
        switches.at(4)->setOutputValue(false);

        elm.updateLogic();

        elm.setOutputValue(0, test.at(8));
        elm.setOutputValue(1, !test.at(8));

        for (int port = 0; port < 5; ++port) {
            switches.at(port)->setOutputValue(test.at(port + 1));
        }

        elm.updateLogic();

        QCOMPARE(elm.outputValue(0), test.at(6));
        QCOMPARE(elm.outputValue(1), test.at(7));
    }
}

void TestLogicElements::testLogicSRFlipFlop()
{
    LogicSRFlipFlop elm;
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);
    elm.connectPredecessor(2, switches.at(2), 0);
    elm.connectPredecessor(3, switches.at(3), 0);
    elm.connectPredecessor(4, switches.at(4), 0);

    const QVector<QVector<bool>> truthTable{
      /* L  S  C  R  p  c  Q  Q  A */
        {0, 0, 0, 0, 0, 1, 1, 0, 0}, /* Preset = false */
        {0, 0, 0, 0, 1, 0, 0, 1, 1}, /* Clear = false*/
        {0, 0, 0, 0, 0, 0, 1, 1, 1}, /* Preset || Clear = false*/

        {0, 0, 0, 0, 1, 1, 0, 1, 0}, /* No change */
        {0, 0, 0, 0, 1, 1, 1, 0, 1}, /* No change */
        {0, 1, 0, 0, 1, 1, 0, 1, 0}, /* No change */
        {0, 1, 0, 0, 1, 1, 1, 0, 1}, /* No change */

        {0, 0, 0, 1, 1, 1, 0, 1, 0}, /* No change */
        {0, 0, 0, 1, 1, 1, 1, 0, 1}, /* No change */
        {1, 0, 1, 0, 1, 1, 0, 1, 0}, /* No change */
        {1, 0, 1, 0, 1, 1, 1, 0, 1}, /* No change */

        {1, 1, 1, 0, 1, 1, 0, 1, 0}, /* No change */
        {1, 1, 1, 0, 1, 1, 1, 0, 1}, /* No change */
        {1, 0, 1, 1, 1, 1, 0, 1, 0}, /* No change */
        {1, 0, 1, 1, 1, 1, 1, 0, 1}, /* No change */

        {0, 0, 1, 0, 1, 1, 0, 1, 0}, /* No change */
        {0, 0, 1, 0, 1, 1, 1, 0, 1}, /* No change */
        {0, 0, 1, 1, 1, 1, 0, 1, 0}, /* Q = 0 */
        {0, 0, 1, 1, 1, 1, 0, 1, 1}, /* Q = 0 */

        {0, 1, 1, 0, 1, 1, 1, 0, 0}, /* Q = 1 */
        {0, 1, 1, 0, 1, 1, 1, 0, 1}, /* Q = 1 */
        {0, 1, 1, 1, 1, 1, 1, 1, 0}, /* Not permitted */
        {0, 1, 1, 1, 1, 1, 1, 1, 1}, /* Not permitted */
    };

    elm.updateLogic();

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(false);
        switches.at(1)->setOutputValue(test.at(0));
        switches.at(2)->setOutputValue(false);

        elm.updateLogic();

        elm.setOutputValue(0, test.at(8));
        elm.setOutputValue(1, !test.at(8));

        for (int port = 0; port < 5; ++port) {
            switches.at(port)->setOutputValue(test.at(port + 1));
        }

        elm.updateLogic();

        QCOMPARE(elm.outputValue(0), test.at(6));
        QCOMPARE(elm.outputValue(1), test.at(7));
    }
}

void TestLogicElements::testLogicTFlipFlop()
{
    LogicTFlipFlop elm;
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);
    elm.connectPredecessor(2, switches.at(2), 0);
    elm.connectPredecessor(3, switches.at(3), 0);

    const QVector<QVector<bool>> truthTable{
      /* L  T  C  p  c  Q ~Q  A */
        {1, 0, 1, 1, 1, 0, 1, 0}, /* No change */
        {1, 1, 1, 1, 1, 0, 1, 0}, /* No change */
        {1, 0, 1, 1, 1, 1, 0, 1}, /* No change */
        {1, 1, 1, 1, 1, 1, 0, 1}, /* No change */

        {1, 0, 0, 1, 1, 0, 1, 0}, /* No change */
        {1, 0, 0, 1, 1, 1, 0, 1}, /* No change */

        {0, 1, 1, 1, 1, 1, 0, 0}, /* Toggle */
        {0, 1, 1, 1, 1, 0, 1, 1}, /* Toggle */

        {1, 0, 1, 0, 1, 1, 0, 0}, /* Preset = false */
        {1, 0, 1, 1, 0, 0, 1, 1}, /* Clear = false */
        {1, 0, 1, 0, 0, 1, 1, 1}, /* Clear and Preset = false */

        /* Test Prst and clr */
    };

    elm.updateLogic();

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(1)); // T
        switches.at(1)->setOutputValue(test.at(0)); // CLK
        switches.at(2)->setOutputValue(true);
        switches.at(3)->setOutputValue(true);

        elm.updateLogic();

        elm.setOutputValue(0, test.at(7));
        elm.setOutputValue(1, !test.at(7));

        for (int port = 0; port < 4; ++port) {
            switches.at(port)->setOutputValue(test.at(port + 1));
        }

        elm.updateLogic();

        QCOMPARE(elm.outputValue(0), test.at(5));
        QCOMPARE(elm.outputValue(1), test.at(6));
    }
}
