#include "testcommands.h"
#include "testelements.h"
#include "testfiles.h"
#include "testsimulationcontroller.h"
#include <QtTest>

int main( int argc, char **argv ) {
  QApplication a( argc, argv );
  a.setOrganizationName( "WPanda" );
  a.setApplicationName( "WiredPanda" );
  a.setApplicationVersion( APP_VERSION );
  TestElements testElements;
  TestSimulationController testSC;
  TestFiles testFiles;
  TestCommands testCommands;
  int status = 0;
//  status |= QTest::qExec( &testElements, argc, argv );
//  status |= QTest::qExec( &testSC, argc, argv );
//  status |= QTest::qExec( &testFiles, argc, argv );
  status |= QTest::qExec( &testCommands, argc, argv );
  return( status );
}
