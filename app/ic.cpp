// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ic.h"

#include "application.h"
#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "serializationfunctions.h"

#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

namespace
{
int id = qRegisterMetaType<IC>();
}

IC::IC(QGraphicsItem *parent)
    : GraphicElement(ElementType::IC, ElementGroup::IC, ":/basic/ic-panda.svg", tr("<b>INTEGRATED CIRCUIT</b>"), tr("IC"), 0, 0, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    m_label->setRotation(90);

    setHasLabel(true);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);

    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, [=](const QString &fileName) {
        loadFile(fileName);

        if (auto *scene_ = qobject_cast<Scene *>(scene())) {
            scene_->simulation()->restart();
        }
    });
}

void IC::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    stream << QFileInfo(m_file).fileName();
}

void IC::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    GraphicElement::load(stream, portMap, version);

    if (version >= 1.2) {
        stream >> m_file;
        loadFile(m_file);
    }
}

void IC::loadInputs()
{
    setMaxInputSize(m_icInputs.size());
    setMinInputSize(m_icInputs.size());
    setInputSize(m_icInputs.size());
    qCDebug(three) << tr("IC") << m_file << tr("-> Inputs. min:") << minInputSize() << tr(", max:") << maxInputSize() << tr(", current:") << inputSize() << tr(", m_inputs:") << m_inputPorts.size();

    for (int inputIndex = 0; inputIndex < m_icInputs.size(); ++inputIndex) {
        auto *inpPort = inputPort(inputIndex);
        inpPort->setName(m_icInputLabels.at(inputIndex));
        inpPort->setRequired(m_icInputs.at(inputIndex)->isRequired());
        inpPort->setDefaultStatus(m_icInputs.at(inputIndex)->status());
        inpPort->setStatus(m_icInputs.at(inputIndex)->status());
    }
}

void IC::loadOutputs()
{
    setMaxOutputSize(m_icOutputs.size());
    setMinOutputSize(m_icOutputs.size());
    setOutputSize(m_icOutputs.size());

    for (int outputIndex = 0; outputIndex < m_icOutputs.size(); ++outputIndex) {
        auto *outPort = outputPort(outputIndex);
        outPort->setName(m_icOutputLabels[outputIndex]);
    }

    qCDebug(three) << tr("IC") << m_file << tr("-> Outputs. min:") << minOutputSize() << tr(", max:") << maxOutputSize() << tr(", current:") << outputSize() << tr(", m_outputs:") << m_outputPorts.size();
}

void IC::loadFile(const QString &fileName)
{
    qCDebug(zero) << QObject::tr("Reading IC.");

    m_icInputs.clear();
    m_icOutputs.clear();
    setInputSize(0);
    setOutputSize(0);
    qDeleteAll(m_icElements);
    m_icElements.clear();

    // ----------------------------------------------

    QFileInfo fileInfo;
    fileInfo.setFile(fileName);

    if (fileInfo.isRelative()) {
        fileInfo.setFile(QFileInfo(GlobalProperties::currentFile).absolutePath(), QFileInfo(fileName).fileName());
    }

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        throw Pandaception(fileInfo.absoluteFilePath() + tr(" not found."));
    }

    m_fileWatcher.addPath(fileInfo.absoluteFilePath());

    // ----------------------------------------------

    QFile file(fileInfo.absoluteFilePath());

    if (!file.open(QIODevice::ReadOnly)) {
        throw Pandaception(QObject::tr("Error opening file: ") + file.errorString());
    }

    QDataStream stream(&file);
    const auto items = SerializationFunctions::load(stream);

    for (auto *item : items) {
        if (item->type() != GraphicElement::Type) {
            continue;
        }

        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);

        switch(elm->elementGroup()){
        case ElementGroup::Input:  loadInputElement(elm);    break;
        case ElementGroup::Output: loadOutputElement(elm);   break;
        default:                   m_icElements.append(elm); break;
        }
    }

    file.close();

    m_icInputLabels = QVector<QString>(m_icInputs.size());
    m_icOutputLabels = QVector<QString>(m_icOutputs.size());
    sortPorts(m_icInputs);
    sortPorts(m_icOutputs);
    loadInputsLabels();
    loadOutputsLabels();

    // ----------------------------------------------

    m_file = fileInfo.absoluteFilePath();
    setToolTip(m_file);

    if (label().isEmpty()) {
        setLabel(fileInfo.baseName().toUpper());
    }

    loadInputs();
    loadOutputs();
    updatePortsProperties();

    const qreal bottom = portsBoundingRect().bottom();
    m_label->setPos(30, bottom + 5);

    qCDebug(zero) << QObject::tr("Finished Reading IC.");
}

QString IC::file() const
{
    return m_file;
}

void IC::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    qApp->mainWindow()->loadPandaFile(m_file);
}

QRectF IC::boundingRect() const
{
    return portsBoundingRect().united(pixmap().rect());
}

