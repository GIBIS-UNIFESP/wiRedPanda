// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
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
#include "logicnand.h"
#include "logicnor.h"
#include "logicnot.h"
#include "logicxor.h"
#include "logicxnor.h"
#include "logicoutput.h"
#include "logicsrlatch.h"
#include "logictruthtable.h"

#include <QTest>
#include <QBitArray>

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

void TestLogicElements::testLogicNAND()
{
    LogicNand elm(2);
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);

    const QVector<QVector<bool>> truthTable{
        {false, false, true},
        {false, true, true},
        {true, false, true},
        {true, true, false},
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(0));
        switches.at(1)->setOutputValue(test.at(1));
        
        elm.updateLogic();
        
        QCOMPARE(elm.outputValue(), test.at(2));
    }
}

void TestLogicElements::testLogicNOR()
{
    LogicNor elm(2);
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);

    const QVector<QVector<bool>> truthTable{
        {false, false, true},
        {false, true, false},
        {true, false, false},
        {true, true, false},
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(0));
        switches.at(1)->setOutputValue(test.at(1));
        
        elm.updateLogic();
        
        QCOMPARE(elm.outputValue(), test.at(2));
    }
}

void TestLogicElements::testLogicNOT()
{
    LogicNot elm;
    elm.connectPredecessor(0, switches.at(0), 0);

    const QVector<QVector<bool>> truthTable{
        {false, true},
        {true, false},
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(0));
        
        elm.updateLogic();
        
        QCOMPARE(elm.outputValue(), test.at(1));
    }
}

void TestLogicElements::testLogicXOR()
{
    LogicXor elm(2);
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);

    const QVector<QVector<bool>> truthTable{
        {false, false, false},
        {false, true, true},
        {true, false, true},
        {true, true, false},
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(0));
        switches.at(1)->setOutputValue(test.at(1));
        
        elm.updateLogic();
        
        QCOMPARE(elm.outputValue(), test.at(2));
    }
}

void TestLogicElements::testLogicXNOR()
{
    LogicXnor elm(2);
    elm.connectPredecessor(0, switches.at(0), 0);
    elm.connectPredecessor(1, switches.at(1), 0);

    const QVector<QVector<bool>> truthTable{
        {false, false, true},
        {false, true, false},
        {true, false, false},
        {true, true, true},
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(0));
        switches.at(1)->setOutputValue(test.at(1));
        
        elm.updateLogic();
        
        QCOMPARE(elm.outputValue(), test.at(2));
    }
}

void TestLogicElements::testLogicOutput()
{
    LogicOutput elm(1);
    elm.connectPredecessor(0, switches.at(0), 0);

    switches.at(0)->setOutputValue(false);
    elm.updateLogic();
    QCOMPARE(elm.outputValue(), false);

    switches.at(0)->setOutputValue(true);
    elm.updateLogic();
    QCOMPARE(elm.outputValue(), true);
}

void TestLogicElements::testLogicTruthTable()
{
    // Test basic 2-input, 1-output AND truth table
    QBitArray andTruthTable(4); // 2^2 = 4 combinations for 2 inputs
    andTruthTable[0] = false; // 00 -> 0
    andTruthTable[1] = false; // 01 -> 0
    andTruthTable[2] = false; // 10 -> 0
    andTruthTable[3] = true;  // 11 -> 1
    
    LogicTruthTable andTable(2, 1, andTruthTable);
    andTable.connectPredecessor(0, switches.at(0), 0);
    andTable.connectPredecessor(1, switches.at(1), 0);
    
    // Test all combinations for AND operation
    const QVector<QVector<bool>> testCases{
        {false, false, false}, // 00 -> 0
        {false, true, false},  // 01 -> 0
        {true, false, false},  // 10 -> 0
        {true, true, true}     // 11 -> 1
    };
    
    for (const auto &testCase : testCases) {
        switches.at(0)->setOutputValue(testCase[0]);
        switches.at(1)->setOutputValue(testCase[1]);
        andTable.updateLogic();
        QCOMPARE(andTable.outputValue(), testCase[2]);
    }
    
    // Test basic 1-input, 1-output NOT truth table
    QBitArray notTruthTable(2); // 2^1 = 2 combinations for 1 input
    notTruthTable[0] = true;  // 0 -> 1
    notTruthTable[1] = false; // 1 -> 0
    
    LogicTruthTable notTable(1, 1, notTruthTable);
    notTable.connectPredecessor(0, switches.at(0), 0);
    
    // Test NOT operation
    switches.at(0)->setOutputValue(false);
    notTable.updateLogic();
    QCOMPARE(notTable.outputValue(), true);
    
    switches.at(0)->setOutputValue(true);
    notTable.updateLogic();
    QCOMPARE(notTable.outputValue(), false);
    
    // Test multi-output truth table (e.g., half adder: sum and carry)
    QBitArray halfAdderTable(512); // 2 outputs * 256 entries per output
    // For 2 inputs: sum = A XOR B, carry = A AND B
    // Output 0 (sum): positions 0-3
    halfAdderTable[0] = false; // 00 -> sum=0
    halfAdderTable[1] = true;  // 01 -> sum=1
    halfAdderTable[2] = true;  // 10 -> sum=1
    halfAdderTable[3] = false; // 11 -> sum=0
    // Output 1 (carry): positions 256-259
    halfAdderTable[256] = false; // 00 -> carry=0
    halfAdderTable[257] = false; // 01 -> carry=0
    halfAdderTable[258] = false; // 10 -> carry=0
    halfAdderTable[259] = true;  // 11 -> carry=1
    
    LogicTruthTable halfAdder(2, 2, halfAdderTable);
    halfAdder.connectPredecessor(0, switches.at(0), 0);
    halfAdder.connectPredecessor(1, switches.at(1), 0);
    
    // Test half adder functionality
    const QVector<QVector<bool>> halfAdderCases{
        {false, false, false, false}, // 00 -> sum=0, carry=0
        {false, true, true, false},   // 01 -> sum=1, carry=0
        {true, false, true, false},   // 10 -> sum=1, carry=0
        {true, true, false, true}     // 11 -> sum=0, carry=1
    };
    
    for (const auto &testCase : halfAdderCases) {
        switches.at(0)->setOutputValue(testCase[0]);
        switches.at(1)->setOutputValue(testCase[1]);
        halfAdder.updateLogic();
        QCOMPARE(halfAdder.outputValue(0), testCase[2]); // sum
        QCOMPARE(halfAdder.outputValue(1), testCase[3]); // carry
    }
}


