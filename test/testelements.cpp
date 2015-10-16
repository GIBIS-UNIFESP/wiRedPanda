#include "testelements.h"

#include "and.h"
#include "inputvcc.h"

#include <inputgnd.h>
#include <qneconnection.h>
TestElements::TestElements( QObject *parent ) : QObject( parent ) {

}

void TestElements::testAnd( ) {
  QNEConnection * conn = new QNEConnection();
  And myAnd;
  QCOMPARE(myAnd.inputSize(), myAnd.inputs().size());
  QCOMPARE(myAnd.inputSize(), 2);
  QCOMPARE(myAnd.outputSize(), myAnd.outputs().size());
  QCOMPARE(myAnd.outputSize(), 1);
  QCOMPARE(myAnd.minInputSz(), 2);
  InputVcc vcc;
  InputGnd gnd;
  vcc.outputs().front()->connect(conn);
  myAnd.inputs().at(0)->connect(conn);
  myAnd.inputs().at(1)->connect(conn);
  myAnd.updateLogic();
  QCOMPARE((int) myAnd.outputs().front()->value(), 1);
}