void IC::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    if (isSelected()) {
        painter->setBrush(m_selectionBrush);
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        painter->drawRoundedRect(boundingRect(), 5, 5);
    }

    painter->setBrush(QColor(126, 126, 126));
    painter->setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));

    QRectF rectChildren = portsBoundingRect();

    // draw package
    QPointF topLeft = rectChildren.topLeft();
    topLeft.setX(topLeft.x() + 7);
    QSizeF sizeFinal = rectChildren.size();
    sizeFinal.setWidth(sizeFinal.width() - 14);
    QRectF rectFinal = QRectF(topLeft, sizeFinal);

    painter->drawRoundedRect(rectFinal, 3, 3);
    QPixmap panda(":/basic/ic-panda2.svg");
    QPointF pandaCenter = QRectF(rectFinal.center(), panda.size()).center();
    painter->drawPixmap(pandaCenter - rectFinal.center(), panda);

    // draw shadow
    painter->setBrush(QColor(78, 78, 78));
    painter->setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));
    QRectF rec(rectFinal.bottomLeft(), rectFinal.bottomRight());
    rec.adjust(0, -3, 0, 0);
    painter->drawRoundedRect(rec, 3, 3);

    // draw semicircle
    QRectF topCenter = QRectF(rectFinal.topLeft() + QPointF(+18, -12), QSize(24, 24));
    painter->drawChord(topCenter, 0, -180 * 16);
}

QRectF IC::portsBoundingRect() const
{
    QRectF rectChildren(0, 0, 64, 64);
    const auto children = childItems();

    for (auto *child : children) {
        if (auto *port = qgraphicsitem_cast<QNEPort *>(child)) {
            rectChildren = rectChildren.united(mapRectFromItem(port, port->boundingRect()));
        }
    }

    return rectChildren;
}

void IC::loadInputElement(GraphicElement *elm)
{
    for (auto *outputPort : elm->outputs()) {
        auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
        nodeElm->setPos(elm->pos());
        nodeElm->setLabel(elm->label().isEmpty() ?
                              ElementFactory::typeToText(elm->elementType())
                            : elm->label());

        auto *nodeInput = nodeElm->inputPort();
        nodeInput->setName(outputPort->name());
        nodeInput->setRequired(elm->elementType() == ElementType::Clock);
        nodeInput->setDefaultStatus(outputPort->status());
        nodeInput->setStatus(outputPort->status());

        m_icInputs.append(nodeInput);
        m_icElements.append(nodeElm);

        const auto conns = outputPort->connections();

        for (auto *conn : conns) {
            conn->setStart(nodeElm->outputPort());
        }
    }

    delete elm;
}

void IC::loadOutputElement(GraphicElement *elm)
{
    for (auto *inputPort : elm->inputs()) {
        auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
        nodeElm->setPos(elm->pos());
        nodeElm->setLabel(elm->label().isEmpty() ?
                              ElementFactory::typeToText(elm->elementType())
                            : elm->label());

        auto *nodeOutput = nodeElm->outputPort();
        nodeOutput->setName(inputPort->name());

        m_icOutputs.append(nodeOutput);
        m_icElements.append(nodeElm);

        for (auto *conn : inputPort->connections()) {
            conn->setEnd(nodeElm->inputPort());
        }
    }

    delete elm;
}

bool IC::comparePorts(QNEPort *port1, QNEPort *port2)
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

void IC::sortPorts(QVector<QNEPort *> &map)
{
    std::stable_sort(map.begin(), map.end(), comparePorts);
}

LogicElement *IC::icInput(const int index)
{
    return m_icInputs.at(index)->logic();
}

LogicElement *IC::icOutput(const int index)
{
    return m_icOutputs.at(index)->logic();
}

void IC::loadInputsLabels()
{
    for (int portIndex = 0; portIndex < inputSize(); ++portIndex) {
        auto *inputPort = m_icInputs.at(portIndex);
        auto *elm = inputPort->graphicElement();
        QString lb = elm->label();

        if (!inputPort->portName().isEmpty()) {
            lb += " ";
            lb += inputPort->portName();
        }

        if (!elm->genericProperties().isEmpty()) {
            lb += " [" + elm->genericProperties() + "]";
        }

        m_icInputLabels[portIndex] = lb;
    }
}

void IC::loadOutputsLabels()
{
    for (int portIndex = 0; portIndex < outputSize(); ++portIndex) {
        auto *outputPort = m_icOutputs.at(portIndex);
        auto *elm = outputPort->graphicElement();
        QString lb = elm->label();

        if (!outputPort->portName().isEmpty()) {
            lb += " ";
            lb += outputPort->portName();
        }

        if (!elm->genericProperties().isEmpty()) {
            lb += " [" + elm->genericProperties() + "]";
        }

        m_icOutputLabels[portIndex] = lb;
    }
}

ElementMapping *IC::generateMap() const
{
    return new ElementMapping(m_icElements);
}
