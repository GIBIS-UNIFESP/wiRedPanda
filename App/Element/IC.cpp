// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/IC.h"

#include <QDir>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QScopeGuard>
#include <QSet>
#include <QStyleOptionGraphicsItem>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/GlobalProperties.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"

IC::IC(QGraphicsItem *parent)
    : GraphicElement(ElementType::IC, ElementGroup::IC, "", tr("INTEGRATED CIRCUIT"), tr("IC"), 0, 0, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    // ICs display their label vertically alongside the chip body, matching the physical
    // convention of reading IC labels along the side of a physical DIP package
    m_label->setRotation(90);

    setHasLabel(true);

    // Hot-reload: when the .panda file backing this IC changes on disk (e.g. the user saved
    // an edited sub-circuit), reload the IC definition and restart the simulation so the
    // parent circuit immediately reflects the updated internals
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &filePath) {
        loadFile(filePath);

        if (auto *scene_ = qobject_cast<Scene *>(scene())) {
            scene_->simulation()->restart();
        }
    });
}

void IC::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    // Store only the base filename, not the full path, so that .panda files remain
    // portable — the context directory is added back at load time
    map.insert("fileName", QFileInfo(m_file).fileName());

    stream << map;
}

void IC::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    // Old format (V_1_2 to V_4_1): IC file path was written as a plain QString
    if ((VERSION("1.2") <= version) && (version < VERSION("4.1"))) {
        stream >> m_file;

        // For tests with old files containing absolute paths, strip to filename only
        if (GlobalProperties::testMode) {
            m_file = QFileInfo(m_file).fileName();
        }

        if (IC::needToCopyFiles) {
            copyFile();
        }

        loadFile(m_file);
    }

    // New format (V_4_1+): IC data stored in a QMap for extensibility
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
        throw PANDACEPTION("Error copying file: %1", destFile.errorString());
    }
}

void IC::loadInputs()
{
    // Lock port count to exactly the number of inputs found in the sub-circuit file;
    // min == max == actual count prevents the user from adding/removing IC input ports
    setMaxInputSize(static_cast<int>(m_icInputs.size()));
    setMinInputSize(static_cast<int>(m_icInputs.size()));
    setInputSize(static_cast<int>(m_icInputs.size()));
    qCDebug(three) << "IC " << m_file << " -> Inputs. min: " << minInputSize() << ", max: " << maxInputSize() << ", current: " << inputSize() << ", m_inputs: " << m_inputPorts.size();

    // Mirror the required/default-status from the sub-circuit's input elements so that
    // unconnected optional inputs (e.g. enable lines) don't flag the IC as invalid
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
    // Same port-count locking as loadInputs; outputs are always driven so
    // required/defaultStatus mirroring is not needed here
    setMaxOutputSize(static_cast<int>(m_icOutputs.size()));
    setMinOutputSize(static_cast<int>(m_icOutputs.size()));
    setOutputSize(static_cast<int>(m_icOutputs.size()));

    for (int outputIndex = 0; outputIndex < m_icOutputs.size(); ++outputIndex) {
        auto *outPort = outputPort(outputIndex);
        outPort->setName(m_icOutputLabels.at(outputIndex));
    }

    qCDebug(three) << "IC " << m_file << " -> Outputs. min: " << minOutputSize() << ", max: " << maxOutputSize() << ", current: " << outputSize() << ", m_outputs: " << m_outputPorts.size();
}

/// Files currently being loaded (cycle detection for circular IC references).
static QSet<QString> s_loadingFiles;

