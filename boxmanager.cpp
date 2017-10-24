#include "boxmanager.h"
#include "logicbox.h"

BoxManager::BoxManager( MainWindow *parent ) : parent( parent ) {

}

void BoxManager::loadFile( QString fname ) {
  if( boxes.contains( fname ) ) {

  }
  LogicBox *box = LogicBox::loadFile( fname );
  if( box ) {
    boxes[ fname ] = box;
    connect( &box->watcher, &QFileSystemWatcher::fileChanged, this, &BoxManager::updateFile );
  }
}

void BoxManager::clear( ) {
  boxes.clear( );
}

void BoxManager::updateFile( QString file ) {
  if( boxes.contains( file ) ) {
    boxes[ file ]->reload( );
  }
}
