#ifndef TESTSIMULATIONCONTROLLER_H
#define TESTSIMULATIONCONTROLLER_H

#include "graphicelement.h"
#include "editor.h"

#include <QTest>

class TestSimulationController : public QObject {
  Q_OBJECT

  QVector< GraphicElement* > elms;
  Editor * editor;
private slots:

  /* functions executed by QtTest before and after each test */
  void init( );
  void cleanup( );

  void testCase1( );
};

#endif /* TESTSIMULATIONCONTROLLER_H */
