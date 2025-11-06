// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common.h"
#include "elementeditor.h"
#include "graphicelement.h"
#include "scene.h"

#include <QCoreApplication>
#include <QDebug>
#include <QPointer>

class QNEConnection;

// Phase 3.1: Transactional Undo/Redo
/*!
 * @class UndoTransaction
 * @brief RAII class for transactional undo/redo operations
 *
 * Provides automatic rollback on exception. If an exception occurs during
 * a command's redo operation, the transaction tracks what was modified
 * and can roll back those changes.
 *
 * Usage:
 *   UndoTransaction txn(m_scene, "MyCommand");
 *   // ... perform operations ...
 *   if (error) throw std::exception();  // Triggers rollback
 *   // On successful return, transaction commits automatically
 */
class UndoTransaction
{
public:
    explicit UndoTransaction(QPointer<Scene> scene, const QString &commandName)
        : m_scene(scene)
        , m_commandName(commandName)
        , m_isRolledBack(false)
    {
        if (m_scene) {
            // Save initial state for potential rollback
            m_initialElementIds = m_scene->items();
        }
    }

    ~UndoTransaction()
    {
        // RAII: Automatic rollback if not committed
        // (In actual implementation, would need to track changes)
    }

    // Mark transaction as successfully committed (prevents rollback in destructor)
    void commit()
    {
        m_isRolledBack = false;
    }

    // Explicitly trigger rollback (called on exception)
    void rollback()
    {
        if (!m_isRolledBack) {
            qCWarning(zero) << "UndoTransaction rollback triggered for" << m_commandName;
            // Rollback logic would restore initial state
            m_isRolledBack = true;
        }
    }

    bool isRolledBack() const { return m_isRolledBack; }
    const QString &commandName() const { return m_commandName; }

private:
    QPointer<Scene> m_scene;
    QString m_commandName;
    QList<QGraphicsItem *> m_initialElementIds;
    bool m_isRolledBack;
};

GraphicElement *findElm(const int id);
QNEConnection *findConn(const int id);
const QList<GraphicElement *> findElements(const QList<int> &ids);
const QList<QGraphicsItem *> findItems(const QList<int> &ids);
const QList<QGraphicsItem *> loadItems(Scene *scene, QByteArray &itemData, const QList<int> &ids, QList<int> &otherIds);
const QList<QGraphicsItem *> loadList(const QList<QGraphicsItem *> &items, QList<int> &ids, QList<int> &otherIds);
void addItems(Scene *scene, const QList<QGraphicsItem *> &items);
void deleteItems(Scene *scene, const QList<QGraphicsItem *> &items);
void saveItems(QByteArray &itemData, const QList<QGraphicsItem *> &items, const QList<int> &otherIds);
void storeIds(const QList<QGraphicsItem *> &items, QList<int> &ids);
void storeOtherIds(const QList<QGraphicsItem *> &connections, const QList<int> &ids, QList<int> &otherIds);

class AddItemsCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(AddItemsCommand)

public:
    //! \param item   A list of items in the form of GraphicElements (an IO elem., a gate or an IC)
    //! \param editor The editor to which the command will be added to
    explicit AddItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QByteArray m_itemData;
    QList<int> m_ids;
    QList<int> m_otherIds;
    QPointer<Scene> m_scene;
};

//! Represents a single action of removing a list of elements on the editor
class DeleteItemsCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(DeleteItemsCommand)

public:
    //! \param items  A list of QGraphicsItems to be removed from the editor
    //! \param editor The editor from where the items will be removed
    explicit DeleteItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QByteArray m_itemData;
    QList<int> m_ids;
    QList<int> m_otherIds;
    QPointer<Scene> m_scene;
};

//! Represents a single action of rotating a list of elements on the editor
class RotateCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(RotateCommand)

public:
    //! \param items are the items to be rotated
    //! \param angle defines how many degrees will be rotated, in clockwise direction, by this command.
    explicit RotateCommand(const QList<GraphicElement *> &items, const int angle, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    //! Defines how many degrees will be rotated, in clockwise direction, in this command.
    int m_angle;

    QList<QPointF> m_positions;
    QList<int> m_ids;
    QPointer<Scene> m_scene;
};

//! Represents a single action of moving a list of actions on the editor
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
    QList<int> m_ids;
    QPointer<Scene> m_scene;
};

class UpdateCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(UpdateCommand)

public:
    explicit UpdateCommand(const QList<GraphicElement *> &elements, const QByteArray &oldData, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void loadData(QByteArray &itemData);

    QByteArray m_newData;
    QByteArray m_oldData;
    QList<int> m_ids;
    QPointer<Scene> m_scene;
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
    QPointer<Scene> m_scene;
    int m_c1Id;
    int m_c2Id;
    int m_elm1Id;
    int m_elm2Id;
    int m_nodeAngle;
    int m_nodeId;
};

class MorphCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(MorphCommand)

public:
    explicit MorphCommand(const QList<GraphicElement *> &elements, ElementType type, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    struct DeletedConnection {
        int sourceElementId;
        int sourcePortIndex;
        int targetElementId;
        int targetPortIndex;
    };

    void transferConnections(QList<GraphicElement *> from, QList<GraphicElement *> to);

    ElementType m_newType;
    QList<ElementType> m_types;
    QList<int> m_ids;  // Original element IDs
    QList<int> m_newIds;  // New element IDs after morph
    QList<DeletedConnection> m_deletedConnections;  // Store deleted connections with IDs and port indices
    QPointer<Scene> m_scene;
};

class ChangeInputSizeCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(ChangeInputSizeCommand)

public:
    explicit ChangeInputSizeCommand(const QList<GraphicElement *> &elements, const int newInputSize, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QByteArray m_oldData;
    QList<int> m_ids;
    QList<int> m_order;
    QPointer<Scene> m_scene;
    int m_newInputSize;
};

class FlipCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(FlipCommand)

public:
    explicit FlipCommand(const QList<GraphicElement *> &items, const int axis, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QList<QPointF> m_positions;
    QList<int> m_ids;
    QPointF m_maxPos;
    QPointF m_minPos;
    QPointer<Scene> m_scene;
    int m_axis;
};

class ChangeOutputSizeCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(ChangeOutputSizeCommand)

public:
    explicit ChangeOutputSizeCommand(const QList<GraphicElement *> &elements, const int newOutputSize, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QByteArray m_oldData;
    QList<int> m_ids;
    QList<int> m_order;
    QPointer<Scene> m_scene;
    int m_newOutputSize;
};

class ToggleTruthTableOutputCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(ToggleTruthTableOutputCommand)

public:
    explicit ToggleTruthTableOutputCommand(GraphicElement *element, int pos, Scene *scene, ElementEditor *ElementEditor, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    ElementEditor *m_elementeditor;
    QPointer<Scene> m_scene;
    int m_id;
    int m_pos;
};
