#ifndef TESTLOGICELEMENTS_H
#define TESTLOGICELEMENTS_H

#include <QObject>
#include <QTest>
#include <logicelement.h>

class TestLogicElements : public QObject {
  Q_OBJECT

  std::array< LogicInput*, 5 > sw;
public:
  explicit TestLogicElements( QObject *parent = nullptr );

signals:

private slots:

  void init( );

  void cleanup( );

  void testLogicNode( );

  void testLogicAnd( );

  void testLogicOr( );

  void testLogicInput( );

  void testLogicMux( );

  void testLogicDemux( );

  void testLogicDFlipFlop( );

  void testLogicDLatch( );

  void testLogicJKFlipFlop( );

  void testLogicSRFlipFlop( );

  void testLogicTFlipFlop( );
};

#endif // TESTLOGICELEMENTS_H
