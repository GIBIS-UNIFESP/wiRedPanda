// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief All QUndoCommand subclasses and the CommandUtils helper namespace.
 */

#pragma once

#include <QCoreApplication>

#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"

class QNEConnection;

namespace CommandUtils {
    // Element/Connection lookup (scene-scoped)
    GraphicElement *findElm(Scene *scene, const int id);
    QNEConnection *findConn(Scene *scene, const int id);
    const QList<GraphicElement *> findElements(Scene *scene, const QList<int> &ids);
    const QList<QGraphicsItem *> findItems(Scene *scene, const QList<int> &ids);

    // Serialization helpers
    const QList<QGraphicsItem *> loadItems(Scene *scene, QByteArray &itemData, const QList<int> &ids, QList<int> &otherIds);
    const QList<QGraphicsItem *> loadList(const QList<QGraphicsItem *> &items, QList<int> &ids, QList<int> &otherIds);
    void saveItems(Scene *scene, QByteArray &itemData, const QList<QGraphicsItem *> &items, const QList<int> &otherIds);

    // Scene operations
    void addItems(Scene *scene, const QList<QGraphicsItem *> &items);
    void deleteItems(Scene *scene, const QList<QGraphicsItem *> &items);

    /// Saves and deletes connections on ports in range [fromPort, toPort).
    void drainPortConnections(GraphicElement *elm, int fromPort, int toPort,
                              bool isInput, QDataStream &stream, Scene *scene);

    // ID management
    void storeIds(const QList<QGraphicsItem *> &items, QList<int> &ids);
    void storeOtherIds(const QList<QGraphicsItem *> &connections, const QList<int> &ids, QList<int> &otherIds);
}

/**
 * \class ElementsCommand
 * \brief Intermediate base for commands that operate on a fixed list of GraphicElements.
 *
 * \details Stores the scene pointer and the stable element IDs captured at construction time,
 * and exposes an elements() accessor that resolves them back to live pointers at undo/redo time.
 * Eliminates the repeated m_ids + m_scene + findElements() boilerplate across command subclasses.
 */
class ElementsCommand : public QUndoCommand
{
protected:
    explicit ElementsCommand(const QList<GraphicElement *> &elements, Scene *scene, QUndoCommand *parent = nullptr);

    /// Returns the live element pointers for this command's targets, looked up by stored ID.
    QList<GraphicElement *> elements() const;

    Scene *m_scene;
    QList<int> m_ids;
};

/**
 * \class AddItemsCommand
 * \brief Undo command that adds a list of graphic elements to the scene.
 */
class AddItemsCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(AddItemsCommand)

public:
    /**
     * \brief Constructs the command for adding \a items.
     * \param items List of GraphicElements (IO elements, gates, ICs) to add.
     * \param scene Target scene.
     * \param parent Optional parent undo command.
     */
    explicit AddItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent = nullptr);

    /// Adds the items to the scene.
    void redo() override;

    /// Removes the items from the scene.
    void undo() override;

private:
    // --- Members ---
    QByteArray m_itemData;
    QList<int> m_ids;
    QList<int> m_otherIds;
    Scene *m_scene;
};

/**
 * \class DeleteItemsCommand
 * \brief Undo command that removes a list of items from the scene.
 */
class DeleteItemsCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(DeleteItemsCommand)

public:
    /**
     * \brief Constructs the command for deleting \a items.
     * \param items List of QGraphicsItems to remove.
     * \param scene Target scene.
     * \param parent Optional parent undo command.
     */
    explicit DeleteItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent = nullptr);

    /// Removes the items from the scene.
    void redo() override;

    /// Restores the items to the scene.
    void undo() override;

private:
    // --- Members ---
    QByteArray m_itemData;
    QList<int> m_ids;
    QList<int> m_otherIds;
    Scene *m_scene;
};

/**
 * \class RotateCommand
 * \brief Undo command that rotates a list of elements by a fixed angle.
 */
class RotateCommand : public ElementsCommand
{
    Q_DECLARE_TR_FUNCTIONS(RotateCommand)

public:
    /**
     * \brief Constructs the command.
     * \param items  Elements to rotate.
     * \param angle  Clockwise rotation in degrees (e.g. 90 or -90).
     * \param scene  Target scene.
     * \param parent Optional parent undo command.
     */
    explicit RotateCommand(const QList<GraphicElement *> &items, const int angle, Scene *scene, QUndoCommand *parent = nullptr);

    /// Applies the rotation.
    void redo() override;

    /// Reverses the rotation.
    void undo() override;

private:
    /// Defines how many degrees will be rotated, in clockwise direction, in this command.
    int m_angle;

    QList<QPointF> m_positions;
};

