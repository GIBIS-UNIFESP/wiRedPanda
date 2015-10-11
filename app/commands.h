#ifndef COMMANDS_H
#define COMMANDS_H

#include <QGraphicsItem>
#include <QList>
#include <QPointF>
#include <QUndoCommand>

class Scene;
class Editor;
class GraphicElement;

class AddElementCommand : public QUndoCommand {
public:
  AddElementCommand( GraphicElement *aItem, Scene *aScene, QUndoCommand *parent = 0 );
  virtual ~AddElementCommand( );

  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;

private:
  GraphicElement *item;
  QDataStream storedData;
  Scene *scene;
};

class DeleteElementsCommand : public QUndoCommand {
public:
  DeleteElementsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor, QUndoCommand *parent = 0 );
  virtual ~DeleteElementsCommand();
  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;

private:
  QByteArray itemData;
  QList< QGraphicsItem* > items;
  Editor *editor;
};


#endif /* COMMANDS_H */
