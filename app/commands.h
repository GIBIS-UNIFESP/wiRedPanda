/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include <QCoreApplication>
#include <QGraphicsScene>
#include <QList>
#include <QPointF>
#include <QUndoCommand>

#include "elementtype.h"
#include "globalproperties.h"

class Scene;
class Editor;
class GraphicElement;
class QGraphicsItem;
class QNEConnection;

void storeIds( const QList< QGraphicsItem* > &items, QVector< int > &ids );
void storeOtherIds( const QList< QGraphicsItem* > &connections, const QVector< int > &ids, QVector< int > &otherIds );
QList< QGraphicsItem* > loadList( const QList< QGraphicsItem* > &aItems, QVector< int > &ids, QVector< int > &otherIds );
QList< QGraphicsItem* > findItems( const QVector< int > &ids );
QList< GraphicElement* > findElements( const QVector< int > &ids );
void saveItems( QByteArray &itemData, const QList< QGraphicsItem* > &items, const QVector< int > &otherIds );
void addItems( Editor *editor, const QList< QGraphicsItem* >& items );
QList< QGraphicsItem* > loadItems( QByteArray &itemData, const QVector< int > &ids, Editor *editor, QVector< int > &otherIds );
void deleteItems( const QList< QGraphicsItem* > &items, Editor *editor );

class AddItemsCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(AddItemsCommand)

    enum { Id = 101 };

public:
    //!
    //! \brief AddItemsCommand ctor taking a GraphicElement as its item
    //! \param aItem   An item in the form of a GraphicElement (an IO elem., a gate or an IC)
    //! \param aEditor The editor to which the command will be added to
    //!
    explicit AddItemsCommand(GraphicElement *aItem, Editor *aEditor, QUndoCommand *parent = nullptr);
    //!
    //! \brief AddItemsCommand ctor taking a QNEConnection as its item
    //! \param aItem   An item in the form of a QNEConnection (a node or a connection between nodes)
    //! \param aEditor The editor to which the command will be added to
    //!
    explicit AddItemsCommand(QNEConnection *aItem, Editor *aEditor, QUndoCommand *parent = nullptr);
    //!
    //! \brief AddItemsCommand ctor taking a several QGraphicsItems as its items
    //! \param aItem   A list of items in the form of GraphicElements (an IO elem., a gate or an IC)
    //! \param aEditor The editor to which the command will be added to
    //!
    explicit AddItemsCommand(const QList<QGraphicsItem *> &aItems, Editor *aEditor, QUndoCommand *parent = nullptr);

    //!
    //! \brief undo reverts a change on the editor made by AddItemsCommand::redo
    //!
    void undo() Q_DECL_OVERRIDE;
    //!
    //! \brief redo applies the change on the editor
    //!
    void redo() Q_DECL_OVERRIDE;

private:
    QByteArray m_itemData;
    Editor * m_editor;
    QVector<int> m_ids, m_otherIds;
};

//!
//! \brief The DeleteItemsCommand class represents a single action of removing a list of elements on the editor
//!
class DeleteItemsCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(DeleteItemsCommand)
    enum { Id = 102 };

public:
    //!
    //! \brief DeleteItemsCommand
    //! \param aItems  A list of QGraphicsItems to be removed from the editor
    //! \param aEditor The editor from where the items will be removed
    //!
    explicit DeleteItemsCommand(const QList<QGraphicsItem *> &aItems, Editor *aEditor, QUndoCommand *parent = nullptr);
    //!
    //! \brief DeleteItemsCommand
    //! \param aItems  A QGraphicsItem to be removed from the editor
    //! \param aEditor The editor from where the items will be removed from
    //!
    explicit DeleteItemsCommand(QGraphicsItem *item, Editor *aEditor, QUndoCommand *parent = nullptr);

    //!
    //! \brief undo reverts a change on the editor made by DeleteItemsCommand::redo.
    //!
    void undo() Q_DECL_OVERRIDE;
    //!
    //! \brief redo applies the change on the editor
    //!
    void redo() Q_DECL_OVERRIDE;

private:
    QByteArray m_itemData;
    Editor * m_editor;
    QVector<int> m_ids, m_otherIds;
};

