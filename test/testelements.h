#ifndef TESTELEMENTS_H
#define TESTELEMENTS_H

#include <QTest>
#include <inputswitch.h>
#include <qneconnection.h>

class TestElements : public QObject {
  Q_OBJECT
  QNEConnection *conn[4];
  InputSwitch *sw[4];
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
};

#endif /* TESTELEMENTS_H */
