#ifndef TESTELEMENTS_H
#define TESTELEMENTS_H

#include <QObject>
#include <QTest>

#include "inputswitch.h"
#include "qneconnection.h"

class IC;

class TestElements : public QObject
{
    Q_OBJECT

    QVector<QNEConnection *> conn{5};
    QVector<InputSwitch *> sw{5};
    void testICData(const IC *ic);

public:
    explicit TestElements(QObject *parent = nullptr);

signals:

private slots:
    void init();
    void cleanup();

    void testNode();
    void testAnd();
    void testOr();
    void testVCC();
    void testGND();

    void testMux();
    void testDemux();

    void testDFlipFlop();
    void testDLatch();
    void testJKFlipFlop();
    void testSRFlipFlop();
    void testTFlipFlop();

    void testIC();
    void testICs();
};

#endif /* TESTELEMENTS_H */
