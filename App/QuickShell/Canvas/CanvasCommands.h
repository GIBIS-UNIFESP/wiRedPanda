// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief CanvasItem-side ports of App/Scene/Commands.h's QUndoCommand subclasses.
 */

#pragma once

#include <QByteArray>
#include <QList>
#include <QPointF>
#include <QUndoCommand>
#include <QVector>

#include "App/Core/Enums.h"

class CanvasItem;
class Connection;
class GraphicElement;
class QDataStream;
class QGraphicsItem;

/**
 * \namespace CanvasCommandUtils
 * \brief CanvasItem-side port of Commands.h's CommandUtils namespace.
 *
 * \details Every function here mirrors its Commands.cpp namesake exactly, substituting
 * CanvasItem for Scene: findItems()/findElements()/findElm()/findConn() resolve through
 * CanvasItem::itemById(); addItems()/deleteItems() go through CanvasItem::addItem()/
 * removeItem() (which now do double duty as both the id-registration and the m_elements/
 * m_connections membership half, see CanvasItem.h's doc comment on addItem()); loadItems()/
 * saveItems()/loadList() are otherwise unmodified, built entirely on the already-portable
 * Serialization/SerializationContext machinery. Unlike the real findItems()/findElements(),
 * a missing id is silently skipped rather than thrown via PANDACEPTION (Scene's Sentry
 * error-reporting infrastructure isn't wired into this Quick-only tree) -- a documented,
 * deliberate deviation, not an oversight.
 */
namespace CanvasCommandUtils {
    GraphicElement *findElm(CanvasItem *canvas, int id);
    Connection *findConn(CanvasItem *canvas, int id);
    QList<GraphicElement *> findElements(CanvasItem *canvas, const QList<int> &ids);
    QList<QGraphicsItem *> findItems(CanvasItem *canvas, const QList<int> &ids);

    QList<QGraphicsItem *> loadItems(CanvasItem *canvas, QByteArray &itemData, const QList<int> &ids, QList<int> &otherIds);
    QList<QGraphicsItem *> loadList(const QList<QGraphicsItem *> &items, QList<int> &ids, QList<int> &otherIds);
    void saveItems(CanvasItem *canvas, QByteArray &itemData, const QList<QGraphicsItem *> &items, const QList<int> &otherIds);

    void addItems(CanvasItem *canvas, const QList<QGraphicsItem *> &items);
    void deleteItems(CanvasItem *canvas, const QList<QGraphicsItem *> &items);

    /// Saves and deletes connections on ports in range [fromPort, toPort).
    void drainPortConnections(GraphicElement *elm, int fromPort, int toPort,
                              bool isInput, QDataStream &stream, CanvasItem *canvas);

    void storeIds(const QList<QGraphicsItem *> &items, QList<int> &ids);
    void storeOtherIds(const QList<QGraphicsItem *> &connections, const QList<int> &ids, QList<int> &otherIds);

    /// Serializes \a items into \a stream (centroid + element data). Port of
    /// ClipboardManager::serializeItems(); used by CanvasItem::copyAction()/cutAction()/
    /// duplicateAction().
    void serializeItems(const QList<QGraphicsItem *> &items, QDataStream &stream);
}

/**
 * \class CanvasElementsCommand
 * \brief CanvasItem-side port of Commands.h's ElementsCommand.
 *
 * \details Stores the stable element ids captured at construction time (via
 * GraphicElement::id(), the real ItemWithId id CanvasItem::addItem() assigns -- see
 * CanvasItem.h's "Real id/registry layer" doc comment) and resolves them back to live
 * pointers through CanvasItem::itemById(), exactly mirroring ElementsCommand's id-based
 * resolution against Scene::itemById(). Unlike CommandUtils::findElements(), a missing id
 * is silently skipped rather than thrown -- see CanvasCommandUtils's doc comment.
 */
