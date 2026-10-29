// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief All QUndoCommand subclasses and the CommandUtils helper namespace.
 */

#pragma once

#include <functional>
#include <optional>

#include <QCoreApplication>
#include <QVariant>

#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"

class Connection;

namespace CommandUtils {
    // Element/Connection lookup (scene-scoped)
    GraphicElement *findElm(Scene *scene, const int id);
    Connection *findConn(Scene *scene, const int id);
    /// The elements with \a ids, in order; none when one of them is not on the scene.
    std::optional<QList<GraphicElement *>> findElements(Scene *scene, const QList<int> &ids);
    /// The items with \a ids, in order; none when one of them is not on the scene.
    std::optional<QList<QGraphicsItem *>> findItems(Scene *scene, const QList<int> &ids);

    /**
     * \brief Gives up \a command, whose record no longer matches the scene: a broken invariant.
     * \details An assertion; a release build logs \a what, the command changes nothing, and the undo stack drops it
     * (QUndoCommand::setObsolete()), so the history stays in step with the scene.
     */
    void abandon(QUndoCommand *command, const char *what);

    // Serialization helpers
    /// Restores the items \a itemData holds under \a ids and reloads the elements wired to them; none, and the scene
    /// unchanged, when one of those elements is not on the scene or the data holds other than \a ids' items.
    std::optional<QList<QGraphicsItem *>> loadItems(Scene *scene, QByteArray &itemData, const QList<int> &ids, QList<int> &otherIds);
    const QList<QGraphicsItem *> loadList(const QList<QGraphicsItem *> &items, QList<int> &ids, QList<int> &otherIds);
    /// Saves \a others, the elements wired to \a items, then \a items, for loadItems() to restore.
    void saveItems(QByteArray &itemData, const QList<GraphicElement *> &others, const QList<QGraphicsItem *> &items);

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

    /// Returns the live element pointers for this command's targets, looked up by stored ID; none, and the command
    /// abandoned (CommandUtils::abandon()), when one of them is not on the scene.
    std::optional<QList<GraphicElement *>> elements();

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

    /// Rotates each element and turns its place about the selection's centre, snapped to the grid.
    void redo() override;

    /// Reverses the rotation and puts each element back at its exact place.
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
    /// Loads \a elements, this command's targets, from \a itemData.
    void loadData(const QList<GraphicElement *> &elements, QByteArray &itemData);

    /// One wireless-capable element's channel-relevant state (mode + label), used to detect
    /// whether the old→new edit changes wireless routing (a genuine topology change).
    struct WirelessState {
        GraphicElement *element = nullptr;
        WirelessMode mode = WirelessMode::None;
        QString label;
    };
    /// Snapshots the wireless state of every wireless-capable element of \a elements.
    static QVector<WirelessState> snapshotWirelessState(const QList<GraphicElement *> &elements);
    /// Returns true when an element's wireless mode differs between the two snapshots, or its
    /// label differs while the element is (or was) a wireless Tx/Rx — the label IS the channel.
    static bool wirelessStateDiffers(const QVector<WirelessState> &before,
                                     const QVector<WirelessState> &after);

    /// Refreshes visuals/dirty-state without a full simulation rebuild — unless the edit
    /// changed wireless routing (m_wirelessTopologyChange), which lives in the simulation
    /// topology and needs one. Shared by redo() and undo().
    void refreshRuntimeState();

    // --- Members ---
    QByteArray m_newData;
    QByteArray m_oldData;
    /// True when old→new changes wireless routing. Precomputed once in the constructor (the
    /// (m_oldData, m_newData) pair is immutable, so undo()/redo() flip between the same two
    /// states forever) — computing it around loadData() instead would miss the FIRST redo(),
    /// whose load is a no-op because the caller mutated the elements before pushing.
    bool m_wirelessTopologyChange = false;
};

/// A property of an element that a SetPropertyCommand sets, through the element's own setter.
enum class ElementProperty {
    Label,
    Color,
    Frequency,
    Delay,
    Volume,
    Audio,
    Trigger,        ///< Held as the key sequence's text.
    Wireless,       ///< A node's wireless mode; held as the WirelessMode's integer.
    Locked,         ///< An input's.
    Value,          ///< An input's output value: a rotary switch's position, any other input's level.
};

