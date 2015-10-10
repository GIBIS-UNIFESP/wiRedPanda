#include "box.h"
#include "commands.h"
#include "editor.h"
#include "graphicelement.h"

AddElementCommand::AddElementCommand( GraphicElement *aItem, Scene *aScene, QUndoCommand *parent ) : QUndoCommand( parent ) {
  item = aItem;
  scene = aScene;
  setText( QString( "Add %1 element" ).arg(item->objectName()) );
}

AddElementCommand::~AddElementCommand( ) {
  if( !item->scene( ) ) {
    delete item;
  }
}

void AddElementCommand::undo( ) {
  scene->removeItem( item );
}

void AddElementCommand::redo( ) {
  scene->addItem( item );
  scene->clearSelection( );
  scene->update( );
}

DeleteElementCommand::DeleteElementCommand( GraphicElement *aItem, Scene *aScene, QUndoCommand *parent ) : QUndoCommand( parent ) {
  item = aItem;
  scene = aScene;
  setText( QString( "Delete %1 element" ).arg(item->objectName()) );
}

DeleteElementCommand::~DeleteElementCommand( ) {
  if( !item->scene( ) ) {
    delete item;
  }
}

void DeleteElementCommand::undo( ) {
  scene->addItem( item );
  scene->clearSelection( );
  scene->update( );
}

void DeleteElementCommand::redo( ) {
  scene->removeItem( item );
}
