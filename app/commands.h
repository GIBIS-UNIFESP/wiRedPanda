#ifndef COMMANDS_H
#define COMMANDS_H

#include <QGraphicsItem>
#include <QPointF>
#include <QUndoCommand>

class Scene;
class GraphicElement;

class AddElementCommand : public QUndoCommand {
public:
  AddElementCommand(GraphicElement * aItem, Scene * aScene, QUndoCommand * parent = 0);
  ~AddElementCommand( );

  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;

private:
  GraphicElement *item;
  Scene *scene;
};

class DeleteElementCommand : public QUndoCommand {
public:
  DeleteElementCommand(GraphicElement * aItem, Scene * aScene, QUndoCommand * parent = 0);
  ~DeleteElementCommand( );

  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;

private:
  GraphicElement *item;
  Scene *scene;
};


#endif /* COMMANDS_H */