void IC::loadFile(const QString &fileName)
{
    qCDebug(zero) << "Reading IC.";

    // Reset all previously loaded state so a hot-reload starts from a clean slate
    m_icInputs.clear();
    m_icOutputs.clear();
    setInputSize(0);
    setOutputSize(0);
    qDeleteAll(m_icElements);
    m_icElements.clear();

    // --- Resolve file path ---

    QFileInfo fileInfo;

    // Always combine with currentDir for cross-platform compatibility.
    // QFileInfo::isAbsolute() is not cross-platform: it only recognizes paths as absolute
    // on the current OS. A Linux system cannot recognize Windows paths as absolute, causing
    // incorrect resolution when loading files saved on a different platform. Since filenames
    // are extracted from old files regardless of their original OS, always combine with currentDir.
    fileInfo.setFile(QDir(GlobalProperties::currentDir), fileName);

    if (!fileInfo.exists() || !fileInfo.isFile()) {
        throw PANDACEPTION("%1 not found.", fileInfo.absoluteFilePath());
    }

    // Cycle detection: if this file is already being loaded up the call stack,
    // a circular IC reference exists (A→B→A→…). Throw instead of stack-overflowing.
    const QString canonicalPath = fileInfo.canonicalFilePath();
    if (s_loadingFiles.contains(canonicalPath)) {
        throw PANDACEPTION("Circular IC reference detected: %1", canonicalPath);
    }
    s_loadingFiles.insert(canonicalPath);
    auto removeGuard = qScopeGuard([&] { s_loadingFiles.remove(canonicalPath); });

    // Register with the file watcher after resolution so the absolute path is used;
    // the watcher callback will trigger a reload if the file changes on disk
    m_fileWatcher.addPath(fileInfo.absoluteFilePath());
    m_file = fileInfo.absoluteFilePath();
    setToolTip(fileInfo.fileName());

    // --- Deserialize sub-circuit elements ---

    QFile file(fileInfo.absoluteFilePath());

    if (!file.open(QIODevice::ReadOnly)) {
        throw PANDACEPTION("Error opening file: %1", file.errorString());
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
        if (!elm) {
            continue;
        }

        // Input/Output elements become the IC's external ports; everything else is internal logic
        switch (elm->elementGroup()) {
        case ElementGroup::Input:  loadInputElement(elm);    break;
        case ElementGroup::Output: loadOutputElement(elm);   break;
        default:                   m_icElements.append(elm); break;
        }
    }

    file.close();

    // --- Build sorted, labelled port lists ---

    m_icInputLabels = QVector<QString>(m_icInputs.size());
    m_icOutputLabels = QVector<QString>(m_icOutputs.size());
    // Sort top-to-bottom by Y position so port order on the IC body matches visual layout
    sortPorts(m_icInputs);
    sortPorts(m_icOutputs);
    loadInputsLabels();
    loadOutputsLabels();
    loadInputs();
    loadOutputs();

    // --- Update visual representation ---

    // Default label: uppercased filename without extension (e.g. "adder.panda" → "ADDER")
    if (label().isEmpty()) {
        setLabel(fileInfo.baseName().toUpper());
    }

    // Position label just below the IC body, which grows with port count
    const qreal bottom = portsBoundingRect().united(QRectF(0, 0, 64, 64)).bottom();
    m_label->setPos(30, bottom + 5);

    generatePixmap();

    qCDebug(zero) << "Finished reading IC.";
}

void IC::generatePixmap()
{
    // The pixmap must encompass both the 64×64 body and any ports that extend beyond it
    const QSize size = portsBoundingRect().united(QRectF(0, 0, 64, 64)).size().toSize();
    QPixmap tempPixmap(size);
    tempPixmap.fill(Qt::transparent);

    QPainter tmpPainter(&tempPixmap);

    // IC body: mid-grey fill with darker border, styled like a physical DIP package
    tmpPainter.setBrush(QColor(126, 126, 126));
    tmpPainter.setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));

    // 7px inset on each side (14px total width reduction) so the port connector dots
    // visually overlap the border, matching the TruthTable and physical DIP appearance
    QPoint topLeft = tempPixmap.rect().topLeft();
    topLeft.setX(topLeft.x() + 7);
    QSize finalSize = tempPixmap.rect().size();
    finalSize.setWidth(finalSize.width() - 14);
    QRectF finalRect = QRectF(topLeft, finalSize);
    tmpPainter.drawRoundedRect(finalRect, 3, 3);

    // Centre the wiRedPanda mascot logo on the IC body
    QPixmap panda(":/Components/Logic/ic-panda2.svg");
    QPointF pandaOrigin = finalRect.center();
    pandaOrigin.setX(pandaOrigin.x() - panda.width() / 2);
    pandaOrigin.setY(pandaOrigin.y() - panda.height() / 2);
    tmpPainter.drawPixmap(pandaOrigin, panda);

    // Draw a thin dark strip at the bottom edge to simulate the package shadow/bevel
    tmpPainter.setBrush(QColor(78, 78, 78));
    tmpPainter.setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));

    // Collapse the two-point rect to a 3px-tall strip flush with the bottom of the body
    QRectF shadowRect(finalRect.bottomLeft(), finalRect.bottomRight());
    shadowRect.adjust(0, -3, 0, 0);
    tmpPainter.drawRoundedRect(shadowRect, 3, 3);

    // Draw the orientation notch (semicircle) at the top centre, matching the physical
    // DIP IC convention for pin-1 identification.
    // drawChord angle parameters are in 1/16th-degree units; -180*16 = lower half-circle.
    // The 24×24 circle is offset left by (body_width/2 - 12) = 18px from the body's
    // top-left corner, and raised 12px above the top edge so only the bottom half shows.
    QRectF topCenter = QRectF(finalRect.topLeft() + QPointF(18, -12), QSize(24, 24));
    tmpPainter.drawChord(topCenter, 0, -180 * 16);

    m_pixmap = tempPixmap;
}