/**
 * \class MoveCommand
 * \brief Undo command that records a drag-move of a set of elements.
 */
class MoveCommand : public ElementsCommand
{
    Q_DECLARE_TR_FUNCTIONS(MoveCommand)

public:
    /**
     * \brief Constructs the command capturing old and new positions.
     * \param list         Elements that were moved.
     * \param oldPositions Positions before the move.
     * \param scene        Target scene.
     * \param parent       Optional parent undo command.
     */
    explicit MoveCommand(const QList<GraphicElement *> &list, const QList<QPointF> &oldPositions, Scene *scene, QUndoCommand *parent = nullptr);

    /// Moves elements to their new positions.
    void redo() override;

    /// Restores elements to their old positions.
    void undo() override;

private:
    // --- Members ---
    QList<QPointF> m_newPositions;
    QList<QPointF> m_oldPositions;
};

/**
 * \class UpdateCommand
 * \brief Undo command for property changes (label, color, frequency, appearance, etc.).
 *
 * \details Captures the pre-change state in \a oldData; the post-change state
 * is serialized from the elements at construction time.
 */
class UpdateCommand : public ElementsCommand
{
    Q_DECLARE_TR_FUNCTIONS(UpdateCommand)

public:
    /**
     * \brief Constructs the command.
     * \param elements Elements whose properties changed.
     * \param oldData  Serialized state before the change.
     * \param scene    Target scene.
     * \param parent   Optional parent undo command.
     */
    explicit UpdateCommand(const QList<GraphicElement *> &elements, const QByteArray &oldData, Scene *scene, QUndoCommand *parent = nullptr);

    /// Applies the new property values.
    void redo() override;

    /// Restores the old property values.
    void undo() override;

private:
    // --- Helpers ---
    void loadData(QByteArray &itemData);

    // --- Members ---
    QByteArray m_newData;
    QByteArray m_oldData;
};

/**
 * \class SplitCommand
 * \brief Undo command that inserts a Node junction into an existing connection.
 */
class SplitCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(SplitCommand)

public:
    /**
     * \brief Constructs the split command.
     * \param conn     Connection to split.
     * \param mousePos Scene position where the node will be inserted.
     * \param scene    Target scene.
     * \param parent   Optional parent undo command.
     */
    explicit SplitCommand(QNEConnection *conn, QPointF mousePos, Scene *scene, QUndoCommand *parent = nullptr);

    /// Removes the node and restores the original connection.
    void undo() override;

    /// Inserts the node and splits the connection.
    void redo() override;

private:
    // --- Members ---
    QPointF m_nodePos;
    Scene *m_scene;
    int m_c1Id;
    int m_c2Id;
    int m_elm1Id;
    int m_elm2Id;
    int m_nodeAngle;
    int m_nodeId;
};

/**
 * \class MorphCommand
 * \brief Undo command that changes the type of selected elements while preserving connections.
 */
class MorphCommand : public ElementsCommand
{
    Q_DECLARE_TR_FUNCTIONS(MorphCommand)

public:
    /**
     * \brief Constructs the morph command.
     * \param elements Elements to morph.
     * \param type     Target element type.
     * \param scene    Target scene.
     * \param parent   Optional parent undo command.
     */
    explicit MorphCommand(const QList<GraphicElement *> &elements, ElementType type, Scene *scene, QUndoCommand *parent = nullptr);

    /// Replaces elements with instances of the new type.
    void redo() override;

    /// Restores the original element types.
    void undo() override;

private:
    // --- Helpers ---

    /// Describes a connection that was deleted because its port was removed during a morph.
    struct DeletedConnectionInfo {
        int connectionId;     ///< Original scene ID of the connection; restored via updateItemId() on undo so that any undo command holding this ID (e.g. DeleteItemsCommand) can still find it.
        int morphedElementId; ///< ID of the morphed element (stable across undo/redo via updateItemId).
        int portIndex;        ///< Port index on the morphed element.
        bool isInput;         ///< True if the port was an input port.
        int otherElementId;   ///< ID of the element on the other end.
        int otherPortIndex;   ///< Port index on the other element.
    };

    /**
     * \brief Transfers wires from \a from to \a to, swapping element references.
     * \param deleted If non-null, records connections that were deleted (port beyond new element's range).
     */
    void transferConnections(const QList<GraphicElement *> &from, const QList<GraphicElement *> &to,
                             QList<DeletedConnectionInfo> *deleted = nullptr);

    // --- Members ---
    ElementType m_newType;
    QList<ElementType> m_types;
    QList<DeletedConnectionInfo> m_deletedConnections; ///< Connections deleted during the last redo(); restored on undo().
};

/**
 * \class ChangePortSizeCommand
 * \brief Undo command that changes the number of input or output ports on selected elements.
 */
