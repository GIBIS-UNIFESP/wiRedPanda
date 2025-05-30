// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ic.h"

#include "application.h"
#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "serialization.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

IC::IC(QGraphicsItem *parent)
    : GraphicElement(ElementType::IC, ElementGroup::IC, "", tr("INTEGRATED CIRCUIT"), tr("IC"), 0, 0, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_label->setRotation(90);

    setHasLabel(true);

    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, [=](const QString &filePath) {
        loadFile(filePath);

        if (auto *scene_ = qobject_cast<Scene *>(scene())) {
            scene_->simulation()->restart();
        }
    });
}

IC::~IC()
{
    delete mapping;
}

void IC::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("fileName", QFileInfo(m_file).fileName());

    stream << map;
}

void IC::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    if ((VERSION("1.2") <= version) && (version < VERSION("4.1"))) {
        stream >> m_file;

        if (IC::needToCopyFiles) {
            copyFile();
        }

        loadFile(m_file);
    }

    if (version >= VERSION("4.1")) {
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("fileName")) {
            m_file = map.value("fileName").toString();

            if (IC::needToCopyFiles) {
                copyFile();
            }

            loadFile(m_file);
        }
    }
}

void IC::copyFile()
{
    const QString srcPath = IC::srcPath_ + "/" + m_file;
    const QString destPath = IC::destPath_ + "/" + m_file;

    QFile destFile;

    if (!QFile::exists(destPath) && !destFile.copy(srcPath, destPath)) {
        throw Pandaception(tr("Error copying file: ") + destFile.errorString());
    }
}

void IC::loadInputs()
{
    setMaxInputSize(m_icInputs.size());
    setMinInputSize(m_icInputs.size());
    setInputSize(m_icInputs.size());
    qCDebug(three) << "IC " << m_file << " -> Inputs. min: " << minInputSize() << ", max: " << maxInputSize() << ", current: " << inputSize() << ", m_inputs: " << m_inputPorts.size();

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
        outPort->setName(m_icOutputLabels.at(outputIndex));
    }

    qCDebug(three) << "IC " << m_file << " -> Outputs. min: " << minOutputSize() << ", max: " << maxOutputSize() << ", current: " << outputSize() << ", m_outputs: " << m_outputPorts.size();
}

void IC::loadFile(const QString &fileName)
{
    qCDebug(zero) << "Reading IC.";

    m_icInputs.clear();
    m_icOutputs.clear();
    setInputSize(0);
    setOutputSize(0);
    qDeleteAll(m_icElements);
    m_icElements.clear();

    // ----------------------------------------------

    QFileInfo fileInfo;
    fileInfo.setFile(GlobalProperties::currentDir, QFileInfo(fileName).fileName());

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        throw Pandaception(fileInfo.absoluteFilePath() + tr(" not found."));
    }

    m_fileWatcher.addPath(fileInfo.absoluteFilePath());
    m_file = fileInfo.absoluteFilePath();
    setToolTip(fileInfo.fileName());

    // ----------------------------------------------

    QFile file(fileInfo.absoluteFilePath());

    if (!file.open(QIODevice::ReadOnly)) {
        throw Pandaception(QObject::tr("Error opening file: ") + file.errorString());
    }

    QDataStream stream(&file);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    Serialization::loadDolphinFileName(stream, version);
    Serialization::loadRect(stream, version);

    const auto items = Serialization::deserialize(stream, {}, version);

    for (auto *item : items) {
        if (item->type() != GraphicElement::Type) {
            continue;
        }

        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);

        switch (elm->elementGroup()) {
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
    loadInputs();
    loadOutputs();

    // ----------------------------------------------

    if (label().isEmpty()) {
        setLabel(fileInfo.baseName().toUpper());
    }

    const qreal bottom = portsBoundingRect().united(QRectF(0, 0, 64, 64)).bottom();
    m_label->setPos(30, bottom + 5);

    generatePixmap();

    qCDebug(zero) << "Finished reading IC.";
}

