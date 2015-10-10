#ifndef COMMANDS_H
#define COMMANDS_H

#include <QGraphicsItem>
#include <QPointF>
#include <QUndoCommand>

class Scene;
class GraphicElement;

class AddCommand : public QUndoCommand {
public:
  AddCommand(GraphicElement * aItem, Scene * aScene, QUndoCommand * parent = 0);
  ~AddCommand( );

  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;

private:
  GraphicElement *item;
  Scene *scene;
};

#endif /* COMMANDS_H */