void IC::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    Application::instance()->mainWindow()->loadPandaFile(m_file);
}

QRectF IC::boundingRect() const
{
    return portsBoundingRect().united(QRectF(0, 0, 64, 64));
}

void IC::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    if (isSelected()) {
        painter->save();
        painter->setBrush(m_selectionBrush);
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        painter->drawRoundedRect(boundingRect(), 5, 5);
        painter->restore();
    }

    painter->drawPixmap(boundingRect().topLeft(), pixmap());
}

void IC::loadInputElement(GraphicElement *elm)
{
    // Each output port of an input element (Switch, Button, Clock, …) becomes one
    // external input pin on the IC.  A proxy Node element is inserted as a bridge:
    //   [IC external input] → Node.input → Node.output → [internal sub-circuit wires]
    // This lets the parent circuit drive the Node's input, which in turn propagates into
    // the sub-circuit without exposing the original input element.
    for (auto *outputPort : elm->outputs()) {
        auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
        nodeElm->setPos(elm->pos());
        nodeElm->setLabel(elm->label().isEmpty() ?
                              ElementFactory::typeToText(elm->elementType())
                            : elm->label());

        auto *nodeInput = nodeElm->inputPort();
        if (elm->outputSize() > 1) {
            nodeInput->setName(outputPort->name());
        }
        // Clock inputs are required (the IC won't function without a clock signal)
        nodeInput->setRequired(elm->elementType() == ElementType::Clock);
        nodeInput->setDefaultStatus(outputPort->status());
        nodeInput->setStatus(outputPort->status());

        m_icInputs.append(nodeInput);
        m_icElements.append(nodeElm);

        // Re-route connections that previously started from the original input element
        // so they now originate from the proxy Node's output port
        const auto conns = outputPort->connections();

        for (auto *conn : conns) {
            conn->setStartPort(nodeElm->outputPort());
        }
    }

    // The original input element is no longer needed — the Node proxy replaces it
    delete elm;
}

void IC::loadOutputElement(GraphicElement *elm)
{
    // Mirror of loadInputElement: each input port of an output element (LED, Display, …) becomes
    // one external output pin on the IC, again bridged through a proxy Node element:
    //   [internal sub-circuit wires] → Node.input → Node.output → [IC external output]
    for (auto *inputPort : elm->inputs()) {
        auto *nodeElm = ElementFactory::buildElement(ElementType::Node);
        nodeElm->setPos(elm->pos());
        nodeElm->setLabel(elm->label().isEmpty() ?
                              ElementFactory::typeToText(elm->elementType())
                            : elm->label());

        auto *nodeOutput = nodeElm->outputPort();
        if (elm->inputSize() > 1) {
            nodeOutput->setName(inputPort->name());
        }

        m_icOutputs.append(nodeOutput);
        m_icElements.append(nodeElm);

        // Re-route connections that previously ended at the original output element
        // so they now terminate at the proxy Node's input port
        for (auto *conn : inputPort->connections()) {
            conn->setEndPort(nodeElm->inputPort());
        }
    }

    delete elm;
}