//!
//! \brief The RotateCommand class represents a single action of rotating a list of elements on the editor
//!
class RotateCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(RotateCommand)

    enum { Id = 103 };

public:
    //!
    //! \brief RotateCommand
    //! \param aItems are the items to be rotated
    //! \param angle defines how many degrees will be rotated, in clockwise direction, by this command.
    //!
    explicit RotateCommand(const QList<GraphicElement *> &aItems, int angle, QUndoCommand *parent = nullptr);

    //!
    //! \brief undo reverts a change on the editor made by RotateCommand::redo
    //!
    void undo() Q_DECL_OVERRIDE;
    //!
    //! \brief redo applies the change on the editor
    //!
    void redo() Q_DECL_OVERRIDE;
    // TODO: mergeWith is unused!
    bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;
    int id() const Q_DECL_OVERRIDE;

private:
    //!
    //! \brief m_angle defines how many degrees will be rotated, in clockwise direction, in this command.
    //!
    int m_angle;
    QVector<int> m_ids;
    QVector<QPointF> m_positions;
};

//!
//! \brief The MoveCommand class represents a single action of moving a list of actions on the editor
//!
class MoveCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(MoveCommand)
public:
    enum { Id = 104 };

    explicit MoveCommand(const QList<GraphicElement *> &list, const QList<QPointF> &aOldPositions, QUndoCommand *parent = nullptr);

    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
    int id() const Q_DECL_OVERRIDE
    {
        return Id;
    }

private:
    QVector<int> m_ids;
    QList<QPointF> m_oldPositions;
    QList<QPointF> m_newPositions;

    QPointF m_offset;
};

//!
//! \brief The UpdateCommand class
//!
class UpdateCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(UpdateCommand)
public:
    enum { Id = 105 };

    explicit UpdateCommand(const QVector<GraphicElement *> &elements, const QByteArray &oldData, Editor *editor, QUndoCommand *parent = nullptr);

    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
    int id() const Q_DECL_OVERRIDE
    {
        return Id;
    }

private:
    QVector<int> ids;
    QByteArray m_oldData;
    QByteArray m_newData;
    Editor *m_editor;

    void loadData(QByteArray itemData);
};

class SplitCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(SplitCommand)
    enum { Id = 106 };

public:
    explicit SplitCommand(QNEConnection *conn, QPointF point, Editor *editor, QUndoCommand *parent = nullptr);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    Editor *m_editor;
    Scene *m_scene;
    QPointF m_nodePos;
    int m_nodeAngle;
    int m_elm1_id, m_elm2_id;
    int m_c1_id, m_c2_id;
    int m_node_id;
};

class MorphCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(MorphCommand)
public:
    enum { Id = 107 };

    explicit MorphCommand(const QVector<GraphicElement *> &elements, ElementType aType, Editor *aEditor, QUndoCommand *parent = nullptr);

    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
    int id() const Q_DECL_OVERRIDE
    {
        return Id;
    }

private:
    QVector<int> m_ids;
    QVector<ElementType> m_types;
    ElementType m_newtype;
    Editor *m_editor;
    void transferConnections(QVector<GraphicElement *> from, QVector<GraphicElement *> to);
};

class ChangeInputSZCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(ChangeInputSZCommand)
public:
    enum { Id = 108 };

    explicit ChangeInputSZCommand(const QVector<GraphicElement *> &elements, int newInputSize, Editor *editor, QUndoCommand *parent = nullptr);

    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

    int id() const Q_DECL_OVERRIDE
    {
        return Id;
    }

private:
    QVector<int> m_elms;
    QVector<int> m_order;
    Editor *m_editor;
    QGraphicsScene *m_scene;
    QByteArray m_oldData;
    int m_newInputSize;
};

class FlipCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(FlipHCommand)

    enum { Id = 109 };

public:
    explicit FlipCommand(const QList<GraphicElement *> &aItems, int aAxis, QUndoCommand *parent = nullptr);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

    int id() const Q_DECL_OVERRIDE
    {
        return Id;
    }

private:
    int m_axis;
    QVector<int> m_ids;
    QVector<QPointF> m_positions;
    QPointF m_minPos, m_maxPos;
};

#endif /* COMMANDS_H */
