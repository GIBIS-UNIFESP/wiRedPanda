#ifndef COMMANDS_H
#define COMMANDS_H

#include <QGraphicsItem>
#include <QPointF>
#include <QUndoCommand>
#include <QList>

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
  QDataStream storedData;
  Scene *scene;
};

class DeleteElementsCommand : public QUndoCommand {
public:
  DeleteElementsCommand(const QList<QGraphicsItem *> & aItems, Scene * aScene, QUndoCommand * parent = 0);

  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;

private:
  QDataStream storedData;
  QList<QGraphicsItem *> items;
  Scene *scene;
};


#endif /* COMMANDS_H */
