#include <QtTest>
#include "testpriorityqueue.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    TestPriorityQueue testPriorityQueue;
    // multiple test suites can be ran like this
    return QTest::qExec(&testPriorityQueue, argc, argv);
}
