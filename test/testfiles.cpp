#include "testfiles.h"

#include <mainwindow.h>

void TestFiles::init( ) {
  w = new MainWindow( );
  w->undoView->hide();
}

void TestFiles::cleanup( ) {
  w->deleteLater( );
}

void TestFiles::testFiles( ) {
  QDir examplesDir( QString( "%1/../examples/" ).arg( CURRENTDIR ) );
  qDebug() << "Examples dir:" << examplesDir.absolutePath();
  QStringList entries;
  entries << "*.panda";
  QFileInfoList files = examplesDir.entryInfoList( entries );
  Q_ASSERT(files.size() > 0);
  int counter = 0;
  for( QFileInfo f : files ) {
    qDebug() << "File " << counter++ << " from " << files.size() << ": " << f.fileName();
    Q_ASSERT( f.exists() );
    Q_ASSERT( w->open( f.absoluteFilePath( ) ) );
    QList< QGraphicsItem* > items = w->editor->getScene()->items( );
    for( QGraphicsItem *item : items ) {
      if( item->type( ) == QNEConnection::Type ) {
        QNEConnection *conn = qgraphicsitem_cast< QNEConnection* >( item );
        Q_ASSERT( conn != nullptr );
        Q_ASSERT( conn->port1( ) != nullptr );
        Q_ASSERT( conn->port2( ) != nullptr );
      }
    }
  }
  w->save( "test.panda" );
  w->close( );
}
