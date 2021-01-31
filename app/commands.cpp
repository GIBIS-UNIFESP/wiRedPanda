// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commands.h"

#include <cmath>
#include <stdexcept>

#include <QDrag>
#include <QGraphicsItem>

#include "common.h"
#include "editor.h"
#include "elementfactory.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "serializationfunctions.h"
#include "simulationcontroller.h"

void storeIds(const QList<QGraphicsItem *> &items, QVector<int> &ids)
{
    ids.reserve(items.size());
    for (QGraphicsItem *item : items) {
        auto *iwid = dynamic_cast<ItemWithId *>(item);
        if (iwid) {
            ids.append(iwid->id());
        }
    }
}

void storeOtherIds(const QList<QGraphicsItem *> &connections, const QVector<int> &ids, QVector<int> &otherIds)
{
    for (QGraphicsItem *item : connections) {
        auto *conn = qgraphicsitem_cast<QNEConnection *>(item);
        if ((item->type() == QNEConnection::Type) && conn) {
            QNEOutputPort *p1 = conn->start();
            if (p1 && p1->graphicElement() && !ids.contains(p1->graphicElement()->id())) {
                otherIds.append(p1->graphicElement()->id());
            }
            QNEInputPort *p2 = conn->end();
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
    for (QGraphicsItem *item : aItems) {
        if (item->type() == GraphicElement::Type) {
            if (!elements.contains(item)) {
                elements.append(item);
            }
        }
    }
    QList<QGraphicsItem *> connections;
    /* Stores all the wires linked to these elements */
    for (QGraphicsItem *item : elements) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm) {
            auto const inputs = elm->inputs();
            for (QNEInputPort *port : inputs) {
                for (QNEConnection *conn : port->connections()) {
                    if (!connections.contains(conn)) {
                        connections.append(conn);
                    }
                }
            }
            auto const outputs = elm->outputs();
            for (QNEOutputPort *port : outputs) {
                for (QNEConnection *conn : port->connections()) {
                    if (!connections.contains(conn)) {
                        connections.append(conn);
                    }
                }
            }
        }
    }
    /* Stores the other wires selected */
    for (QGraphicsItem *item : aItems) {
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
        throw std::runtime_error(ERRORMSG("One or more items was not found on the scene."));
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
        throw std::runtime_error(ERRORMSG("One or more elements was not found on the scene."));
    }
    return items;
}

void saveItems(QByteArray &itemData, const QList<QGraphicsItem *> &items, const QVector<int> &otherIds)
{
    itemData.clear();
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    QList<GraphicElement *> others = findElements(otherIds);
    for (GraphicElement *elm : qAsConst(others)) {
        elm->save(dataStream);
    }
    SerializationFunctions::serialize(items, dataStream);
}