/**
 * \class SetPropertyCommand
 * \brief Undo command that sets one property of elements (ElementProperty), holding each one's value before and after.
 *
 * \details Undo and redo write those values and nothing else of the elements. A command pushed in an edit session
 * merges into the one on top of the stack from the same session when both set the same property of the same
 * elements, so a field typed into or a slider dragged is one undo step; a merge back to the values before is no step.
 */
class SetPropertyCommand : public ElementsCommand
{
public:
    /**
     * \brief Sets \a property of each of \a elements that has it to \a value, as the element takes it, and returns the
     * command that holds the change, to push; null when no value changed.
     * \details Every element that has the property stays in the command, one whose value did not change included, so
     * the commands of an edit session name the same elements and merge. \a session is the edit session, a number
     * the caller chooses: commands of one session merge; -1 is none.
     */
    [[nodiscard]] static SetPropertyCommand *apply(const QList<GraphicElement *> &elements, ElementProperty property,
                                                   const QVariant &value, Scene *scene, int session = -1);

    /// Returns whether \a element has \a property.
    static bool appliesTo(const GraphicElement &element, ElementProperty property);

    /// Sets the values after; the first redo(), as the command is pushed, finds them set.
    void redo() override;

    /// Sets the values before.
    void undo() override;

    /// Returns the merge id, or -1 for a command of no session.
    int id() const override;

    /// Takes the values after from \a other when it is of the same session and sets the same property of the same
    /// elements; with every value back to its value before, the command is obsolete.
    bool mergeWith(const QUndoCommand *other) override;

private:
    SetPropertyCommand(const QList<GraphicElement *> &elements, ElementProperty property, const QVariantList &before,
                       const QVariantList &after, Scene *scene, int session);

    /// Returns the value of \a property of \a element.
    static QVariant read(const GraphicElement &element, ElementProperty property);

    /// Sets \a property of \a element to \a value.
    static void write(GraphicElement &element, ElementProperty property, const QVariant &value);

    /// Sets each element's value before or after, then selects the elements and reports the change to the scene.
    void write(bool after);

    ElementProperty m_property;
    QVariantList m_before;    ///< One for each of elements(), in order.
    QVariantList m_after;     ///< One for each of elements(), in order.
    int m_session = -1;       ///< The edit session, or -1.
    bool m_firstRedo = true;  ///< apply() set the values after, so the first redo() leaves them.

    /// Whether the change moves a wireless channel (a node's mode, or the label of a Tx or Rx node), which the
    /// simulation reads only as it is rebuilt.
    bool m_wirelessTopologyChange = false;
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
    explicit SplitCommand(Connection *conn, QPointF mousePos, Scene *scene, QUndoCommand *parent = nullptr);

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

    /// Replaces elements with instances of the new type, which take what the types share.
    void redo() override;

    /// Restores the original elements as they were before redo() morphed them.
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

    /// Gives \a newElm what it shares with \a oldElm, the element it replaces: its place, inputs, label, trigger,
    /// flips, and the parameters both types have.
    static void copyProperties(const GraphicElement *oldElm, GraphicElement *newElm);

    /**
     * \brief Puts each element of \a to in the place of the one of \a from at its index: its wires, its ID and its
     * place in the scene; deletes the elements of \a from.
     * \param deleted If non-null, records connections that were deleted (port beyond new element's range).
     */
    void replaceElements(const QList<GraphicElement *> &from, const QList<GraphicElement *> &to,
                         QList<DeletedConnectionInfo> *deleted = nullptr);

    /**
     * \brief Transfers wires on all ports of one direction between two elements.
     * \param oldElm  Source element whose ports are being drained.
     * \param newElm  Target element that receives the migrated connections.
     * \param isInput True to migrate input ports; false for output ports.
     * \param deleted If non-null, records connections deleted because the port no longer exists.
     */
    void transferPortConnections(GraphicElement *oldElm, GraphicElement *newElm,
                                 bool isInput, QList<DeletedConnectionInfo> *deleted);

