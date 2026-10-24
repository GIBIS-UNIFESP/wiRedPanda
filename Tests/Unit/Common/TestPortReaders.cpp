// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Common/TestPortReaders.h"

#include <memory>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

namespace TestPortReadersHelpers {

const Status kStatuses[] = {Status::Inactive, Status::Active, Status::Unknown, Status::Error};

/// Four LEDs, each wired to a switch so its input port is valid and holds whatever status it is given.
struct Leds {
    std::unique_ptr<WorkSpace> workspace = std::make_unique<WorkSpace>();
    InputSwitch *switches[4] = {};
    Led *leds[4] = {};

    Leds()
    {
        CircuitBuilder builder(workspace->scene());
        for (int i = 0; i < 4; ++i) {
            switches[i] = new InputSwitch();
            leds[i] = new Led();
            builder.add(switches[i], leds[i]);
            builder.connect(switches[i], 0, leds[i], 0);
        }
    }

    void hold(const QVector<Status> &statuses)
    {
        for (int i = 0; i < 4; ++i) {
            leds[i]->inputPort()->setStatus(statuses.at(i));
        }
    }
};

} // namespace TestPortReadersHelpers

using TestPortReadersHelpers::kStatuses;
using TestPortReadersHelpers::Leds;
using TestUtils::Bits;

void TestPortReaders::testInputStatusReturnsWhatThePortHolds()
{
    Leds f;
    for (const Status status : kStatuses) {
        f.leds[0]->inputPort()->setStatus(status);
        QCOMPARE(TestUtils::inputStatus(f.leds[0]), status);
    }
}

void TestPortReaders::testOutputStatusReturnsWhatThePortHolds()
{
    Leds f;
    for (const Status status : kStatuses) {
        f.switches[0]->outputPort()->setStatus(status);
        QCOMPARE(TestUtils::outputStatus(f.switches[0]), status);
    }
}

void TestPortReaders::testBitsEqualANumberOnlyWhenEveryBitIsALevel()
{
    QVERIFY(Bits({Status::Inactive, Status::Inactive}) == 0);
    QVERIFY(Bits({Status::Active, Status::Inactive}) == 1);
    QVERIFY(Bits({Status::Inactive, Status::Active}) == 2);

    // A non-level bit is neither 0 nor 1, so a test expecting either fails on it.
    for (const Status nonLevel : {Status::Unknown, Status::Error}) {
        const Bits bits({nonLevel, Status::Inactive});
        QVERIFY(!bits.isLevel());
        QVERIFY(bits != 0);
        QVERIFY(bits != 1);
    }

    // Two readings compare bit by bit, a non-level included.
    QVERIFY(Bits({Status::Unknown, Status::Active}) == Bits({Status::Unknown, Status::Active}));
    QVERIFY(Bits({Status::Unknown, Status::Active}) != Bits({Status::Error, Status::Active}));
}

void TestPortReaders::testBitsPrintMostSignificantFirst()
{
    const Bits bits({Status::Active, Status::Unknown, Status::Inactive, Status::Error});
    QCOMPARE(bits.text(), QString("E0U1"));

    // QCOMPARE prints a Bits through toString(), found by argument-dependent lookup.
    std::unique_ptr<char[]> printed(toString(bits));
    QCOMPARE(QString::fromUtf8(printed.get()), QString("\"E0U1\""));

    QCOMPARE(TestUtils::statusText(Status::Inactive), QString("0"));
    QCOMPARE(TestUtils::statusText(Status::Active), QString("1"));
    QCOMPARE(TestUtils::statusText(Status::Unknown), QString("U"));
    QCOMPARE(TestUtils::statusText(Status::Error), QString("E"));
}

void TestPortReaders::testReadMultiBitOutputKeepsEachBitsStatus()
{
    Leds f;

    f.hold({Status::Active, Status::Inactive, Status::Active, Status::Inactive});
    QCOMPARE(TestUtils::readMultiBitOutput(f.leds), 0b0101);

    // An Unknown bit where the number has a 0: the reading is not 0b0101, and it prints the U.
    f.hold({Status::Active, Status::Unknown, Status::Active, Status::Inactive});
    const Bits withUnknown = TestUtils::readMultiBitOutput(f.leds);
    QVERIFY(withUnknown != 0b0101);
    QCOMPARE(withUnknown.text(), QString("01U1"));

    // All four Unknown: the reading is not 0.
    f.hold({Status::Unknown, Status::Unknown, Status::Unknown, Status::Unknown});
    QVERIFY(TestUtils::readMultiBitOutput(f.leds) != 0);

    // The QVector overload reads the same bits.
    f.hold({Status::Error, Status::Active, Status::Inactive, Status::Active});
    const QVector<Led *> leds(std::begin(f.leds), std::end(f.leds));
    QCOMPARE(TestUtils::readMultiBitOutput(leds), TestUtils::readMultiBitOutput(f.leds));
    QCOMPARE(TestUtils::readMultiBitOutput(leds).text(), QString("101E"));
}

void TestPortReaders::testValueFailsOnANonLevel()
{
    QCOMPARE(Bits({Status::Active, Status::Active, Status::Inactive}).value(), 3);

    // value() fails the running test on a bit that is not a level, and that failure is the expected one. Had value()
    // not failed, the check after it would pass with the expectation still pending: an unexpected pass, which fails.
    const Bits bits({Status::Active, Status::Unknown});
    QEXPECT_FAIL("", "value() of bits that are not all levels", Continue);
    const int value = bits.value();
    QVERIFY(!bits.isLevel());
    QCOMPARE(value, -1);
}
