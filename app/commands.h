#ifndef COMMANDS_H
#define COMMANDS_H

#include "node.h"
#include "qneconnection.h"

#include <memory>
#include <QApplication>
#include <QGraphicsItem>
#include <QList>
#include <QPointF>
#include <QUndoCommand>

class Scene;
class Editor;
class GraphicElement;

void storeIds( const QList< QGraphicsItem* > &items, QVector< int > &ids );
void storeOtherIds( const QList< QGraphicsItem* > &connections, const QVector< int > &ids, QVector< int > &otherIds );
QList< QGraphicsItem* > loadList( const QList< QGraphicsItem* > &aItems, QVector< int > &ids, QVector< int > &otherIds );
QList< QGraphicsItem* > findItems( const QVector< int > &ids );
QList< GraphicElement* > findElements( const QVector< int > &ids );
void saveitems( QByteArray &itemData, const QList< QGraphicsItem* > &items, const QVector< int > &otherIds );
void addItems( Editor *editor, QList< QGraphicsItem* > items );
QList< QGraphicsItem* > loadItems( QByteArray &itemData, const QVector< int > &ids, Editor *editor, QVector< int > &otherIds );
void deleteItems( const QList< QGraphicsItem* > &items, Editor *editor );

class AddItemsCommand : public QUndoCommand {
  Q_DECLARE_TR_FUNCTIONS( AddItemsCommand )

  enum { Id = 101 };
public:
  explicit AddItemsCommand( GraphicElement *aItem, Editor *aEditor, QUndoCommand *parent = nullptr );
  explicit AddItemsCommand( QNEConnection *aItem, Editor *aEditor, QUndoCommand *parent = nullptr );
  explicit AddItemsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor, QUndoCommand *parent = nullptr );

  virtual void undo( ) Q_DECL_OVERRIDE;
  virtual void redo( ) Q_DECL_OVERRIDE;

private:
  QByteArray itemData;
  Editor *editor;
  QVector< int > ids, otherIds;
};

class DeleteItemsCommand : public QUndoCommand {
  Q_DECLARE_TR_FUNCTIONS( DeleteItemsCommand )
  enum { Id = 102 };
public:
  explicit DeleteItemsCommand( const QList< QGraphicsItem* > &aItems, Editor *aEditor, QUndoCommand *parent = nullptr );
  explicit DeleteItemsCommand( QGraphicsItem *item, Editor *aEditor, QUndoCommand *parent = nullptr );

  virtual void undo( ) Q_DECL_OVERRIDE;
  virtual void redo( ) Q_DECL_OVERRIDE;

private:
  QByteArray itemData;
  Editor *editor;
  QVector< int > ids, otherIds;
};

class RotateCommand : public QUndoCommand {
  Q_DECLARE_TR_FUNCTIONS( RotateCommand )

  enum { Id = 103 };
public:
  explicit RotateCommand( const QList< GraphicElement* > &aItems, int angle, QUndoCommand *parent = nullptr );
  virtual void undo( ) Q_DECL_OVERRIDE;
  virtual void redo( ) Q_DECL_OVERRIDE;
  bool mergeWith( const QUndoCommand *command ) Q_DECL_OVERRIDE;
  int id( ) const Q_DECL_OVERRIDE;
private:
  int angle;
  QVector< int > ids;
  QVector< QPointF > positions;
};


class MoveCommand : public QUndoCommand {
  Q_DECLARE_TR_FUNCTIONS( MoveCommand )
public:
  enum { Id = 104 };

  explicit MoveCommand( const QList< GraphicElement* > &list,
                        const QList< QPointF > &aOldPositions,
                        QUndoCommand *parent = nullptr );

  virtual void undo( ) Q_DECL_OVERRIDE;
  virtual void redo( ) Q_DECL_OVERRIDE;
  int id( ) const Q_DECL_OVERRIDE {
    return( Id );
  }

private:
  QVector< int > ids;
  QList< QPointF > oldPositions;
  QList< QPointF > newPositions;

  QPointF offset;
};

class UpdateCommand : public QUndoCommand {
  Q_DECLARE_TR_FUNCTIONS( UpdateCommand )
public:
  enum { Id = 105 };

  explicit UpdateCommand( const QVector< GraphicElement* > &elements,
                          QByteArray oldData,
                          Editor *editor,
                          QUndoCommand *parent = nullptr );

  virtual void undo( ) Q_DECL_OVERRIDE;
  virtual void redo( ) Q_DECL_OVERRIDE;
  int id( ) const Q_DECL_OVERRIDE {
    return( Id );
  }

private:
  QVector< int > ids;
  QByteArray m_oldData;
  QByteArray m_newData;
  Editor *editor;

  void loadData( QByteArray itemData );
};

class SplitCommand : public QUndoCommand {
  Q_DECLARE_TR_FUNCTIONS( SplitCommand )
  enum { Id = 106 };

public:

  explicit SplitCommand( QNEConnection *conn, QPointF point, Editor *editor, QUndoCommand *parent = nullptr );
  virtual void undo( ) Q_DECL_OVERRIDE;
  virtual void redo( ) Q_DECL_OVERRIDE;

private:
  Editor *editor;
  Scene *scene;
  QPointF nodePos;
  int nodeAngle;
  int elm1_id, elm2_id;
  int c1_id, c2_id;
  int node_id;
};

class MorphCommand : public QUndoCommand {
  Q_DECLARE_TR_FUNCTIONS( MorphCommand )
public:
  enum { Id = 107 };

  explicit MorphCommand( const QVector< GraphicElement* > &elements,
                         ElementType aType,
                         Editor *aEditor,
                         QUndoCommand *parent =
                           nullptr );

  virtual void undo( ) Q_DECL_OVERRIDE;
  virtual void redo( ) Q_DECL_OVERRIDE;
  int id( ) const Q_DECL_OVERRIDE {
    return( Id );
  }

private:
  QVector< int > ids;
  QVector< ElementType > types;
  ElementType newtype;
  Editor *editor;
  void transferConnections( QVector< GraphicElement* > from, QVector< GraphicElement* > to );
};

class ChangeInputSZCommand : public QUndoCommand {
  Q_DECLARE_TR_FUNCTIONS( ChangeInputSZCommand )
public:
  enum { Id = 108 };

  explicit ChangeInputSZCommand( const QVector< GraphicElement* > &elements,
                                 int newInputSize,
                                 Editor *editor,
                                 QUndoCommand *parent = nullptr );

  virtual void undo( ) Q_DECL_OVERRIDE;
  virtual void redo( ) Q_DECL_OVERRIDE;

  int id( ) const Q_DECL_OVERRIDE {
    return( Id );
  }

private:
  QVector< int > elms;
  QVector< int > order;
  Editor *editor;
  QGraphicsScene *scene;
  QByteArray m_oldData;
  int m_newInputSize;

};

class FlipCommand : public QUndoCommand {
  Q_DECLARE_TR_FUNCTIONS( FlipHCommand )

  enum { Id = 109 };
public:
  explicit FlipCommand( const QList< GraphicElement* > &aItems, int aAxis, QUndoCommand *parent = nullptr );
  virtual void undo( ) Q_DECL_OVERRIDE;
  virtual void redo( ) Q_DECL_OVERRIDE;

  int id( ) const Q_DECL_OVERRIDE {
    return( Id );
  }
private:
  int axis;
  QVector< int > ids;
  QVector< QPointF > positions;
  QPointF minPos, maxPos;
};

#endif /* COMMANDS_H */
