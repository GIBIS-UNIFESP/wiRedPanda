#include "testelements.h"
#include "testpriorityqueue.h"
#include <QtTest>

int main( int argc, char **argv ) {
  QApplication app( argc, argv );
  TestPriorityQueue testPriorityQueue;
  TestElements testElements;
  int status = 0;
  status |= QTest::qExec( &testPriorityQueue, argc, argv );
  status |= QTest::qExec( &testElements, argc, argv );
  /* multiple test suites can be ran like this */
  return( status );
}