class ChangePortSizeCommand : public ElementsCommand
{
    Q_DECLARE_TR_FUNCTIONS(ChangePortSizeCommand)

public:
    /**
     * \brief Constructs the command.
     * \param elements    Elements whose port count will change.
     * \param newPortSize Desired number of ports.
     * \param scene       Target scene.
     * \param isInput     If \c true, adjusts input ports; otherwise adjusts output ports.
     * \param parent      Optional parent undo command.
     */
    explicit ChangePortSizeCommand(const QList<GraphicElement *> &elements, const int newPortSize, Scene *scene, const bool isInput, QUndoCommand *parent = nullptr);

    /// Applies the new port count.
    void redo() override;

    /// Restores the old port count and connections.
    void undo() override;

private:
    // --- Members ---
    QByteArray m_oldData;
    QList<int> m_order;
    int m_newPortSize;
    bool m_isInput;
};

/**
 * \class FlipCommand
 * \brief Undo command that flips a selection of elements along a horizontal or vertical axis.
 */
class FlipCommand : public ElementsCommand
{
    Q_DECLARE_TR_FUNCTIONS(FlipCommand)

public:
    /**
     * \brief Constructs the flip command.
     * \param items  Elements to flip.
     * \param axis   0 = horizontal flip (mirror left-right), 1 = vertical flip (mirror top-bottom).
     * \param scene  Target scene.
     * \param parent Optional parent undo command.
     */
    explicit FlipCommand(const QList<GraphicElement *> &items, const int axis, Scene *scene, QUndoCommand *parent = nullptr);

    /// Applies the flip transformation.
    void redo() override;

    /// Reverses the flip transformation (involution: flip twice = identity).
    void undo() override;

private:
    // --- Members ---
    QList<QPointF> m_positions;
    QPointF m_maxPos;
    QPointF m_minPos;
    int m_axis;
};

/**
 * \class ToggleTruthTableOutputCommand
 * \brief Undo command that toggles one output bit in a TruthTable element.
 */
class ToggleTruthTableOutputCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(ToggleTruthTableOutputCommand)

public:
    /**
     * \brief Constructs the toggle command.
     * \param element TruthTable element to modify.
     * \param pos     Flat index of the bit to toggle in the proposition array.
     * \param scene   Target scene.
     * \param parent  Optional parent undo command.
     */
    explicit ToggleTruthTableOutputCommand(GraphicElement *element, int pos, Scene *scene, QUndoCommand *parent = nullptr);

    /// Toggles the bit at \a pos.
    void redo() override;

    /// Toggles the bit again (involution).
    void undo() override;

private:
    Scene *m_scene;
    int m_id;
    int m_pos;
};

/**
 * \class RegisterBlobCommand
 * \brief Undo command that registers/unregisters a blob in the IC registry.
 *
 * \details Used with beginMacro/endMacro to pair blob registration with
 * AddItemsCommand when creating new embedded ICs.
 */
class RegisterBlobCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(RegisterBlobCommand)

public:
    RegisterBlobCommand(const QString &blobName, const QByteArray &data, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QString m_blobName;
    QByteArray m_data;
    Scene *m_scene;
};

/**
 * \class UpdateBlobCommand
 * \brief Undo command for embedded IC blob changes that may alter port counts.
 *
 * \details Captures connection topology before the change so connections to
 * removed ports can be restored on undo. Swaps blob registry entries on undo/redo.
 */
class UpdateBlobCommand : public ElementsCommand
{
    Q_DECLARE_TR_FUNCTIONS(UpdateBlobCommand)

public:
    struct ConnectionInfo {
        int connectionId;
        int elementId;
        int portIndex;
        bool isInput;
        int otherElementId;
        int otherPortIndex;
    };

    explicit UpdateBlobCommand(const QList<GraphicElement *> &elements, const QByteArray &oldData,
                               const QList<ConnectionInfo> &connections, Scene *scene, QUndoCommand *parent = nullptr);

    /// Captures connection topology for all target elements before a blob operation.
    static QList<ConnectionInfo> captureConnections(const QList<GraphicElement *> &targets);

    /// Sets the old blob data for registry swap on undo.
    void setOldBlob(const QByteArray &blob) { m_oldBlob = blob; }

    /// Overrides the blob name (needed when elements are already file-backed at construction time).
    void setBlobName(const QString &name) { m_blobName = name; }

    void redo() override;
    void undo() override;

private:
    void loadData(QByteArray &itemData);
    void reconnectConnections();

    QByteArray m_oldData;
    QByteArray m_newData;
    QByteArray m_oldBlob;
    QByteArray m_newBlob;
    QString m_blobName;
    QList<ConnectionInfo> m_connections;
};