void addItems(Editor *editor, const QList<QGraphicsItem *>& items)
{
    editor->getScene()->clearSelection();
    for (QGraphicsItem *item : items) {
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
        return QList<QGraphicsItem *>();
    }
    QVector<GraphicElement *> otherElms = findElements(otherIds).toVector();
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    double version = GlobalProperties::version;
    QMap<quint64, QNEPort *> portMap;
    for (GraphicElement *elm : qAsConst(otherElms)) {
        elm->load(dataStream, portMap, version);
    }
    /*
     * Assuming that all connections are stored after the elements, we will deserialize the elements first.
     * We will store one additional information: The element IDs!
     */
    QList<QGraphicsItem *> items = SerializationFunctions::deserialize(dataStream, version, GlobalProperties::currentFile, portMap);
    if (items.size() != ids.size()) {
        QString msg("One or more elements were not found on scene. Expected %1, found %2.");
        msg = msg.arg(ids.size()).arg(items.size());
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

void AddItemsCommand::undo()
{
    COMMENT("UNDO " + text().toStdString(), 0);
    QList<QGraphicsItem *> items = findItems(m_ids);

    SimulationController *sc = this->m_editor->getSimulationController();
    // We need to restart the simulation controller when deleting through the Undo command to
    // guarantee that no crashes occur when deleting input elements (clocks, input buttons, etc.)
    sc->shouldRestart = true;

    saveItems(m_itemData, items, m_otherIds);
    deleteItems(items, m_editor);
    emit m_editor->circuitHasChanged();
}

void AddItemsCommand::redo()
{
    COMMENT("REDO " + text().toStdString(), 0);
    // TODO: items seems unused
    QList<QGraphicsItem *> items = loadItems(m_itemData, m_ids, m_editor, m_otherIds);
    emit m_editor->circuitHasChanged();
}

void DeleteItemsCommand::undo()
{
    COMMENT("UNDO " + text().toStdString(), 0);
    loadItems(m_itemData, m_ids, m_editor, m_otherIds);
    emit m_editor->circuitHasChanged();
}

void DeleteItemsCommand::redo()
{
    COMMENT("REDO " + text().toStdString(), 0);
    QList<QGraphicsItem *> items = findItems(m_ids);
    saveItems(m_itemData, items, m_otherIds);
    deleteItems(items, m_editor);
    emit m_editor->circuitHasChanged();
}

RotateCommand::RotateCommand(const QList<GraphicElement *> &aItems, int aAngle, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    m_angle = aAngle;
    setText(tr("Rotate %1 degrees").arg(m_angle));
    m_ids.reserve(aItems.size());
    m_positions.reserve(aItems.size());
    for (GraphicElement *item : aItems) {
        m_positions.append(item->pos());
        item->setPos(item->pos());
        m_ids.append(item->id());
    }
}

void RotateCommand::undo()
{
    COMMENT("UNDO " + text().toStdString(), 0);
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
}

void RotateCommand::redo()
{
    COMMENT("REDO " + text().toStdString(), 0);
    QList<GraphicElement *> list = findElements(m_ids);
    QGraphicsScene *scn = list[0]->scene();
    scn->clearSelection();
    double cx = 0, cy = 0;
    int sz = 0;
    for (GraphicElement *item : qAsConst(list)) {
        cx += item->pos().x();
        cy += item->pos().y();
        sz++;
    }
    cx /= sz;
    cy /= sz;
    for (GraphicElement *elm : qAsConst(list)) {
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
}

bool RotateCommand::mergeWith(const QUndoCommand *command)
{
    const auto *rotateCommand = static_cast<const RotateCommand *>(command);
    if (m_ids.size() != rotateCommand->m_ids.size()) {
        return false;
    }
    QVector<GraphicElement *> list = findElements(m_ids).toVector();
    QVector<GraphicElement *> list2 = findElements(rotateCommand->m_ids).toVector();
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

MoveCommand::MoveCommand(const QList<GraphicElement *> &list, const QList<QPointF> &oldPositions, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    m_oldPositions = oldPositions;
    m_newPositions.reserve(list.size());
    m_ids.reserve(list.size());
    for (GraphicElement *elm : list) {
        m_ids.append(elm->id());
        m_newPositions.append(elm->pos());
    }
    setText(tr("Move elements"));
}

void MoveCommand::undo()
{
    COMMENT("UNDO " + text().toStdString(), 0);
    QVector<GraphicElement *> elms = findElements(m_ids).toVector();
    for (int i = 0; i < elms.size(); ++i) {
        elms[i]->setPos(m_oldPositions[i]);
    }
}

void MoveCommand::redo()
{
    COMMENT("REDO " + text().toStdString(), 0);
    QVector<GraphicElement *> elms = findElements(m_ids).toVector();
    for (int i = 0; i < elms.size(); ++i) {
        elms[i]->setPos(m_newPositions[i]);
    }
}

UpdateCommand::UpdateCommand(const QVector<GraphicElement *> &elements, QByteArray oldData, Editor *editor, QUndoCommand *parent)
    : QUndoCommand(parent)
    , editor(editor)
{
    m_oldData = oldData;
    ids.reserve(elements.size());
    QDataStream dataStream(&m_newData, QIODevice::WriteOnly);
    for (GraphicElement *elm : elements) {
        elm->save(dataStream);
        ids.append(elm->id());
    }
    setText(tr("Update %1 elements").arg(elements.size()));
}

void UpdateCommand::undo()
{
    COMMENT("UNDO " + text().toStdString(), 0);
    loadData(m_oldData);
    emit editor->circuitHasChanged();
}

void UpdateCommand::redo()
{
    COMMENT("REDO " + text().toStdString(), 0);
    loadData(m_newData);
    emit editor->circuitHasChanged();
}

void UpdateCommand::loadData(QByteArray itemData)
{
    QVector<GraphicElement *> elements = findElements(ids).toVector();
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    QMap<quint64, QNEPort *> portMap;
    if (!elements.isEmpty() && elements.front()->scene()) {
        elements.front()->scene()->clearSelection();
    }
    double version = GlobalProperties::version;
    if (!elements.isEmpty()) {
        for (GraphicElement *elm : qAsConst(elements)) {
            elm->load(dataStream, portMap, version);
            elm->setSelected(true);
        }
    }
}

SplitCommand::SplitCommand(QNEConnection *conn, QPointF point, Editor *editor, QUndoCommand *parent)
    : QUndoCommand(parent)
    , editor(editor)
    , scene(editor->getScene())
{
    GraphicElement *node = ElementFactory::buildElement(ElementType::NODE);
    QNEConnection *conn2 = ElementFactory::instance->buildConnection();

    /* Align node to Grid */
    nodePos = point - QPointF(node->boundingRect().center());
    if (scene) {
        int gridSize = scene->gridSize();
        qreal xV = qRound(nodePos.x() / gridSize) * gridSize;
        qreal yV = qRound(nodePos.y() / gridSize) * gridSize;
        nodePos = QPointF(xV, yV);
    }
    /* Rotate line according to angle between p1 and p2 */
    nodeAngle = conn->angle();
    nodeAngle = 360 - 90 * (std::round(nodeAngle / 90.0));

    /* Assingning class attributes */
    elm1_id = conn->start()->graphicElement()->id();
    elm2_id = conn->end()->graphicElement()->id();

    c1_id = conn->id();
    c2_id = conn2->id();

    node_id = node->id();

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
    COMMENT("REDO " + text().toStdString(), 0);
    QNEConnection *c1 = findConn(c1_id);
    QNEConnection *c2 = findConn(c2_id);
    GraphicElement *node = findElm(node_id);
    GraphicElement *elm1 = findElm(elm1_id);
    GraphicElement *elm2 = findElm(elm2_id);
    if (!c2) {
        c2 = ElementFactory::buildConnection();
        ElementFactory::updateItemId(c2, c2_id);
    }
    if (!node) {
        node = ElementFactory::buildElement(ElementType::NODE);
        ElementFactory::updateItemId(node, node_id);
    }
    if (c1 && c2 && elm1 && elm2 && node) {
        node->setPos(nodePos);
        node->setRotation(nodeAngle);

        /*    QNEOutputPort *startPort = c1->start( ); */
        QNEInputPort *endPort = c1->end();
        c2->setStart(node->output());
        c2->setEnd(endPort);
        c1->setEnd(node->input());

        scene->addItem(node);
        scene->addItem(c2);

        c1->updatePosFromPorts();
        c1->updatePath();
        c2->updatePosFromPorts();
        c2->updatePath();
    } else {
        throw std::runtime_error(ERRORMSG(QString("Error tryng to redo %1").arg(text()).toStdString()));
    }
    emit editor->circuitHasChanged();
}

void SplitCommand::undo()
{
    COMMENT("UNDO " + text().toStdString(), 0);
    QNEConnection *c1 = findConn(c1_id);
    QNEConnection *c2 = findConn(c2_id);
    GraphicElement *node = findElm(node_id);
    GraphicElement *elm1 = findElm(elm1_id);
    GraphicElement *elm2 = findElm(elm2_id);
    if (c1 && c2 && elm1 && elm2 && node) {
        c1->setEnd(c2->end());

        c1->updatePosFromPorts();
        c1->updatePath();

        editor->getScene()->removeItem(c2);
        editor->getScene()->removeItem(node);

        delete c2;
        delete node;
    } else {
        throw std::runtime_error(ERRORMSG(QString("Error tryng to undo %1").arg(text()).toStdString()));
    }
    emit editor->circuitHasChanged();
}

MorphCommand::MorphCommand(const QVector<GraphicElement *> &elements, ElementType aType, Editor *aEditor, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    newtype = aType;
    editor = aEditor;
    ids.reserve(elements.size());
    types.reserve(elements.size());
    for (GraphicElement *oldElm : elements) {
        ids.append(oldElm->id());
        types.append(oldElm->elementType());
    }
    setText(tr("Morph %1 elements to %2").arg(elements.size()).arg(elements.front()->objectName()));
}

void MorphCommand::undo()
{
    COMMENT("UNDO " + text().toStdString(), 0);
    QVector<GraphicElement *> newElms = findElements(ids).toVector();
    QVector<GraphicElement *> oldElms(newElms.size());
    for (int i = 0; i < ids.size(); ++i) {
        oldElms[i] = ElementFactory::buildElement(types[i]);
    }
    transferConnections(newElms, oldElms);
    emit editor->circuitHasChanged();
}

void MorphCommand::redo()
{
    COMMENT("REDO " + text().toStdString(), 0);
    QVector<GraphicElement *> oldElms = findElements(ids).toVector();
    QVector<GraphicElement *> newElms(oldElms.size());
    for (int i = 0; i < ids.size(); ++i) {
        newElms[i] = ElementFactory::buildElement(newtype);
    }
    transferConnections(oldElms, newElms);
    emit editor->circuitHasChanged();
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
        if ((newElm->elementType() == ElementType::NOT) && (oldElm->elementType() == ElementType::NODE)) {
            newElm->setRotation(oldElm->rotation() + 90.0);
        } else if ((newElm->elementType() == ElementType::NODE) && (oldElm->elementType() == ElementType::NOT)) {
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
        editor->getScene()->removeItem(oldElm);
        delete oldElm;

        ElementFactory::updateItemId(newElm, oldId);
        editor->getScene()->addItem(newElm);
        newElm->updatePorts();
    }
}

ChangeInputSZCommand::ChangeInputSZCommand(const QVector<GraphicElement *> &elements, int newInputSize, Editor *editor, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_editor(editor)
{
    for (GraphicElement *elm : elements) {
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
    COMMENT("REDO " + text().toStdString(), 0);
    const QVector<GraphicElement *> m_elements = findElements(m_elms).toVector();
    if (!m_elements.isEmpty() && m_elements.front()->scene()) {
        m_scene->clearSelection();
    }
    QVector<GraphicElement *> serializationOrder;
    m_oldData.clear();
    QDataStream dataStream(&m_oldData, QIODevice::WriteOnly);
    for (int i = 0; i < m_elements.size(); ++i) {
        GraphicElement *elm = m_elements[i];
        elm->save(dataStream);
        serializationOrder.append(elm);
        for (int in = m_newInputSize; in < elm->inputSize(); ++in) {
            for (QNEConnection *conn : elm->input(in)->connections()) {
                QNEPort *otherPort = conn->otherPort(elm->input(in));
                otherPort->graphicElement()->save(dataStream);
                serializationOrder.append(otherPort->graphicElement());
            }
        }
    }
    for (int i = 0; i < m_elements.size(); ++i) {
        GraphicElement *elm = m_elements[i];
        for (int in = m_newInputSize; in < elm->inputSize(); ++in) {
            while (!elm->input(in)->connections().isEmpty()) {
                QNEConnection *conn = elm->input(in)->connections().front();
                conn->save(dataStream);
                m_scene->removeItem(conn);
                QNEPort *otherPort = conn->otherPort(elm->input(in));
                elm->input(in)->disconnect(conn);
                otherPort->disconnect(conn);
            }
        }
        elm->setInputSize(m_newInputSize);
        elm->setSelected(true);
    }
    m_order.clear();
    for (GraphicElement *elm : serializationOrder) {
        m_order.append(elm->id());
    }
    emit m_editor->circuitHasChanged();
}

void ChangeInputSZCommand::undo()
{
    COMMENT("UNDO " + text().toStdString(), 0);
    const QVector<GraphicElement *> m_elements = findElements(m_elms).toVector();
    const QVector<GraphicElement *> serializationOrder = findElements(m_order).toVector();
    if (!m_elements.isEmpty() && m_elements.front()->scene()) {
        m_scene->clearSelection();
    }
    QDataStream dataStream(&m_oldData, QIODevice::ReadOnly);
    double version = GlobalProperties::version;
    QMap<quint64, QNEPort *> portMap;
    for (GraphicElement *elm : serializationOrder) {
        elm->load(dataStream, portMap, version);
    }
    for (int i = 0; i < m_elements.size(); ++i) {
        GraphicElement *elm = m_elements[i];
        for (int in = m_newInputSize; in < elm->inputSize(); ++in) {
            QNEConnection *conn = ElementFactory::buildConnection();
            conn->load(dataStream, portMap);
            m_scene->addItem(conn);
        }
        elm->setSelected(true);
    }
    emit m_editor->circuitHasChanged();
}

FlipCommand::FlipCommand(const QList<GraphicElement *> &aItems, int aAxis, QUndoCommand *parent)
    : QUndoCommand(parent)
{
    m_axis = aAxis;
    setText(tr("Flip %1 elements in axis %2").arg(aItems.size()).arg(aAxis));
    m_ids.reserve(aItems.size());
    m_positions.reserve(aItems.size());
    double xmin, xmax, ymin, ymax;
    if (aItems.size() > 0) {
        xmin = xmax = aItems.first()->pos().rx();
        ymin = ymax = aItems.first()->pos().ry();
        for (GraphicElement *item : aItems) {
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
    COMMENT("UNDO " + text().toStdString(), 0);
    redo();
}

void FlipCommand::redo()
{
    COMMENT("REDO " + text().toStdString(), 0);
    // TODO: this might detach the QList
    QList<GraphicElement *> list = findElements(m_ids);
    QGraphicsScene *scn = list[0]->scene();
    scn->clearSelection();
    for (GraphicElement *elm : qAsConst(list)) {
        QPointF pos = elm->pos();
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
}
