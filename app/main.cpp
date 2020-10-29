#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main( int argc, char *argv[] ) {
  QApplication a( argc, argv );
  a.setOrganizationName( "WPanda" );
  a.setApplicationName( "WiredPanda" );
  a.setApplicationVersion( APP_VERSION );

  QCommandLineParser parser;
  parser.setApplicationDescription( a.applicationName( ) );
  parser.addHelpOption( );
  parser.addVersionOption( );
  parser.addPositionalArgument( "file", QCoreApplication::translate( "main", "Circuit file to open." ) );

  QCommandLineOption arduinoFileOption( QStringList( ) << "a" << "arduino-file",
                                        QCoreApplication::translate( "main", "Export circuit to <arduino-file>" ),
                                        QCoreApplication::translate( "main", "arduino file" ) );
  parser.addOption( arduinoFileOption );

  QCommandLineOption waveformFileOption( QStringList( ) << "w" << "waveform",
                                         QCoreApplication::translate( "main",
                                                                      "Export circuit to <waveform> text file" ),
                                         QCoreApplication::translate( "main", "waveform text file" ) );
  parser.addOption( waveformFileOption );

  parser.process( a );


  MainWindow w;
  w.show( );

  QStringList args = parser.positionalArguments( );
  if( args.size( ) > 0 ) {
    w.open( args[ 0 ] );
    QString arduFile = parser.value( arduinoFileOption );
    if( !arduFile.isEmpty( ) ) {
      return( !w.ExportToArduino( arduFile ) );
    }
    QString wfFile = parser.value( waveformFileOption );
    if( !wfFile.isEmpty( ) ) {
      return( !w.ExportToWaveFormFile( wfFile ) );
    }
  }
  return( a.exec( ) );
}

/*
 * TODO: Tests for all elements
 * TODO: Create arduino version of all elements
 * TODO: Select some elements, and input wires become input buttons, output wires become leds... Connections are then
 * transfered to the box's ports.
 */