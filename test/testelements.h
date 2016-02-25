#ifndef TESTELEMENTS_H
#define TESTELEMENTS_H

#include <QTest>
#include <array>
#include <inputswitch.h>
#include <qneconnection.h>

class TestElements : public QObject {
  Q_OBJECT
  std::array< QNEConnection*, 5 > conn;
  std::array< InputSwitch*, 4 > sw;
public:
  explicit TestElements( QObject *parent = 0 );

signals:

private slots:
  void init( );
  void cleanup( );

  void testAnd( );
  void testOr( );
  void testVCC( );
  void testGND( );

  void testDFlipFlop( );
  void testDLatch( );
  void testJKFlipFlop( );
  void testJKLatch( );

};

#endif /* TESTELEMENTS_H */
