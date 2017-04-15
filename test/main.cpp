#include "testcommands.h"
#include "testelements.h"
#include "testfiles.h"
#include "testicons.h"
#include "testsimulationcontroller.h"
#include "testwaveform.h"
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
  TestWaveForm testWf;
  TestIcons testIcons;
  int status = 0;
  status |= QTest::qExec( &testElements, argc, argv );
  status |= QTest::qExec( &testSC, argc, argv );
  status |= QTest::qExec( &testFiles, argc, argv );
  status |= QTest::qExec( &testCommands, argc, argv );
  status |= QTest::qExec( &testWf, argc, argv );
  status |= QTest::qExec( &testIcons, argc, argv );
  if(status == false){
    std::cout << "All tests have passed!" << std::endl;
  }else{
    std::cout << "Some test failed!" << std::endl;
  }
  return( status );
}
