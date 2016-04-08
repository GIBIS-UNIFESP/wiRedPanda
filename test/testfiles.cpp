#include "testfiles.h"

#include "globalproperties.h"
#include <mainwindow.h>
#include <stdexcept>
void TestFiles::init( ) {
  editor = new Editor( this );
}

void TestFiles::cleanup( ) {
  delete editor;
}

void TestFiles::testFiles( ) {
  qDebug( ) << "CURRENTDIR: " << CURRENTDIR;
  QDir examplesDir( QString( "%1/../examples/" ).arg( CURRENTDIR ) );
  qDebug( ) << "Examples dir:" << examplesDir.absolutePath( );
  QStringList entries;
  entries << "*.panda";
  QFileInfoList files = examplesDir.entryInfoList( entries );
  Q_ASSERT( files.size( ) > 0 );
  int counter = 0;
  for( QFileInfo f : files ) {
    qDebug( ) << "File " << counter++ << " from " << files.size( ) << ": " << f.fileName( );
    Q_ASSERT( f.exists( ) );
    QFile pandaFile( f.absoluteFilePath( ) );
    GlobalProperties::currentFile = f.absoluteFilePath( );
    Q_ASSERT( pandaFile.exists( ) );
    Q_ASSERT( pandaFile.open( QFile::ReadOnly ) );

    QDataStream ds( &pandaFile );
    try {
      editor->load( ds );
    }
    catch( std::runtime_error e ) {
      QFAIL( QString( "Could not load the file! Error: %1" ).arg( QString::fromStdString( e.what( ) ) ).toUtf8( ) );
    }
    pandaFile.close( );
    QList< QGraphicsItem* > items = editor->getScene( )->items( );
    for( QGraphicsItem *item : items ) {
      if( item->type( ) == QNEConnection::Type ) {
        QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
        Q_ASSERT( conn != nullptr );
        Q_ASSERT( conn->port1( ) != nullptr );
        Q_ASSERT( conn->port2( ) != nullptr );
      }
    }
  }
}
