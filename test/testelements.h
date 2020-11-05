#ifndef TESTELEMENTS_H
#define TESTELEMENTS_H

#include "inputswitch.h"
#include "qneconnection.h"

#include <QTest>

class Box;

class TestElements : public QObject {
  Q_OBJECT

  QVector< QNEConnection* > conn{ 5 };
  QVector< InputSwitch* > sw{ 5 };
  void testBoxData( const Box *box );

public:
  explicit TestElements( QObject *parent = nullptr );

signals:

private slots:
  void init( );
  void cleanup( );

  void testNode( );
  void testAnd( );
  void testOr( );
  void testVCC( );
  void testGND( );

  void testMux( );
  void testDemux( );

  void testDFlipFlop( );
  void testDLatch( );
  void testJKFlipFlop( );
  void testSRFlipFlop( );
  void testTFlipFlop( );

  void testBox( );
  void testBoxes( );
};

#endif /* TESTELEMENTS_H */