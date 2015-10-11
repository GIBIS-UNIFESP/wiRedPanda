#ifndef COMMANDS_H
#define COMMANDS_H

#include <QGraphicsItem>
#include <QList>
#include <QPointF>
#include <QUndoCommand>
#include <qneconnection.h>
#include <memory>

class Scene;
class Editor;
class GraphicElement;

class AddItemsCommand : public QUndoCommand {
public:
  AddItemsCommand( GraphicElement *aItem, Editor *aEditor, QUndoCommand *parent = 0 );
  AddItemsCommand( QNEConnection *aItem, Editor *aEditor, QUndoCommand *parent = 0 );
  AddItemsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor, QUndoCommand *parent = 0 );
  virtual ~AddItemsCommand( );

  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;

private:
  QList< QGraphicsItem* > items;
  QDataStream storedData;
  Editor *editor;
};

class DeleteItemsCommand : public QUndoCommand {
public:
  DeleteItemsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor, QUndoCommand *parent = 0 );
  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;

private:
  QByteArray itemData;
  QList< GraphicElement* > elements;
  QList< QNEConnection* > connections;
  Editor *editor;
};

#endif /* COMMANDS_H */
