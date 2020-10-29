#ifndef TESTCOMMANDS_H
#define TESTCOMMANDS_H

#include "editor.h"

#include <QTest>

class TestCommands : public QObject {
  Q_OBJECT
  Editor * editor;

private slots:
  void init( );
  void cleanup( );

  void testAddDeleteCommands( );

};

#endif /* TESTCOMMANDS_H */