// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icprototypeimpl.h"

#include <QFile>
#include <QFileInfo>

#include "common.h"
#include "elementfactory.h"
#include "ic.h"
#include "icprototype.h"
#include "icmapping.h"
#include "qneconnection.h"
#include "qneport.h"
#include "serializationfunctions.h"

ICPrototypeImpl::~ICPrototypeImpl()
{
    qDeleteAll(m_elements);
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

bool ICPrototypeImpl::updateLocalIC(const QString &src_fileName, const QString &tgt_fileName, const QString &dirName)
{
    COMMENT("Recursive call to sub ics: " << src_fileName.toStdString(), 0);
    COMMENT("Elements: " << QString::number(m_elements.size()).toStdString(), 0);
    for (GraphicElement *elm : qAsConst(m_elements)) {
        if (elm->elementType() == ElementType::IC) {
            IC *ic = dynamic_cast<IC *>(elm);
            QString originalSubICName = ic->getFile();
            COMMENT("moving reference of IC " << originalSubICName.toStdString(), 0);
            QString subICFileName = dirName + "/boxes/" + QFileInfo(originalSubICName).fileName();
            COMMENT("moving to " << subICFileName.toStdString(), 0);
            auto prototype = ic->getPrototype();
            if (!QFile::exists(subICFileName)) {
                COMMENT("Copying subic file to local dir. File does not exist yet.", 0);
                QFile::copy(originalSubICName, subICFileName);
                if (prototype->updateLocalIC(originalSubICName, subICFileName, dirName)) {
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
    COMMENT("Updating references of subics in IC files!!!", 0);
    try {
        if (!SerializationFunctions::update(src_fileName, tgt_fileName, dirName)) {
            std::cerr << "Error saving local ic." << std::endl;
            return false;
        }
        COMMENT("Saved at prototype impl.", 0);

    } catch (std::runtime_error &e) {
        std::cerr << "Error saving project: " << e.what() << std::endl;
        return false;
    }
    return true;
}

void ICPrototypeImpl::loadFile(const QString &fileName)
{
    COMMENT("Reading ic", 0);
    clear();
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        QDataStream ds(&file);
        QList<QGraphicsItem *> items = SerializationFunctions::load(ds, fileName);
        for (QGraphicsItem *item : qAsConst(items)) {
            loadItem(item);
        }
        file.close();
    }
    setInputSize(m_inputs.size());
    setOutputSize(m_outputs.size());
    sortPorts(m_inputs);
    sortPorts(m_outputs);
    loadInputs();
    loadOutputs();
    COMMENT("Finished Reading ic", 0);
}

void ICPrototypeImpl::loadInputs()
{
    for (int portIndex = 0; portIndex < getInputSize(); ++portIndex) {
        GraphicElement *elm = m_inputs.at(portIndex)->graphicElement();
        QString lb = elm->getLabel();
        if (!m_inputs.at(portIndex)->portName().isEmpty()) {
            lb += " ";
            lb += m_inputs.at(portIndex)->portName();
        }
        if (!elm->genericProperties().isEmpty()) {
            lb += " [" + elm->genericProperties() + "]";
        }
        m_inputLabels[portIndex] = lb;
    }
}

void ICPrototypeImpl::loadOutputs()
{
    for (int portIndex = 0; portIndex < getOutputSize(); ++portIndex) {
        GraphicElement *elm = m_outputs.at(portIndex)->graphicElement();
        QString lb = elm->getLabel();
        if (!m_outputs.at(portIndex)->portName().isEmpty()) {
            lb += " ";
            lb += m_outputs.at(portIndex)->portName();
        }
        if (!elm->genericProperties().isEmpty()) {
            lb += " [" + elm->genericProperties() + "]";
        }
        m_outputLabels[portIndex] = lb;
    }
}

void ICPrototypeImpl::loadInputElement(GraphicElement *elm)
{
    auto const outputs = elm->outputs();
    for (QNEOutputPort *port : outputs) {
        GraphicElement *nodeElm = ElementFactory::buildElement(ElementType::NODE);
        nodeElm->setPos(elm->pos());
        nodeElm->setLabel(elm->getLabel());
        if (elm->getLabel().isEmpty()) {
            nodeElm->setLabel(ElementFactory::typeToText(elm->elementType()));
        }
        QNEInputPort *nodeInput = nodeElm->input();
        nodeInput->setPos(port->pos());
        nodeInput->setName(port->getName());
        nodeInput->setRequired(false);
        nodeInput->setDefaultValue(port->value());
        nodeInput->setValue(port->value());
        if (elm->elementType() == ElementType::CLOCK) {
            nodeInput->setRequired(true);
        }
        m_inputs.append(nodeInput);
        m_elements.append(nodeElm);
        QList<QNEConnection *> conns = port->connections();
        for (QNEConnection *conn : conns) {
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
        if (elm->getLabel().isEmpty()) {
            nodeElm->setLabel(ElementFactory::typeToText(elm->elementType()));
        }
        QNEOutputPort *nodeOutput = nodeElm->output();
        nodeOutput->setPos(port->pos());
        nodeOutput->setName(port->getName());
        m_outputs.append(nodeOutput);
        m_elements.append(nodeElm);
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
                m_elements.append(elm);
            }
        }
    }
}

void ICPrototypeImpl::clear()
{
    m_inputs.clear();
    m_outputs.clear();
    setInputSize(0);
    setOutputSize(0);
    qDeleteAll(m_elements);
    m_elements.clear();
}

int ICPrototypeImpl::getInputSize() const
{
    return m_inputs.size();
}

int ICPrototypeImpl::getOutputSize() const
{
    return m_outputs.size();
}

void ICPrototypeImpl::setOutputSize(int outputSize)
{
    m_outputLabels = QVector<QString>(outputSize);
}

void ICPrototypeImpl::setInputSize(int inputSize)
{
    m_inputLabels = QVector<QString>(inputSize);
}

GraphicElement *ICPrototypeImpl::getElement(int index)
{
    return m_elements[index];
}

QString ICPrototypeImpl::getInputLabel(int index) const
{
    return m_inputLabels[index];
}

QString ICPrototypeImpl::getOutputLabel(int index) const
{
    return m_outputLabels[index];
}

QNEPort *ICPrototypeImpl::getInput(int index)
{
    return m_inputs[index];
}

QNEPort *ICPrototypeImpl::getOutput(int index)
{
    return m_outputs[index];
}

ICMapping *ICPrototypeImpl::generateMapping() const
{
    return new ICMapping(m_elements, m_inputs, m_outputs);
}
