#include "testpriorityqueue.h"

#include <priorityelement.h>
#include <priorityqueue.h>

//void TestPriorityQueue::initTestCase() {

//}

//void TestPriorityQueue::cleanupTestCase() {

//}

void TestPriorityQueue::init() {
  QVector<int> values = {25,40,55,20,44,35,38,99,10,65,50};
  foreach (int i , values) {
    elements.append(new PriorityElement(i));
  }
}

void TestPriorityQueue::cleanup() {
  elements.clear();
}

void TestPriorityQueue::testSize() {
  PriorityQueue queue(elements);
  QCOMPARE(queue.size(),elements.size());
}

void TestPriorityQueue::testBuild() {
  PriorityQueue queue(elements);
  QCOMPARE(queue.toString() , QString("( 99, 65, 55, 40, 50, 35, 38, 20, 10, 44, 25 )"));
  QCOMPARE(queue.isValid(),true);
}

void TestPriorityQueue::testEmpty() {
  PriorityQueue queue(elements);
  queue.clear();
  QCOMPARE(queue.toString(), QString("(  )"));
  QCOMPARE(queue.size(), 0);
  QVERIFY(queue.pop() == nullptr);
}

void TestPriorityQueue::testPop() {
  PriorityQueue queue(elements);
  PriorityElement * elm = queue.pop();
  QCOMPARE(elm->priority(), 99);

  elm = queue.pop();
  QCOMPARE(elm->priority(), 65);
  int sz = queue.size();
  while (queue.isEmpty()) {
    queue.pop();
    queue.print();
    QCOMPARE(queue.size(),--sz);
  }
}

void TestPriorityQueue::testHeapify() {
  PriorityQueue queue(elements);
  queue.pop();
  queue.pop();

  QCOMPARE(queue.isValid(),true);
}
