#ifndef COMMANDS_H
#define COMMANDS_H

#include <QGraphicsItem>
#include <QList>
#include <QPointF>
#include <QUndoCommand>
#include <memory>
#include <node.h>
#include <qneconnection.h>

class Scene;
class Editor;
class GraphicElement;

class AddItemsCommand : public QUndoCommand {
  enum { Id = 123 };
public:
  AddItemsCommand( GraphicElement *aItem, Editor *aEditor, QUndoCommand *parent = 0 );
  AddItemsCommand( QNEConnection *aItem, Editor *aEditor, QUndoCommand *parent = 0 );
  AddItemsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor, QUndoCommand *parent = 0 );
  virtual ~AddItemsCommand( );

  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;

private:
  QList< QGraphicsItem* > items;
  QByteArray itemData;
  Editor *editor;
  int nConnections;
  QVector< GraphicElement* > serializationOrder;
};

class DeleteItemsCommand : public QUndoCommand {
  enum { Id = 123 };
public:
  DeleteItemsCommand( const QList< QGraphicsItem* > &aItems, QUndoCommand *parent = 0 );
  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;

private:
  QByteArray itemData;
  QList< GraphicElement* > elements;
  QList< QNEConnection* > connections;
  QGraphicsScene *scene;
};

class RotateCommand : public QUndoCommand {
  enum { Id = 90 };
public:
  RotateCommand( const QList< GraphicElement* > &aItems, int angle, QUndoCommand *parent = 0 );
  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;
  bool mergeWith( const QUndoCommand *command ) Q_DECL_OVERRIDE;
  int id( ) const;
private:
  int angle;
  QList< GraphicElement* > list;
  QList< QPointF > positions;
};


class MoveCommand : public QUndoCommand {
public:
  enum { Id = 1234 };

  MoveCommand( const QList< GraphicElement* > &list, const QList< QPointF > &aOldPositions, QUndoCommand *parent = 0 );

  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;
  int id( ) const Q_DECL_OVERRIDE {
    return( Id );
  }

private:
  QList< GraphicElement* > myList;
  QList< QPointF > oldPositions;
  QList< QPointF > newPositions;

  QPointF offset;
};

class UpdateCommand : public QUndoCommand {
public:
  enum { Id = 42 };

  UpdateCommand( const QVector< GraphicElement* > &elements, QByteArray oldData, QUndoCommand *parent = 0 );

  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;
  int id( ) const Q_DECL_OVERRIDE {
    return( Id );
  }

private:
  QVector< GraphicElement* > m_elements;
  QByteArray m_oldData;
  QByteArray m_newData;

  void loadData( QByteArray itemData );
};

class SplitCommand : public QUndoCommand {
  enum { Id = 2244 };

public:

  SplitCommand( QNEConnection *conn, QPointF point, QUndoCommand *parent = 0 );
  virtual ~SplitCommand( );
  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;

private:
  QNEConnection *c1;
  QNEConnection *c2;
  QNEPort *p1, *p2;
  QNEPort *trueP1, *trueP2;

  Node *node;
};

class MorphCommand : public QUndoCommand {
public:
  enum { Id = 4567 };

  MorphCommand( const QVector< GraphicElement* > &elements, ElementType type, Editor *editor,
                QUndoCommand *parent = 0 );

  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;
  int id( ) const Q_DECL_OVERRIDE {
    return( Id );
  }

private:
  QVector< GraphicElement* > old_elements;
  QVector< GraphicElement* > new_elements;
  QGraphicsScene *scene;
  void transferConnections( QVector< GraphicElement* > from, QVector< GraphicElement* > to );
};

class ChangeInputSZCommand : public QUndoCommand {
public:
  enum { Id = 9999 };

  ChangeInputSZCommand( const QVector< GraphicElement* > &elements, int newInputSize, QUndoCommand *parent = 0 );

  void undo( ) Q_DECL_OVERRIDE;
  void redo( ) Q_DECL_OVERRIDE;
  int id( ) const Q_DECL_OVERRIDE {
    return( Id );
  }

private:
  QVector< GraphicElement* > m_elements;
  QVector< GraphicElement* > serializationOrder;
  QVector< QNEConnection * > storedConnections;
  QGraphicsScene *scene;
  QByteArray m_oldData;
  int m_newInputSize;

};

#endif /* COMMANDS_H */
