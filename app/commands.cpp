// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commands.h"

#include "common.h"
#include "editor.h"
#include "elementfactory.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "serializationfunctions.h"
#include "simulationcontroller.h"

#include <QDrag>
#include <QGraphicsItem>
#include <QIODevice>
#include <cmath>
#include <stdexcept>

void storeIds(const QList<QGraphicsItem *> &items, QVector<int> &ids)
{
    ids.reserve(items.size());
    for (auto *item : items) {
        auto *iwid = dynamic_cast<ItemWithId *>(item);
        if (iwid) {
            ids.append(iwid->id());
        }
    }
}

void storeOtherIds(const QList<QGraphicsItem *> &connections, const QVector<int> &ids, QVector<int> &otherIds)
{
    for (auto *item : connections) {
        auto *conn = qgraphicsitem_cast<QNEConnection *>(item);
        if ((item->type() == QNEConnection::Type) && conn) {
            auto *p1 = conn->start();
            if (p1 && p1->graphicElement() && !ids.contains(p1->graphicElement()->id())) {
                otherIds.append(p1->graphicElement()->id());
            }
            auto *p2 = conn->end();
            if (p2 && p2->graphicElement() && !ids.contains(p2->graphicElement()->id())) {
                otherIds.append(p2->graphicElement()->id());
            }
        }
    }
}

QList<QGraphicsItem *> loadList(const QList<QGraphicsItem *> &aItems, QVector<int> &ids, QVector<int> &otherIds)
{
    QList<QGraphicsItem *> elements;
    /* Stores selected graphicElements */
    for (auto *item : aItems) {
        if (item->type() == GraphicElement::Type) {
            if (!elements.contains(item)) {
                elements.append(item);
            }
        }
    }
    QList<QGraphicsItem *> connections;
    /* Stores all the wires linked to these elements */
    for (auto *item : elements) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm) {
            const auto inputs = elm->inputs();
            for (auto *port : inputs) {
                for (auto *conn : port->connections()) {
                    if (!connections.contains(conn)) {
                        connections.append(conn);
                    }
                }
            }
            const auto outputs = elm->outputs();
            for (auto *port : outputs) {
                for (auto *conn : port->connections()) {
                    if (!connections.contains(conn)) {
                        connections.append(conn);
                    }
                }
            }
        }
    }
    /* Stores the other wires selected */
    for (auto *item : aItems) {
        if (item->type() == QNEConnection::Type) {
            if (!connections.contains(item)) {
                connections.append(item);
            }
        }
    }
    /* Stores the ids of all elements listed in items; */
    storeIds(elements + connections, ids);
    /* Stores all the elements linked to each connection that will not be deleted. */
    storeOtherIds(connections, ids, otherIds);
    return elements + connections;
}

QList<QGraphicsItem *> findItems(const QVector<int> &ids)
{
    QList<QGraphicsItem *> items;
    for (int id : ids) {
        auto *item = dynamic_cast<QGraphicsItem *>(ElementFactory::getItemById(id));
        if (item) {
            items.append(item);
        }
    }
    if (items.size() != ids.size()) {
        throw std::runtime_error(ERRORMSG(QObject::tr("One or more items was not found on the scene.").toStdString()));
    }
    return items;
}

QList<GraphicElement *> findElements(const QVector<int> &ids)
{
    QList<GraphicElement *> items;
    for (int id : ids) {
        auto *item = dynamic_cast<GraphicElement *>(ElementFactory::getItemById(id));
        if (item) {
            items.append(item);
        }
    }
    if (items.size() != ids.size()) {
        throw std::runtime_error(ERRORMSG(QObject::tr("One or more elements was not found on the scene.").toStdString()));
    }
    return items;
}

void saveItems(QByteArray &itemData, const QList<QGraphicsItem *> &items, const QVector<int> &otherIds)
{
    itemData.clear();
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    QList<GraphicElement *> others = findElements(otherIds);
    for (auto *elm : qAsConst(others)) {
        elm->save(dataStream);
    }
    SerializationFunctions::serialize(items, dataStream);
}

