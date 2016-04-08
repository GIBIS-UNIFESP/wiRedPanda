#include "testcommands.h"

#include <and.h>
#include <commands.h>


void TestCommands::init( ) {
  editor = new Editor( this );
}

void TestCommands::cleanup( ) {
  delete editor;
}

extern QList< QGraphicsItem* > deletedItems;

void TestCommands::testAddDeleteCommands( ) {
  //FIXME: Implementar gerenciamento de memória inteligente.
  editor->getUndoStack( )->setUndoLimit( 1 );
  QList< QGraphicsItem* > items;
  items << new And( ) << new And( ) << new And( ) << new And( );
  editor->receiveCommand( new AddItemsCommand( items, editor ) );
  QCOMPARE( editor->getScene( )->getElements( ).size( ), items.size( ) );
  QCOMPARE( deletedItems.size( ), 0 );
  editor->receiveCommand( new DeleteItemsCommand( items ) );
  QCOMPARE( deletedItems.size( ), items.size( ) );
  editor->getUndoStack( )->undo( );
  QCOMPARE( deletedItems.size( ), 0 );
  QCOMPARE( editor->getScene( )->getElements( ).size( ), 0 );
  QVERIFY( editor->getUndoStack( )->index( ) == 0 );
}
