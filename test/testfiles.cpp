#include "testfiles.h"

#include <mainwindow.h>

void TestFiles::init( ) {
  w = new MainWindow( );
}

void TestFiles::cleanup( ) {
  w->deleteLater( );
}

void TestFiles::testFiles( ) {
  QDir examplesDir( QString( "%1/../examples/" ).arg( CURRENTDIR ) );
  QStringList entries;
  entries << "*.panda";
  QFileInfoList files = examplesDir.entryInfoList( entries );
  for( QFileInfo f : files ) {
    Q_ASSERT( w->open( f.absoluteFilePath( ) ) );
  }
  w->save( "test.panda" );
  w->close( );
}