void addItems(Editor *editor, const QList<QGraphicsItem *> &items)
{
    editor->getScene()->clearSelection();
    for (auto *item : items) {
        if (item->scene() != editor->getScene()) {
            editor->getScene()->addItem(item);
        }
        if (item->type() == GraphicElement::Type) {
            item->setSelected(true);
        }
    }
}

QList<QGraphicsItem *> loadItems(QByteArray &itemData, const QVector<int> &ids, Editor *editor, QVector<int> &otherIds)
{
    if (itemData.isEmpty()) {
        return {};
    }
    auto otherElms = findElements(otherIds).toVector();
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    double version = GlobalProperties::version;
    QMap<quint64, QNEPort *> portMap;
    for (auto *elm : qAsConst(otherElms)) {
        elm->load(dataStream, portMap, version);
    }
    /*
     * Assuming that all connections are stored after the elements, we will deserialize the elements first.
     * We will store one additional information: The element IDs!
     */
    auto items = SerializationFunctions::deserialize(dataStream, version, portMap);
    if (items.size() != ids.size()) {
        QString msg = QObject::tr("One or more elements were not found on scene. Expected %1, found %2.").arg(ids.size(), items.size());
        throw std::runtime_error(ERRORMSG(msg.toStdString()));
    }
    for (int i = 0; i < items.size(); ++i) {
        auto *iwid = dynamic_cast<ItemWithId *>(items[i]);
        if (iwid) {
            ElementFactory::updateItemId(iwid, ids[i]);
        }
    }
    addItems(editor, items);
    return items;
}

void deleteItems(const QList<QGraphicsItem *> &items, Editor *editor)
{
    QVector<QGraphicsItem *> itemsVec = items.toVector();
    /* Delete items on reverse order */
    for (int i = itemsVec.size() - 1; i >= 0; --i) {
        editor->getScene()->removeItem(itemsVec[i]);
        delete itemsVec[i];
    }
}

AddItemsCommand::AddItemsCommand(GraphicElement *aItem, Editor *aEditor, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    QList<QGraphicsItem *> items({aItem});
    items = loadList(items, m_ids, m_otherIds);
    m_editor = aEditor;
    addItems(m_editor, items);
    setText(tr("Add %1 element").arg(aItem->objectName()));
}

AddItemsCommand::AddItemsCommand(QNEConnection *aItem, Editor *aEditor, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    QList<QGraphicsItem *> items({aItem});
    items = loadList(items, m_ids, m_otherIds);
    m_editor = aEditor;
    addItems(m_editor, items);
    setText(tr("Add connection"));
}

AddItemsCommand::AddItemsCommand(const QList<QGraphicsItem *> &aItems, Editor *aEditor, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    QList<QGraphicsItem *> items = loadList(aItems, m_ids, m_otherIds);
    m_editor = aEditor;
    addItems(m_editor, items);
    setText(tr("Add %1 elements").arg(items.size()));
}

void AddItemsCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    QList<QGraphicsItem *> items = findItems(m_ids);

    SimulationController *sc = m_editor->getSimulationController();
    // We need to restart the simulation controller when deleting through the Undo command to
    // guarantee that no crashes occur when deleting input elements (clocks, input buttons, etc.)
    sc->setRestart();

    saveItems(m_itemData, items, m_otherIds);
    deleteItems(items, m_editor);
    m_editor->setCircuitUpdateRequired();
}

void AddItemsCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    loadItems(m_itemData, m_ids, m_editor, m_otherIds);
    m_editor->setCircuitUpdateRequired();
}

DeleteItemsCommand::DeleteItemsCommand(const QList<QGraphicsItem *> &aItems, Editor *aEditor, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    QList<QGraphicsItem *> items = loadList(aItems, m_ids, m_otherIds);
    m_editor = aEditor;
    setText(tr("Delete %1 elements").arg(items.size()));
}

DeleteItemsCommand::DeleteItemsCommand(QGraphicsItem *item, Editor *aEditor, QUndoCommand *parent)
    : DeleteItemsCommand(QList<QGraphicsItem *>({item}), aEditor, parent)
{
}

void DeleteItemsCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    loadItems(m_itemData, m_ids, m_editor, m_otherIds);
    m_editor->setCircuitUpdateRequired();
}

void DeleteItemsCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    QList<QGraphicsItem *> items = findItems(m_ids);
    saveItems(m_itemData, items, m_otherIds);
    deleteItems(items, m_editor);
    m_editor->setCircuitUpdateRequired();
}

RotateCommand::RotateCommand(const QList<GraphicElement *> &aItems, int aAngle, Editor *aEditor, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_angle(aAngle)
    , m_editor(aEditor)
{
    setText(tr("Rotate %1 degrees").arg(m_angle));
    m_ids.reserve(aItems.size());
    m_positions.reserve(aItems.size());
    for (auto *item : aItems) {
        m_positions.append(item->pos());
        item->setPos(item->pos());
        m_ids.append(item->id());
    }
}

void RotateCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    QList<GraphicElement *> list = findElements(m_ids);
    QGraphicsScene *scn = list[0]->scene();
    scn->clearSelection();
    for (int i = 0; i < list.size(); ++i) {
        GraphicElement *elm = list[i];
        if (elm->rotatable()) {
            elm->setRotation(elm->rotation() - m_angle);
        }
        elm->setPos(m_positions[i]);
        elm->update();
        elm->setSelected(true);
    }
    m_editor->setAutoSaveRequired();
}

void RotateCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    QList<GraphicElement *> list = findElements(m_ids);
    QGraphicsScene *scn = list[0]->scene();
    scn->clearSelection();
    double cx = 0, cy = 0;
    int sz = 0;
    for (auto *item : qAsConst(list)) {
        cx += item->pos().x();
        cy += item->pos().y();
        sz++;
    }
    cx /= sz;
    cy /= sz;
    for (auto *elm : qAsConst(list)) {
        QTransform transform;
        transform.translate(cx, cy);
        transform.rotate(m_angle);
        transform.translate(-cx, -cy);
        if (elm->rotatable()) {
            elm->setRotation(elm->rotation() + m_angle);
        }
        elm->setPos(transform.map(elm->pos()));
        elm->update();
        elm->setSelected(true);
    }
    m_editor->setAutoSaveRequired();
}

bool RotateCommand::mergeWith(const QUndoCommand *command)
{
    const auto *rotateCommand = dynamic_cast<const RotateCommand *>(command);
    if (m_ids.size() != rotateCommand->m_ids.size()) {
        return false;
    }
    auto list = findElements(m_ids).toVector();
    auto list2 = findElements(rotateCommand->m_ids).toVector();
    for (int i = 0; i < list.size(); ++i) {
        if (list[i] != list2[i]) {
            return false;
        }
    }
    m_angle = (m_angle + rotateCommand->m_angle) % 360;
    setText(tr("Rotate %1 degrees").arg(m_angle));
    undo();
    redo();
    return true;
}

int RotateCommand::id() const
{
    return Id;
}

MoveCommand::MoveCommand(const QList<GraphicElement *> &list, const QList<QPointF> &oldPositions, Editor *aEditor, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_oldPositions(oldPositions)
    , m_editor(aEditor)
{
    m_newPositions.reserve(list.size());
    m_ids.reserve(list.size());
    for (auto *elm : list) {
        m_ids.append(elm->id());
        m_newPositions.append(elm->pos());
    }
    setText(tr("Move elements"));
}

void MoveCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    auto elms = findElements(m_ids).toVector();
    for (int i = 0; i < elms.size(); ++i) {
        elms[i]->setPos(m_oldPositions[i]);
    }
    m_editor->setAutoSaveRequired();
}

void MoveCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    auto elms = findElements(m_ids).toVector();
    for (int i = 0; i < elms.size(); ++i) {
        elms[i]->setPos(m_newPositions[i]);
    }
    m_editor->setAutoSaveRequired();
}

