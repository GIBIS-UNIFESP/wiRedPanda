#ifndef TESTELEMENTS_H
#define TESTELEMENTS_H

#include <QTest>
#include <array>
#include <inputswitch.h>
#include <qneconnection.h>


class Box;

class TestElements : public QObject {
  Q_OBJECT
  std::array< QNEConnection*, 5 > conn;
  std::array< InputSwitch*, 5 > sw;
  void testBoxData( Box *box );

public:
  explicit TestElements( QObject *parent = 0 );

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
  void testJKLatch( );
  void testSRFlipFlop( );
  void testTFlipFlop( );
  void testTLatch( );

  void testBox( );
  void testBoxes( );
};

#endif /* TESTELEMENTS_H */