void IC::generatePixmap()
{
    // make pixmap
    const QSize size = portsBoundingRect().united(QRectF(0, 0, 64, 64)).size().toSize();
    QPixmap tempPixmap(size);
    tempPixmap.fill(Qt::transparent);

    QPainter tmpPainter(&tempPixmap);

    tmpPainter.setBrush(QColor(126, 126, 126));
    tmpPainter.setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));

    // draw package
    QPoint topLeft = tempPixmap.rect().topLeft();
    topLeft.setX(topLeft.x() + 7);
    QSize finalSize = tempPixmap.rect().size();
    finalSize.setWidth(finalSize.width() - 14);
    QRectF finalRect = QRectF(topLeft, finalSize);
    tmpPainter.drawRoundedRect(finalRect, 3, 3);

    QPixmap panda(":/basic/ic-panda2.svg");
    QPointF pandaOrigin = finalRect.center();
    pandaOrigin.setX(pandaOrigin.x() - panda.width() / 2);
    pandaOrigin.setY(pandaOrigin.y() - panda.height() / 2);
    tmpPainter.drawPixmap(pandaOrigin, panda);

    // draw shadow
    tmpPainter.setBrush(QColor(78, 78, 78));
    tmpPainter.setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));

    QRectF shadowRect(finalRect.bottomLeft(), finalRect.bottomRight());
    shadowRect.adjust(0, -3, 0, 0);
    tmpPainter.drawRoundedRect(shadowRect, 3, 3);

    // draw semicircle
    QRectF topCenter = QRectF(finalRect.topLeft() + QPointF(18, -12), QSize(24, 24));
    tmpPainter.drawChord(topCenter, 0, -180 * 16);

    m_pixmap = tempPixmap;
}

void IC::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    qApp->mainWindow()->loadPandaFile(m_file);
}

void IC::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    if (isSelected()) {
        painter->save();
        painter->setBrush(m_selectionBrush);
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        painter->drawRoundedRect(portsBoundingRect().united(QRectF(0, 0, 64, 64)), 5, 5);
        painter->restore();
    }

    painter->drawPixmap(boundingRect().topLeft(), pixmap());
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
            conn->setStartPort(nodeElm->outputPort());
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
            conn->setEndPort(nodeElm->inputPort());
        }
    }

    delete elm;
}

bool IC::comparePorts(QNEPort *port1, QNEPort *port2)
{
    QPointF p1 = port1->graphicElement()->pos();
    QPointF p2 = port2->graphicElement()->pos();

    if (p1 != p2) {
        return (p1.y() < p2.y()) || (qFuzzyCompare(p1.y(), p2.y()) && (p1.x() < p2.x()));
    }

    p1 = port1->pos();
    p2 = port2->pos();

    return (p1.x() < p2.x()) || (qFuzzyCompare(p1.x(), p2.x()) && (p1.y() < p2.y()));
}

void IC::sortPorts(QVector<QNEPort *> &map)
{
    std::stable_sort(map.begin(), map.end(), comparePorts);
}

LogicElement *IC::inputLogic(const int index)
{
    return m_icInputs.at(index)->logic();
}

LogicElement *IC::outputLogic(const int index)
{
    return m_icOutputs.at(index)->logic();
}

void IC::loadInputsLabels()
{
    for (int portIndex = 0; portIndex < m_icInputs.size(); ++portIndex) {
        auto *inputPort = m_icInputs.at(portIndex);
        auto *elm = inputPort->graphicElement();
        QString lb = elm->label();

        if (!inputPort->name().isEmpty()) {
            lb += " ";
            lb += inputPort->name();
        }

        if (!elm->genericProperties().isEmpty()) {
            lb += " [" + elm->genericProperties() + "]";
        }

        m_icInputLabels[portIndex] = lb;
    }
}

void IC::loadOutputsLabels()
{
    for (int portIndex = 0; portIndex < m_icOutputs.size(); ++portIndex) {
        auto *outputPort = m_icOutputs.at(portIndex);
        auto *elm = outputPort->graphicElement();
        QString label = elm->label();

        if (!outputPort->name().isEmpty()) {
            label += " ";
            label += outputPort->name();
        }

        if (!elm->genericProperties().isEmpty()) {
            label += " [" + elm->genericProperties() + "]";
        }

        m_icOutputLabels[portIndex] = label;
    }
}

const QVector<std::shared_ptr<LogicElement>> IC::generateMap()
{
    mapping = new ElementMapping(m_icElements);
    return mapping->logicElms();
}

void IC::refresh()
{
}

void IC::copyFiles(const QFileInfo &srcPath, const QFileInfo &destPath)
{
    IC::needToCopyFiles = true;

    QFile destFile;

    if (!QFile::exists(destPath.absoluteFilePath()) && !destFile.copy(srcPath.absoluteFilePath(), destPath.absoluteFilePath())) {
        throw Pandaception(tr("Error copying file: ") + destFile.errorString());
    }

    destFile.setFileName(destPath.absoluteFilePath());

    if (!destFile.open(QIODevice::ReadOnly)) {
        throw Pandaception(QObject::tr("Error opening file: ") + destFile.errorString());
    }

    QDataStream stream(&destFile);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    Serialization::loadDolphinFileName(stream, version);
    Serialization::loadRect(stream, version);

    IC::srcPath_ = srcPath.absolutePath();
    IC::destPath_ = destPath.absolutePath();
    Serialization::deserialize(stream, {}, version);

    IC::needToCopyFiles = false;
    IC::srcPath_.clear();
    IC::destPath_.clear();
}