UpdateCommand::UpdateCommand(const QVector<GraphicElement *> &elements, const QByteArray &oldData, Editor *editor, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_editor(editor)
{
    m_oldData = oldData;
    ids.reserve(elements.size());
    QDataStream dataStream(&m_newData, QIODevice::WriteOnly);
    for (auto *elm : elements) {
        elm->save(dataStream);
        ids.append(elm->id());
    }
    setText(tr("Update %1 elements").arg(elements.size()));
}

void UpdateCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    loadData(m_oldData);
    m_editor->setCircuitUpdateRequired();
}

void UpdateCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    loadData(m_newData);
    m_editor->setCircuitUpdateRequired();
}

void UpdateCommand::loadData(QByteArray itemData)
{
    auto elements = findElements(ids).toVector();
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    QMap<quint64, QNEPort *> portMap;
    if (!elements.isEmpty() && elements.front()->scene()) {
        elements.front()->scene()->clearSelection();
    }
    double version = GlobalProperties::version;
    if (!elements.isEmpty()) {
        for (auto *elm : qAsConst(elements)) {
            elm->load(dataStream, portMap, version);
            elm->setSelected(true);
        }
    }
}

SplitCommand::SplitCommand(QNEConnection *conn, QPointF point, Editor *editor, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_editor(editor)
    , m_scene(editor->getScene())
{
    GraphicElement *node = ElementFactory::buildElement(ElementType::Node);
    QNEConnection *conn2 = ElementFactory::instance->buildConnection();

    /* Align node to Grid */
    m_nodePos = point - node->boundingRect().center();
    if (m_scene) {
        int gridSize = m_scene->gridSize();
        qreal xV = qRound(m_nodePos.x() / gridSize) * gridSize;
        qreal yV = qRound(m_nodePos.y() / gridSize) * gridSize;
        m_nodePos = QPointF(xV, yV);
    }
    /* Rotate line according to angle between p1 and p2 */
    m_nodeAngle = conn->angle();
    m_nodeAngle = 360 - 90 * (std::round(m_nodeAngle / 90.0));

    /* Assingning class attributes */
    m_elm1_id = conn->start()->graphicElement()->id();
    m_elm2_id = conn->end()->graphicElement()->id();

    m_c1_id = conn->id();
    m_c2_id = conn2->id();

    m_node_id = node->id();

    setText(tr("Wire split"));
}

QNEConnection *findConn(int id)
{
    return dynamic_cast<QNEConnection *>(ElementFactory::getItemById(id));
}

GraphicElement *findElm(int id)
{
    return dynamic_cast<GraphicElement *>(ElementFactory::getItemById(id));
}

void SplitCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    QNEConnection *c1 = findConn(m_c1_id);
    QNEConnection *c2 = findConn(m_c2_id);
    GraphicElement *node = findElm(m_node_id);
    GraphicElement *elm1 = findElm(m_elm1_id);
    GraphicElement *elm2 = findElm(m_elm2_id);
    if (!c2) {
        c2 = ElementFactory::buildConnection();
        ElementFactory::updateItemId(c2, m_c2_id);
    }
    if (!node) {
        node = ElementFactory::buildElement(ElementType::Node);
        ElementFactory::updateItemId(node, m_node_id);
    }
    if (c1 && c2 && elm1 && elm2 && node) {
        node->setPos(m_nodePos);
        node->setRotation(m_nodeAngle);

        // QNEOutputPort *startPort = c1->start();
        QNEInputPort *endPort = c1->end();
        c2->setStart(node->output());
        c2->setEnd(endPort);
        c1->setEnd(node->input());

        m_scene->addItem(node);
        m_scene->addItem(c2);

        c1->updatePosFromPorts();
        c1->updatePath();
        c2->updatePosFromPorts();
        c2->updatePath();
    } else {
        throw std::runtime_error(ERRORMSG(QObject::tr("Error trying to redo ").toStdString()) + text().toStdString());
    }
    m_editor->setCircuitUpdateRequired();
}

void SplitCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    QNEConnection *c1 = findConn(m_c1_id);
    QNEConnection *c2 = findConn(m_c2_id);
    GraphicElement *node = findElm(m_node_id);
    GraphicElement *elm1 = findElm(m_elm1_id);
    GraphicElement *elm2 = findElm(m_elm2_id);
    if (c1 && c2 && elm1 && elm2 && node) {
        c1->setEnd(c2->end());

        c1->updatePosFromPorts();
        c1->updatePath();

        m_editor->getScene()->removeItem(c2);
        m_editor->getScene()->removeItem(node);

        delete c2;
        delete node;
    } else {
        throw std::runtime_error(ERRORMSG(QObject::tr("Error trying to undo ").toStdString()) + text().toStdString());
    }
    m_editor->setCircuitUpdateRequired();
}

MorphCommand::MorphCommand(const QVector<GraphicElement *> &elements, ElementType aType, Editor *aEditor, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    m_newtype = aType;
    m_editor = aEditor;
    m_ids.reserve(elements.size());
    m_types.reserve(elements.size());
    for (auto *oldElm : elements) {
        m_ids.append(oldElm->id());
        m_types.append(oldElm->elementType());
    }
    setText(tr("Morph %1 elements to %2").arg(elements.size()).arg(elements.front()->objectName()));
}

void MorphCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    QVector<GraphicElement *> newElms = findElements(m_ids).toVector();
    QVector<GraphicElement *> oldElms(newElms.size());
    for (int i = 0; i < m_ids.size(); ++i) {
        oldElms[i] = ElementFactory::buildElement(m_types[i]);
    }
    transferConnections(newElms, oldElms);
    m_editor->setCircuitUpdateRequired();
}

void MorphCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    QVector<GraphicElement *> oldElms = findElements(m_ids).toVector();
    QVector<GraphicElement *> newElms(oldElms.size());
    for (int i = 0; i < m_ids.size(); ++i) {
        newElms[i] = ElementFactory::buildElement(m_newtype);
    }
    transferConnections(oldElms, newElms);
    m_editor->setCircuitUpdateRequired();
}

void MorphCommand::transferConnections(QVector<GraphicElement *> from, QVector<GraphicElement *> to)
{
    for (int elm = 0; elm < from.size(); ++elm) {
        GraphicElement *oldElm = from[elm];
        GraphicElement *newElm = to[elm];
        newElm->setInputSize(oldElm->inputSize());

        newElm->setPos(oldElm->pos());
        if (newElm->rotatable() && oldElm->rotatable()) {
            newElm->setRotation(oldElm->rotation());
        }
        if ((newElm->elementType() == ElementType::Not) && (oldElm->elementType() == ElementType::Node)) {
            newElm->setRotation(oldElm->rotation() + 90.0);
        } else if ((newElm->elementType() == ElementType::Node) && (oldElm->elementType() == ElementType::Not)) {
            newElm->setRotation(oldElm->rotation() - 90.0);
        }
        if (newElm->hasLabel() && oldElm->hasLabel()) {
            newElm->setLabel(oldElm->getLabel());
        }
        if (newElm->hasColors() && oldElm->hasColors()) {
            newElm->setColor(oldElm->getColor());
        }
        if (newElm->hasFrequency() && oldElm->hasFrequency()) {
            newElm->setFrequency(oldElm->getFrequency());
        }
        if (newElm->hasTrigger() && oldElm->hasTrigger()) {
            newElm->setTrigger(oldElm->getTrigger());
        }
        for (int in = 0; in < oldElm->inputSize(); ++in) {
            while (!oldElm->input(in)->connections().isEmpty()) {
                QNEConnection *conn = oldElm->input(in)->connections().first();
                if (conn->end() == oldElm->input(in)) {
                    conn->setEnd(newElm->input(in));
                }
            }
        }
        for (int out = 0; out < oldElm->outputSize(); ++out) {
            while (!oldElm->output(out)->connections().isEmpty()) {
                QNEConnection *conn = oldElm->output(out)->connections().first();
                if (conn->start() == oldElm->output(out)) {
                    conn->setStart(newElm->output(out));
                }
            }
        }
        int oldId = oldElm->id();
        m_editor->getScene()->removeItem(oldElm);
        delete oldElm;

        ElementFactory::updateItemId(newElm, oldId);
        m_editor->getScene()->addItem(newElm);
        newElm->updatePorts();
    }
}

