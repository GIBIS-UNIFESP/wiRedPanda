// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include <iostream>

#include "icprototypeimpl.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>

#include "elementfactory.h"
#include "ic.h"
#include "icprototype.h"
#include "qneconnection.h"
#include "qneport.h"
#include "serializationfunctions.h"

ICPrototypeImpl::~ICPrototypeImpl()
{
    for (GraphicElement *elm : qAsConst(elements)) {
        delete elm;
    }
}

bool comparePorts(QNEPort *port1, QNEPort *port2)
{
    QPointF p1 = port1->graphicElement()->pos();
    QPointF p2 = port2->graphicElement()->pos();
    if (p1 != p2) {
        return p1.y() < p2.y() || (qFuzzyCompare(p1.y(), p2.y()) && p1.x() < p2.x());
    }
    p1 = port1->pos();
    p2 = port2->pos();
    return p1.x() < p2.x() || (qFuzzyCompare(p1.x(), p2.x()) && p1.y() < p2.y());
}

void ICPrototypeImpl::sortPorts(QVector<QNEPort *> &map)
{
    std::stable_sort(map.begin(), map.end(), comparePorts);
}

bool ICPrototypeImpl::updateLocalIC(const QString &fileName, const QString &dirName)
{
    qDebug() << "Recursive call to sub ics.";
    for (GraphicElement *elm : qAsConst(elements)) {
        if (elm->elementType() == ElementType::IC) {
            IC *ic = dynamic_cast<IC *>(elm);
            QString originalSubICName = ic->getFile();
            QString subICFileName = dirName + "/boxes/" + QFileInfo(originalSubICName).fileName();
            auto prototype = ic->getPrototype();
            if (!QFile::exists(subICFileName)) {
                qDebug() << "Copying subic file to local dir. File does not exist yet.";
                QFile::copy(originalSubICName, subICFileName);
                if (prototype->updateLocalIC(subICFileName, dirName)) {
                    if (!ic->setFile(subICFileName)) {
                        std::cerr << "Error updating subic name." << std::endl;
                        return false;
                    }
                } else {
                    std::cerr << "Error saving subic." << std::endl;
                }
            }
        }
    }
    qDebug() << "Updating references of subics.";
    try {
        if (!SerializationFunctions::update(fileName, dirName)) {
            std::cerr << "Error saving local ic." << std::endl;
            return false;
        }
        qDebug() << "Saved at prototype impl.";

    } catch (std::runtime_error &e) {
        std::cerr << "Error saving project: " << e.what() << std::endl;
        return false;
    }
    return true;
}

void ICPrototypeImpl::loadFile(const QString &fileName)
{
    qDebug() << "Reading ic";
    clear();
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        QDataStream ds(&file);
        QList<QGraphicsItem *> items = SerializationFunctions::load(ds, fileName);
        for (QGraphicsItem *item : qAsConst(items)) {
            loadItem(item);
        }
    }
    setInputSize(inputs.size());
    setOutputSize(outputs.size());
    sortPorts(inputs);
    sortPorts(outputs);
    loadInputs();
    loadOutputs();
    qDebug() << "Finished Reading ic";
}

void ICPrototypeImpl::loadInputs()
{
    for (int portIndex = 0; portIndex < getInputSize(); ++portIndex) {
        GraphicElement *elm = inputs.at(portIndex)->graphicElement();
        QString lb = elm->getLabel();
        if (lb.isEmpty()) {
            lb = elm->objectName();
        }
        if (!inputs.at(portIndex)->portName().isEmpty()) {
            lb += " ";
            lb += inputs.at(portIndex)->portName();
        }
        if (!elm->genericProperties().isEmpty()) {
            lb += " [" + elm->genericProperties() + "]";
        }
        inputLabels[portIndex] = lb;
    }
}

void ICPrototypeImpl::loadOutputs()
{
    for (int portIndex = 0; portIndex < getOutputSize(); ++portIndex) {
        GraphicElement *elm = outputs.at(portIndex)->graphicElement();
        QString lb = elm->getLabel();
        if (lb.isEmpty()) {
            lb = elm->objectName();
        }
        if (!outputs.at(portIndex)->portName().isEmpty()) {
            lb += " ";
            lb += outputs.at(portIndex)->portName();
        }
        if (!elm->genericProperties().isEmpty()) {
            lb += " [" + elm->genericProperties() + "]";
        }
        outputLabels[portIndex] = lb;
    }
}

void ICPrototypeImpl::loadInputElement(GraphicElement *elm)
{
    auto const outputs = elm->outputs();
    for (QNEOutputPort *port : outputs) {
        GraphicElement *nodeElm = ElementFactory::buildElement(
            ElementType::NODE); // Problem here. Inputs and outputs are transformed into nodes. And I can not use created connections with new elements...
        nodeElm->setPos(elm->pos()); // Solution 1: Create new connections cloning all circuit.
        nodeElm->setLabel(elm->getLabel()); // Solution 2: Load and save ic circuit out of this code...
        QNEInputPort *nodeInput = nodeElm->input();
        nodeInput->setPos(port->pos());
        nodeInput->setName(port->getName());
        nodeInput->setRequired(false);
        nodeInput->setDefaultValue(port->value());
        nodeInput->setValue(port->value());
        if (elm->elementType() == ElementType::CLOCK) {
            nodeInput->setRequired(true);
        }
        inputs.append(nodeInput);
        elements.append(nodeElm);
        QList<QNEConnection *> conns = port->connections();
        for (QNEConnection *conn : conns) { // Solution 3. Revert this process before saving and then make it again after saving...
            if (port == conn->start()) {
                conn->setStart(nodeElm->output());
            }
        }
    }
    elm->disable(); // Maybe I will have to enable and disable it as well.
}

void ICPrototypeImpl::loadOutputElement(GraphicElement *elm)
{
    auto const inputs = elm->inputs();
    for (QNEInputPort *port : inputs) {
        GraphicElement *nodeElm = ElementFactory::buildElement(ElementType::NODE);
        nodeElm->setPos(elm->pos());
        nodeElm->setLabel(elm->getLabel());
        QNEOutputPort *nodeOutput = nodeElm->output();
        nodeOutput->setPos(port->pos());
        nodeOutput->setName(port->getName());
        outputs.append(nodeOutput);
        elements.append(nodeElm);
        QList<QNEConnection *> conns = port->connections();
        for (QNEConnection *conn : conns) {
            if (port == conn->end()) {
                conn->setEnd(nodeElm->input());
            }
        }
    }
}

void ICPrototypeImpl::loadItem(QGraphicsItem *item)
{
    if (item->type() == GraphicElement::Type) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm) {
            if (elm->elementGroup() == ElementGroup::INPUT) {
                loadInputElement(elm);
            } else if (elm->elementGroup() == ElementGroup::OUTPUT) {
                loadOutputElement(elm);
            } else {
                elements.append(elm);
            }
        }
    }
}

void ICPrototypeImpl::clear()
{
    inputs.clear();
    outputs.clear();
    setInputSize(0);
    setOutputSize(0);
    qDeleteAll(elements);
    elements.clear();
}

int ICPrototypeImpl::getInputSize() const
{
    return inputs.size();
}

int ICPrototypeImpl::getOutputSize() const
{
    return outputs.size();
}

void ICPrototypeImpl::setOutputSize(int outputSize)
{
    outputLabels = QVector<QString>(outputSize);
}

void ICPrototypeImpl::setInputSize(int inputSize)
{
    inputLabels = QVector<QString>(inputSize);
}
