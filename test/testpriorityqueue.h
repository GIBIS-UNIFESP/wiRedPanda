#ifndef TESTPRIORITYQUEUE_H
#define TESTPRIORITYQUEUE_H

#include <QTest>

class PriorityElement;

class TestPriorityQueue : public QObject {
  Q_OBJECT

private slots:
  // functions executed by QtTest before and after test suite
  //  void initTestCase();
  //  void cleanupTestCase();

  // functions executed by QtTest before and after each test
  void init();
  void cleanup();
  void testSize();
  void testBuild();

  void testEmpty();
  void testPop();
  void testHeapify();
private:
  QVector<PriorityElement *> elements;
};

#endif // TESTPRIORITYQUEUE_H