class CanvasElementsCommand : public QUndoCommand
{
protected:
    explicit CanvasElementsCommand(const QList<GraphicElement *> &elements, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    /// Returns the live element pointers for this command's targets, looked up by stored id.
    [[nodiscard]] QList<GraphicElement *> elements() const;

    CanvasItem *m_canvas;
    QList<int> m_ids;
};

/**
 * \class CanvasMoveCommand
 * \brief Port of Commands.h's MoveCommand: records a drag-move (or nudge) of a set of elements.
 */
class CanvasMoveCommand : public CanvasElementsCommand
{
public:
    explicit CanvasMoveCommand(const QList<GraphicElement *> &elements, const QList<QPointF> &oldPositions,
                               CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QList<QPointF> m_newPositions;
    QList<QPointF> m_oldPositions;
};

/**
 * \class CanvasRotateCommand
 * \brief Port of Commands.h's RotateCommand: rotates a list of elements by a fixed angle
 * around their shared centroid.
 */
class CanvasRotateCommand : public CanvasElementsCommand
{
public:
    explicit CanvasRotateCommand(const QList<GraphicElement *> &items, int angle, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    int m_angle;
    QList<QPointF> m_positions;
};

/**
 * \class CanvasFlipCommand
 * \brief Port of Commands.h's FlipCommand: flips a selection of elements along a horizontal
 * or vertical axis. Involution -- undo() == redo().
 */
class CanvasFlipCommand : public CanvasElementsCommand
{
public:
    /// \a axis: 0 = horizontal flip (mirror left-right), 1 = vertical flip (mirror top-bottom).
    explicit CanvasFlipCommand(const QList<GraphicElement *> &items, int axis, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QList<QPointF> m_positions;
    QPointF m_maxPos;
    QPointF m_minPos;
    int m_axis;
};

/**
 * \class CanvasAddItemsCommand
 * \brief Port of Commands.h's AddItemsCommand: adds a list of items (elements + their wires)
 * to the canvas.
 */
class CanvasAddItemsCommand : public QUndoCommand
{
public:
    explicit CanvasAddItemsCommand(const QList<QGraphicsItem *> &items, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QByteArray m_itemData;
    QList<int> m_ids;
    QList<int> m_otherIds;
    CanvasItem *m_canvas;
};

/**
 * \class CanvasDeleteItemsCommand
 * \brief Port of Commands.h's DeleteItemsCommand: removes a list of items from the canvas.
 */
class CanvasDeleteItemsCommand : public QUndoCommand
{
public:
    explicit CanvasDeleteItemsCommand(const QList<QGraphicsItem *> &items, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QByteArray m_itemData;
    QList<int> m_ids;
    QList<int> m_otherIds;
    CanvasItem *m_canvas;
};

/**
 * \class CanvasUpdateCommand
 * \brief Port of Commands.h's UpdateCommand: property changes (label, color, frequency,
 * appearance, ...) captured as a pre/post serialized-state diff.
 */
class CanvasUpdateCommand : public CanvasElementsCommand
{
public:
    /// \a oldData must be captured by the caller *before* applying the change (the
    /// constructor snapshots the elements' current, already-changed state as "new").
    explicit CanvasUpdateCommand(const QList<GraphicElement *> &elements, const QByteArray &oldData,
                                 CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    void loadData(QByteArray &itemData);

    /// One wireless-capable element's channel-relevant state, used to detect whether the
    /// old->new edit changes wireless routing (a genuine topology change) -- see
    /// Commands.h's UpdateCommand::WirelessState doc comment for the full rationale.
    struct WirelessState {
        GraphicElement *element = nullptr;
        WirelessMode mode = WirelessMode::None;
        QString label;
    };
    QVector<WirelessState> snapshotWirelessState() const;
    static bool wirelessStateDiffers(const QVector<WirelessState> &before, const QVector<WirelessState> &after);
    void refreshRuntimeState();

    QByteArray m_newData;
    QByteArray m_oldData;
    bool m_wirelessTopologyChange = false;
};

/**
 * \class CanvasChangePortSizeCommand
 * \brief Port of Commands.h's ChangePortSizeCommand: changes the number of input or output
 * ports on selected elements.
 */
class CanvasChangePortSizeCommand : public CanvasElementsCommand
{
public:
    explicit CanvasChangePortSizeCommand(const QList<GraphicElement *> &elements, int newPortSize,
                                         CanvasItem *canvas, bool isInput, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QByteArray m_oldData;
    QList<int> m_order;
    int m_newPortSize;
    bool m_isInput;
};

/**
 * \class CanvasRegisterBlobCommand
 * \brief Port of Commands.h's RegisterBlobCommand: registers/unregisters a blob in this
 * canvas's ICRegistry port. Used with beginMacro/endMacro to pair blob registration with
 * CanvasAddItemsCommand when creating new embedded ICs (see CanvasICRegistry::createEmbeddedIC()).
 */
class CanvasRegisterBlobCommand : public QUndoCommand
{
public:
    CanvasRegisterBlobCommand(const QString &blobName, const QByteArray &data, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QString m_blobName;
    QByteArray m_data;
    CanvasItem *m_canvas;
};

/**
 * \class CanvasRemoveBlobCommand
 * \brief Port of Commands.h's RemoveBlobCommand: removes/restores a blob. Inverse of
 * CanvasRegisterBlobCommand.
 */
class CanvasRemoveBlobCommand : public QUndoCommand
{
public:
    CanvasRemoveBlobCommand(const QString &blobName, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QString m_blobName;
    QByteArray m_data;
    CanvasItem *m_canvas;
};

/**
 * \class CanvasRenameBlobCommand
 * \brief Port of Commands.h's RenameBlobCommand: renames a blob in this canvas's ICRegistry port.
 */
class CanvasRenameBlobCommand : public QUndoCommand
{
public:
    CanvasRenameBlobCommand(const QString &oldName, const QString &newName, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    QString m_oldName;
    QString m_newName;
    CanvasItem *m_canvas;
};

/**
 * \class CanvasUpdateBlobCommand
 * \brief Port of Commands.h's UpdateBlobCommand: embedded IC blob changes that may alter port
 * counts. Captures connection topology before the change so connections to removed ports can
 * be restored on undo.
 */
class CanvasUpdateBlobCommand : public CanvasElementsCommand
{
public:
    struct ConnectionInfo {
        int connectionId;
        int elementId;
        int portIndex;
        bool isInput;
        int otherElementId;
        int otherPortIndex;
    };