    /// Recreates each connection recorded in \a deleted, restoring its original scene ID.
    /// Shared by redo() and undo() so both directions restore a connection that couldn't
    /// survive the *other* direction's port-count change.
    void restoreDeletedConnections(const QList<DeletedConnectionInfo> &deleted);

    // --- Members ---
    ElementType m_newType;
    QList<ElementType> m_types;
    QByteArray m_originals; ///< The original elements, saved by the last redo() before it morphed them; undo() loads them.
    QList<DeletedConnectionInfo> m_deletedConnections; ///< Connections deleted during the last redo(); restored on the next undo().
    QList<DeletedConnectionInfo> m_deletedConnectionsOnUndo; ///< Connections deleted during the last undo(); restored on the next redo().
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

    /// Mirrors each element's place about the selection's axis, snapped to the grid, and toggles its mirror flag.
    void redo() override;

    /// Puts each element back at its exact place and toggles its mirror flag back.
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
 * \class RemoveBlobCommand
 * \brief Undo command that removes/restores a blob in the IC registry.
 *
 * \details Inverse of RegisterBlobCommand — used with beginMacro to pair
 * blob removal with DeleteItemsCommand when removing an embedded IC, so
 * undo restores both the IC instances AND the blob they reference.
 */
class RemoveBlobCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(RemoveBlobCommand)

public:
    RemoveBlobCommand(const QString &blobName, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QString m_blobName;
    QByteArray m_data;
    Scene *m_scene;
};

/**
 * \class RemoveICFileCommand
 * \brief Undo command for an IC file removed from the palette: its instances deleted and the file in the trash.
 *
 * \details The caller moves the file to the trash before pushing, so a file that cannot be moved is refused before
 * anything changes; the first redo deletes the instances only. Undo puts the file back from the trash, then the
 * instances, which load it. A later redo deletes the instances and moves the file to the trash again.
 */
class RemoveICFileCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(RemoveICFileCommand)

public:
    /**
     * \brief Constructs the command for an IC file already moved to the trash.
     * \param filePath Where the file was.
     * \param pathInTrash Where it is in the trash, empty when there was no file.
     * \param instances The ICs that use the file, deleted with it.
     * \param scene Target scene.
     * \param filesChanged Called after each undo and redo, when the folder's files have changed.
     * \param parent Optional parent undo command.
     */
    RemoveICFileCommand(const QString &filePath, const QString &pathInTrash, const QList<QGraphicsItem *> &instances,
                        Scene *scene, std::function<void()> filesChanged, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

    /// Returns where the file is in the trash, empty while it is not there.
    QString pathInTrash() const { return m_pathInTrash; }

private:
    /// Removes the record the freedesktop.org trash keeps of a file taken back out of it.
    static void forgetTrashRecord(const QString &pathInTrash);

    QString m_filePath;
    QString m_pathInTrash;
    std::function<void()> m_filesChanged;
    bool m_firstRedo = true;
};

/**
 * \class RenameBlobCommand
 * \brief Undo command that renames a blob in the IC registry.
 *
 * \details Pushed standalone (never sharing a transaction with an UpdateCommand)
 * so a rename is its own independent undo step — see ElementEditor's
 * lineEditBlobName editingFinished handler.
 */
class RenameBlobCommand : public QUndoCommand
{
    Q_DECLARE_TR_FUNCTIONS(RenameBlobCommand)

public:
    RenameBlobCommand(const QString &oldName, const QString &newName, Scene *scene, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QString m_oldName;
    QString m_newName;
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
    /// Loads \a elements, this command's targets, from \a itemData.
    void loadData(const QList<GraphicElement *> &elements, QByteArray &itemData);
    void reconnectConnections();

    QByteArray m_oldData;
    QByteArray m_newData;
    QByteArray m_oldBlob;
    QByteArray m_newBlob;
    QString m_blobName;
    QList<ConnectionInfo> m_connections;
};
