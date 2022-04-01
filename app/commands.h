/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "elementtype.h"
#include "globalproperties.h"

#include <QCoreApplication>
#include <QGraphicsScene>
#include <QList>
#include <QPointF>
#include <QUndoCommand>

class Scene;
class Editor;
class GraphicElement;
class QGraphicsItem;
class QNEConnection;

QList<GraphicElement *> findElements(const QVector<int> &ids);
QList<QGraphicsItem *> findItems(const QVector<int> &ids);
QList<QGraphicsItem *> loadItems(QByteArray &itemData, const QVector<int> &ids, Editor *editor, QVector<int> &otherIds);
QList<QGraphicsItem *> loadList(const QList<QGraphicsItem *> &aItems, QVector<int> &ids, QVector<int> &otherIds);
void addItems(Editor *editor, const QList<QGraphicsItem *> &items);
void deleteItems(const QList<QGraphicsItem *> &items, Editor *editor);
void saveItems(QByteArray &itemData, const QList<QGraphicsItem *> &items, const QVector<int> &otherIds);
void storeIds(const QList<QGraphicsItem *> &items, QVector<int> &ids);
void storeOtherIds(const QList<QGraphicsItem *> &connections, const QVector<int> &ids, QVector<int> &otherIds);

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
    void undo() override;

    //!
    //! \brief redo applies the change on the editor
    //!
    void redo() override;

private:
    Editor *m_editor;
    QByteArray m_itemData;
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
    void undo() override;

    //!
    //! \brief redo applies the change on the editor
    //!
    void redo() override;

private:
    Editor *m_editor;
    QByteArray m_itemData;
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
    explicit RotateCommand(const QList<GraphicElement *> &aItems, int angle, Editor *aEditor, QUndoCommand *parent = nullptr);

    //!
    //! \brief undo reverts a change on the editor made by RotateCommand::redo
    //!
    void undo() override;

    //!
    //! \brief redo applies the change on the editor
    //!
    void redo() override;

    int id() const override { return Id; }

private:
    //!
    //! \brief m_angle defines how many degrees will be rotated, in clockwise direction, in this command.
    //!
    int m_angle;
    Editor *m_editor;
    QVector<QPointF> m_positions;
    QVector<int> m_ids;
};

//!
//! \brief The MoveCommand class represents a single action of moving a list of actions on the editor
//!
class MoveCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(MoveCommand)

    enum { Id = 104 };

public:
    explicit MoveCommand(const QList<GraphicElement *> &list, const QList<QPointF> &aOldPositions, Editor *aEditor, QUndoCommand *parent = nullptr);

    int id() const override { return Id; }
    void redo() override;
    void undo() override;

private:
    Editor *m_editor;
    QList<QPointF> m_newPositions;
    QList<QPointF> m_oldPositions;
    QPointF m_offset;
    QVector<int> m_ids;
};

//!
//! \brief The UpdateCommand class
//!
class UpdateCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(UpdateCommand)

    enum { Id = 105 };

public:
    explicit UpdateCommand(const QVector<GraphicElement *> &elements, const QByteArray &oldData, Editor *editor, QUndoCommand *parent = nullptr);

    int id() const override { return Id; }
    void redo() override;
    void undo() override;

private:
    void loadData(QByteArray itemData);

    Editor *m_editor;
    QByteArray m_newData;
    QByteArray m_oldData;
    QVector<int> ids;
};

class SplitCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(SplitCommand)

    enum { Id = 106 };

public:
    explicit SplitCommand(QNEConnection *conn, QPointF point, Editor *editor, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Editor *m_editor;
    QPointF m_nodePos;
    Scene *m_scene;
    int m_c1_id, m_c2_id;
    int m_elm1_id, m_elm2_id;
    int m_nodeAngle;
    int m_node_id;
};

class MorphCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(MorphCommand)

    enum { Id = 107 };

public:
    explicit MorphCommand(const QVector<GraphicElement *> &elements, ElementType aType, Editor *aEditor, QUndoCommand *parent = nullptr);

    int id() const override { return Id; }
    void redo() override;
    void undo() override;

private:
    void transferConnections(QVector<GraphicElement *> from, QVector<GraphicElement *> to);

    Editor *m_editor;
    ElementType m_newtype;
    QVector<ElementType> m_types;
    QVector<int> m_ids;
};

class ChangeInputSZCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(ChangeInputSZCommand)

    enum { Id = 108 };

public:
    explicit ChangeInputSZCommand(const QVector<GraphicElement *> &elements, int newInputSize, Editor *editor, QUndoCommand *parent = nullptr);

    int id() const override { return Id; }
    void redo() override;
    void undo() override;

private:
    Editor *m_editor;
    QByteArray m_oldData;
    QGraphicsScene *m_scene;
    QVector<int> m_elms;
    QVector<int> m_order;
    int m_newInputSize;
};

class FlipCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(FlipHCommand)

    enum { Id = 109 };

public:
    explicit FlipCommand(const QList<GraphicElement *> &aItems, int aAxis, Editor *aEditor, QUndoCommand *parent = nullptr);

    int id() const override { return Id; }
    void redo() override;
    void undo() override;

private:
    Editor *m_editor;
    QPointF m_minPos, m_maxPos;
    QVector<QPointF> m_positions;
    QVector<int> m_ids;
    int m_axis;
};

class ChangeOutputSZCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(ChangeOutputSZCommand)

    enum { Id = 110 };

public:
    explicit ChangeOutputSZCommand(const QVector<GraphicElement *> &elements, int newOutputSize, Editor *editor, QUndoCommand *parent = nullptr);

    int id() const override { return Id; }
    void redo() override;
    void undo() override;

private:
    Editor *m_editor;
    QByteArray m_oldData;
    QGraphicsScene *m_scene;
    QVector<int> m_elms;
    QVector<int> m_order;
    int m_newOutputSize;
};

