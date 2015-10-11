#include "box.h"
#include "commands.h"
#include "editor.h"
#include "graphicelement.h"
#include "serializationfunctions.h"
#include "scene.h"

//TODO Criar comandos usando mesma técnica do copy n' paste.

AddElementCommand::AddElementCommand( GraphicElement *aItem, Scene *aScene, QUndoCommand *parent ) : QUndoCommand( parent ) {
  item = aItem;
  scene = aScene;
  setText( QString( "Add %1 element" ).arg(item->objectName()) );
}

AddElementCommand::~AddElementCommand( ) {
  if( !scene->items().contains(item) ) {
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

DeleteElementsCommand::DeleteElementsCommand(const QList<QGraphicsItem *> & aItems, Scene *aScene, QUndoCommand *parent ) : QUndoCommand( parent ) {
  scene = aScene;
  items = aItems;
  setText( QString( "Delete %1 elements" ).arg(items.size()) );
}

void DeleteElementsCommand::undo( ) {

}

void DeleteElementsCommand::redo( ) {
  SerializationFunctions::serialize(items, storedData);
  foreach( QGraphicsItem * item, items ) {
    if( !scene->items( ).contains( item ) ) {
      continue;
    }
    scene->removeItem( item );
    delete item;
  }
  items.clear();
}