bool IC::comparePorts(QNEPort *port1, QNEPort *port2)
{
    auto *elem1 = port1->graphicElement();
    auto *elem2 = port2->graphicElement();
    if (!elem1 || !elem2) {
        return false;
    }
    QPointF p1 = elem1->pos();
    QPointF p2 = elem2->pos();

    // Primary sort: top-to-bottom by parent element Y, then left-to-right by X.
    // This gives an intuitive pin order that matches the visual layout in the sub-circuit.
    if (p1 != p2) {
        return (p1.y() < p2.y()) || (qFuzzyCompare(p1.y(), p2.y()) && (p1.x() < p2.x()));
    }

    // Secondary sort: when two ports share the same parent element position, sort by
    // the port's own local coordinates (left-to-right, top-to-bottom)
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

        // Append generic properties (e.g. clock frequency) in brackets so the IC pin tooltip
        // carries enough context for the user to identify the signal without opening the sub-circuit
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
    // Build a fresh ElementMapping each time the IC is wired into the simulation;
    // the previous mapping is automatically released by the unique_ptr assignment
    m_mapping = std::make_unique<ElementMapping>(m_icElements);
    return m_mapping->logicElms();
}

void IC::refresh()
{
}

void IC::copyFiles(const QFileInfo &srcPath, const QFileInfo &destPath)
{
    IC::needToCopyFiles = true;

    QFile destFile;

    // Skip the copy if the destination already exists (e.g. two ICs reference the same file)
    if (!QFile::exists(destPath.absoluteFilePath()) && !destFile.copy(srcPath.absoluteFilePath(), destPath.absoluteFilePath())) {
        throw PANDACEPTION("Error copying file: %1", destFile.errorString());
    }

    destFile.setFileName(destPath.absoluteFilePath());

    if (!destFile.open(QIODevice::ReadOnly)) {
        throw PANDACEPTION("Error opening file: %1", destFile.errorString());
    }

    // Deserialize the copied file so that any nested ICs it references also get
    // copied transitively into the destination directory
    QDataStream stream(&destFile);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    Serialization::loadDolphinFileName(stream, version);
    Serialization::loadRect(stream, version);

    // Set the global src/dest paths before deserialization so that any IC elements
    // encountered while parsing can call copyFile() with the correct directories
    IC::srcPath_ = srcPath.absolutePath();
    IC::destPath_ = destPath.absolutePath();
    Serialization::deserialize(stream, {}, version);

    // Clear after deserialization to prevent stale paths from affecting later operations
    IC::needToCopyFiles = false;
    IC::srcPath_.clear();
    IC::destPath_.clear();
}

LogicElement *IC::getInputLogic(int portIndex) const
{
    return const_cast<IC *>(this)->inputLogic(portIndex);
}

LogicElement *IC::getOutputLogic(int portIndex) const
{
    return const_cast<IC *>(this)->outputLogic(portIndex);
}

int IC::getInputIndexForPort(int portIndex) const
{
    Q_UNUSED(portIndex)
    // Each IC input is represented by a single-output Node element; the driven port
    // is always the first (and only) output, so the index into the logic element is always 0
    return 0;
}

int IC::getOutputIndexForPort(int portIndex) const
{
    Q_UNUSED(portIndex)
    // Same reasoning as getInputIndexForPort: each IC output is a Node with one output port
    return 0;
}

QVector<std::shared_ptr<LogicElement>> IC::getLogicElementsForMapping()
{
    return generateMap();
}

bool IC::canSetPortNames() const
{
    return true;
}

void IC::setInputPortName(int port, const QString &name)
{
    if (port < m_icInputs.size()) {
        m_icInputs.value(port)->setName(name);
    }
}

void IC::setOutputPortName(int port, const QString &name)
{
    if (port < m_icOutputs.size()) {
        m_icOutputs.value(port)->setName(name);
    }
}

void IC::loadFromDrop(const QString &fileName)
{
    loadFile(fileName);
}

