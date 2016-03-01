#include "mainwindow.h"
#include <QApplication>

int main( int argc, char *argv[] ) {
  QApplication a( argc, argv );
  a.setOrganizationName( "WPanda" );
  a.setApplicationName( "WiredPanda" );
  a.setApplicationVersion( "1.9" );
  MainWindow w;
  w.show( );
  if( argc == 2 ) {
    w.open( QString( argv[ 1 ] ) );
  }
  else if( argc == 3 ) {
    if( w.open( QString( argv[ 1 ] ) ) ) {
      return( !w.ExportToArduino( QString( argv[ 2 ] ) ) );
    }
    else {
      return( 1 );
    }
  }
  return( a.exec( ) );
}

//TODO Tests for all elements
//TODO Create arduino version of all elements
//TODO Sometimes the CTRL-Drag option is called even if the  Ctrl button is not pressed.
