#include "box.h"
#include "commands.h"
#include "editor.h"
#include "graphicelement.h"

AddCommand::AddCommand( GraphicElement *aItem, Scene *aScene, QUndoCommand *parent ) : QUndoCommand( parent ) {
  item = aItem;
  scene = aScene;
/*  scene->addItem( item ); */
  setText( QString( "Add %1 element" ).arg(item->objectName()) );
}

AddCommand::~AddCommand( ) {
  if( !item->scene( ) ) {
    delete item;
  }
}

void AddCommand::undo( ) {
  scene->removeItem( item );
}

void AddCommand::redo( ) {
  scene->addItem( item );
  scene->clearSelection( );
  scene->update( );
}