ChangeInputSZCommand::ChangeInputSZCommand(const QVector<GraphicElement *> &elements, int newInputSize, Editor *editor, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_editor(editor)
{
    for (auto *elm : elements) {
        m_elms.append(elm->id());
    }
    m_newInputSize = newInputSize;
    if (!elements.isEmpty()) {
        m_scene = elements.front()->scene();
    }
    setText(tr("Change input size to %1").arg(newInputSize));
}

void ChangeInputSZCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    const QVector<GraphicElement *> m_elements = findElements(m_elms).toVector();
    if (!m_elements.isEmpty() && m_elements.front()->scene()) {
        m_scene->clearSelection();
    }
    QVector<GraphicElement *> serializationOrder;
    m_oldData.clear();
    QDataStream dataStream(&m_oldData, QIODevice::WriteOnly);
    for (auto *elm : m_elements) {
        elm->save(dataStream);
        serializationOrder.append(elm);
        for (int in = m_newInputSize; in < elm->inputSize(); ++in) {
            for (auto *conn : elm->input(in)->connections()) {
                auto *otherPort = conn->otherPort(elm->input(in));
                otherPort->graphicElement()->save(dataStream);
                serializationOrder.append(otherPort->graphicElement());
            }
        }
    }
    for (auto *elm : m_elements) {
        for (int in = m_newInputSize; in < elm->inputSize(); ++in) {
            while (!elm->input(in)->connections().isEmpty()) {
                auto *conn = elm->input(in)->connections().front();
                conn->save(dataStream);
                m_scene->removeItem(conn);
                auto *otherPort = conn->otherPort(elm->input(in));
                elm->input(in)->disconnect(conn);
                otherPort->disconnect(conn);
            }
        }
        elm->setInputSize(m_newInputSize);
        elm->setSelected(true);
    }
    m_order.clear();
    for (auto *elm : serializationOrder) {
        m_order.append(elm->id());
    }
    m_editor->setCircuitUpdateRequired();
}

void ChangeInputSZCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    const QVector<GraphicElement *> m_elements = findElements(m_elms).toVector();
    const QVector<GraphicElement *> serializationOrder = findElements(m_order).toVector();
    if (!m_elements.isEmpty() && m_elements.front()->scene()) {
        m_scene->clearSelection();
    }
    QDataStream dataStream(&m_oldData, QIODevice::ReadOnly);
    double version = GlobalProperties::version;
    QMap<quint64, QNEPort *> portMap;
    for (auto *elm : serializationOrder) {
        elm->load(dataStream, portMap, version);
    }
    for (auto *elm : m_elements) {
        for (int in = m_newInputSize; in < elm->inputSize(); ++in) {
            auto *conn = ElementFactory::buildConnection();
            conn->load(dataStream, portMap);
            m_scene->addItem(conn);
        }
        elm->setSelected(true);
    }
    m_editor->setCircuitUpdateRequired();
}

FlipCommand::FlipCommand(const QList<GraphicElement *> &aItems, int aAxis, Editor *aEditor, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_axis(aAxis)
    , m_editor(aEditor)
{
    setText(tr("Flip %1 elements in axis %2").arg(aItems.size(), aAxis));
    m_ids.reserve(aItems.size());
    m_positions.reserve(aItems.size());
    double xmin, xmax, ymin, ymax;
    if (!aItems.empty()) {
        xmin = xmax = aItems.first()->pos().rx();
        ymin = ymax = aItems.first()->pos().ry();
        for (auto *item : aItems) {
            m_positions.append(item->pos());
            item->setPos(item->pos());
            m_ids.append(item->id());
            xmin = qMin(xmin, item->pos().rx());
            xmax = qMax(xmax, item->pos().rx());
            ymin = qMin(ymin, item->pos().ry());
            ymax = qMax(ymax, item->pos().ry());
        }
        m_minPos = QPointF(xmin, ymin);
        m_maxPos = QPointF(xmax, ymax);
    }
}

void FlipCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    redo();
}

void FlipCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    // TODO: this might detach the QList
    auto list = findElements(m_ids);
    auto *scn = list[0]->scene();
    scn->clearSelection();
    for (auto *elm : qAsConst(list)) {
        auto pos = elm->pos();
        if (m_axis == 0) {
            pos.setX(m_minPos.rx() + (m_maxPos.rx() - pos.rx()));
        } else {
            pos.setY(m_minPos.ry() + (m_maxPos.ry() - pos.ry()));
        }
        elm->setPos(pos);
        elm->update();
        elm->setSelected(true);
        if (elm->rotatable()) {
            elm->setRotation(elm->rotation() + 180);
        }
    }
    m_editor->setAutoSaveRequired();
}

ChangeOutputSZCommand::ChangeOutputSZCommand(const QVector<GraphicElement *> &elements, int newOutputSize, Editor *editor, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_editor(editor)
{
    for (auto *elm : elements) {
        m_elms.append(elm->id());
    }
    m_newOutputSize = newOutputSize;
    if (!elements.isEmpty()) {
        m_scene = elements.front()->scene();
    }
    setText(tr("Change input size to %1").arg(newOutputSize));
}

void ChangeOutputSZCommand::redo()
{
    qCDebug(zero) << "REDO " + text();
    const QVector<GraphicElement *> m_elements = findElements(m_elms).toVector();
    if (!m_elements.isEmpty() && m_elements.front()->scene()) {
        m_scene->clearSelection();
    }
    QVector<GraphicElement *> serializationOrder;
    m_oldData.clear();
    QDataStream dataStream(&m_oldData, QIODevice::WriteOnly);
    for (auto *elm : m_elements) {
        elm->save(dataStream);
        serializationOrder.append(elm);
        for (int out = m_newOutputSize; out < elm->outputSize(); ++out) {
            for (auto *conn : elm->output(out)->connections()) {
                auto *otherPort = conn->otherPort(elm->output(out));
                otherPort->graphicElement()->save(dataStream);
                serializationOrder.append(otherPort->graphicElement());
            }
        }
    }
    for (auto *elm : m_elements) {
        for (int out = m_newOutputSize; out < elm->outputSize(); ++out) {
            while (!elm->output(out)->connections().isEmpty()) {
                auto *conn = elm->output(out)->connections().front();
                conn->save(dataStream);
                m_scene->removeItem(conn);
                auto *otherPort = conn->otherPort(elm->output(out));
                elm->output(out)->disconnect(conn);
                otherPort->disconnect(conn);
            }
        }
        elm->setOutputSize(m_newOutputSize);
        elm->setSelected(true);
    }
    m_order.clear();
    for (auto *elm : serializationOrder) {
        m_order.append(elm->id());
    }
    m_editor->setCircuitUpdateRequired();
}

void ChangeOutputSZCommand::undo()
{
    qCDebug(zero) << "UNDO " + text();
    const QVector<GraphicElement *> m_elements = findElements(m_elms).toVector();
    const QVector<GraphicElement *> serializationOrder = findElements(m_order).toVector();
    if (!m_elements.isEmpty() && m_elements.front()->scene()) {
        m_scene->clearSelection();
    }
    QDataStream dataStream(&m_oldData, QIODevice::ReadOnly);
    double version = GlobalProperties::version;
    QMap<quint64, QNEPort *> portMap;
    for (auto *elm : serializationOrder) {
        elm->load(dataStream, portMap, version);
    }
    for (auto *elm : m_elements) {
        for (int out = m_newOutputSize; out < elm->outputSize(); ++out) {
            auto *conn = ElementFactory::buildConnection();
            conn->load(dataStream, portMap);
            m_scene->addItem(conn);
        }
        elm->setSelected(true);
    }
    m_editor->setCircuitUpdateRequired();
}
