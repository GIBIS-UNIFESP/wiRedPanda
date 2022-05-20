/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "workspace.h"

#include <QCoreApplication>

class QNEConnection;

QList<GraphicElement *> findElements(const QVector<int> &ids);
QList<QGraphicsItem *> findItems(const QVector<int> &ids);
QList<QGraphicsItem *> loadItems(Scene *scene, QByteArray &itemData, const QVector<int> &ids, QVector<int> &otherIds);
QList<QGraphicsItem *> loadList(const QList<QGraphicsItem *> &items, QVector<int> &ids, QVector<int> &otherIds);
void addItems(Scene *scene, const QList<QGraphicsItem *> &items);
void deleteItems(Scene *scene, const QList<QGraphicsItem *> &items);
void saveItems(QByteArray &itemData, const QList<QGraphicsItem *> &items, const QVector<int> &otherIds);
void storeIds(const QList<QGraphicsItem *> &items, QVector<int> &ids);
void storeOtherIds(const QList<QGraphicsItem *> &connections, const QVector<int> &ids, QVector<int> &otherIds);

class AddItemsCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(AddItemsCommand)

public:
    //!
    //! \brief AddItemsCommand ctor taking a several QGraphicsItems as its items
    //! \param aItem   A list of items in the form of GraphicElements (an IO elem., a gate or an IC)
    //! \param aEditor The editor to which the command will be added to
    //!
    explicit AddItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent = nullptr);

    //!
    //! \brief undo reverts a change on the editor made by AddItemsCommand::redo
    //!
    void undo() override;

    //!
    //! \brief redo applies the change on the editor
    //!
    void redo() override;

private:
    QByteArray m_itemData;
    QVector<int> m_ids;
    QVector<int> m_otherIds;
    Scene *m_scene;
};

//!
//! \brief The DeleteItemsCommand class represents a single action of removing a list of elements on the editor
//!
class DeleteItemsCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(DeleteItemsCommand)

public:
    //!
    //! \brief DeleteItemsCommand
    //! \param aItems  A list of QGraphicsItems to be removed from the editor
    //! \param aEditor The editor from where the items will be removed
    //!
    explicit DeleteItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent = nullptr);

    //!
    //! \brief undo reverts a change on the editor made by DeleteItemsCommand::redo.
    //!
    void undo() override;

    //!
    //! \brief redo applies the change on the editor
    //!
    void redo() override;

private:
    QByteArray m_itemData;
    QVector<int> m_ids;
    QVector<int> m_otherIds;
    Scene *m_scene;
};

//!
//! \brief The RotateCommand class represents a single action of rotating a list of elements on the editor
//!
class RotateCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(RotateCommand)

public:
    //!
    //! \brief RotateCommand
    //! \param aItems are the items to be rotated
    //! \param angle defines how many degrees will be rotated, in clockwise direction, by this command.
    //!
    explicit RotateCommand(const QList<GraphicElement *> &items, const int angle, Scene *scene, QUndoCommand *parent = nullptr);

    //!
    //! \brief undo reverts a change on the editor made by RotateCommand::redo
    //!
    void undo() override;

    //!
    //! \brief redo applies the change on the editor
    //!
    void redo() override;

private:
    //!
    //! \brief m_angle defines how many degrees will be rotated, in clockwise direction, in this command.
    //!
    QVector<QPointF> m_positions;
    QVector<int> m_ids;
    Scene *m_scene;
    int m_angle;
};

//!
//! \brief The MoveCommand class represents a single action of moving a list of actions on the editor
//!
class MoveCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(MoveCommand)

public:
    explicit MoveCommand(const QList<GraphicElement *> &list, const QList<QPointF> &oldPositions, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QList<QPointF> m_newPositions;
    QList<QPointF> m_oldPositions;
    QPointF m_offset;
    QVector<int> m_ids;
    Scene *m_scene;
};

//!
//! \brief The UpdateCommand class
//!
class UpdateCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(UpdateCommand)

public:
    explicit UpdateCommand(const QVector<GraphicElement *> &elements, const QByteArray &oldData, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void loadData(QByteArray itemData);

    QByteArray m_newData;
    QByteArray m_oldData;
    QVector<int> ids;
    Scene *m_scene;
};

class SplitCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(SplitCommand)

public:
    explicit SplitCommand(QNEConnection *conn, QPointF mousePos, Scene *scene, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
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

public:
    explicit MorphCommand(const QVector<GraphicElement *> &elements, ElementType type, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void transferConnections(QVector<GraphicElement *> from, QVector<GraphicElement *> to);

    ElementType m_newtype;
    QVector<ElementType> m_types;
    QVector<int> m_ids;
    Scene *m_scene;
};

class ChangeInputSizeCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(ChangeInputSZCommand)

public:
    explicit ChangeInputSizeCommand(const QVector<GraphicElement *> &elements, const int newInputSize, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QByteArray m_oldData;
    QVector<int> m_elms;
    QVector<int> m_order;
    Scene *m_scene;
    int m_newInputSize;
};

class FlipCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(FlipHCommand)

public:
    explicit FlipCommand(const QList<GraphicElement *> &items, const int axis, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QPointF m_minPos, m_maxPos;
    QVector<QPointF> m_positions;
    QVector<int> m_ids;
    Scene *m_scene;
    int m_axis;
};

class ChangeOutputSizeCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(ChangeOutputSZCommand)

public:
    explicit ChangeOutputSizeCommand(const QVector<GraphicElement *> &elements, const int newOutputSize, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QByteArray m_oldData;
    QVector<int> m_elms;
    QVector<int> m_order;
    Scene *m_scene;
    int m_newOutputSize;
};