    explicit CanvasUpdateBlobCommand(const QList<GraphicElement *> &elements, const QByteArray &oldData,
                                     const QList<ConnectionInfo> &connections, CanvasItem *canvas, QUndoCommand *parent = nullptr);

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

/**
 * \class CanvasSplitCommand
 * \brief Port of Commands.h's SplitCommand: inserts a Node junction into an existing
 * connection, splitting it into two wire segments. Picks Phase 1's deferred wire-splitting
 * gesture back up (see CanvasItem::mouseDoubleClickEvent()).
 */
class CanvasSplitCommand : public QUndoCommand
{
public:
    explicit CanvasSplitCommand(Connection *conn, QPointF mousePos, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    QPointF m_nodePos;
    CanvasItem *m_canvas;
    int m_c1Id;
    int m_c2Id;
    int m_elm1Id;
    int m_elm2Id;
    int m_nodeAngle;
    int m_nodeId;
};

/**
 * \class CanvasMorphCommand
 * \brief Port of Commands.h's MorphCommand: changes the type of selected elements in place
 * while preserving connections (used by nextElement()/prevElement()'s type-cycling shortcuts).
 */
class CanvasMorphCommand : public CanvasElementsCommand
{
public:
    explicit CanvasMorphCommand(const QList<GraphicElement *> &elements, ElementType type, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    /// Describes a connection deleted because its port was removed during a morph.
    struct DeletedConnectionInfo {
        int connectionId;
        int morphedElementId;
        int portIndex;
        bool isInput;
        int otherElementId;
        int otherPortIndex;
    };

    void transferConnections(const QList<GraphicElement *> &from, const QList<GraphicElement *> &to,
                             QList<DeletedConnectionInfo> *deleted = nullptr);
    void transferPortConnections(GraphicElement *oldElm, GraphicElement *newElm,
                                 bool isInput, QList<DeletedConnectionInfo> *deleted);
    void restoreDeletedConnections(const QList<DeletedConnectionInfo> &deleted);

    ElementType m_newType;
    QList<ElementType> m_types;
    QList<DeletedConnectionInfo> m_deletedConnections;
    QList<DeletedConnectionInfo> m_deletedConnectionsOnUndo;
};

/**
 * \class CanvasToggleTruthTableOutputCommand
 * \brief Port of Commands.h's ToggleTruthTableOutputCommand: toggles one output bit in a
 * TruthTable element. Command logic only -- its real UI trigger (a cell click in the
 * TruthTable editor dialog) is Phase 4 chrome, not exercised here.
 */
class CanvasToggleTruthTableOutputCommand : public QUndoCommand
{
public:
    explicit CanvasToggleTruthTableOutputCommand(GraphicElement *element, int pos, CanvasItem *canvas, QUndoCommand *parent = nullptr);

    void redo() override;
    void undo() override;

private:
    CanvasItem *m_canvas;
    int m_id;
    int m_pos;
};
