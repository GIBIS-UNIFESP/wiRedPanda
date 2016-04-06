#ifndef TESTFILES_H
#define TESTFILES_H

#include <QTest>
#include <editor.h>

class TestFiles : public QObject {
  Q_OBJECT

  MainWindow * w;
private slots:

  /* functions executed by QtTest before and after each test */
  void init( );
  void cleanup( );

  void testFiles( );
};

#endif /* TESTFILES_H */
