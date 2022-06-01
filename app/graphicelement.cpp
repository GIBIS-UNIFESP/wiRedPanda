// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "graphicelement.h"

#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "thememanager.h"

#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <iostream>
#include <stdexcept>

namespace
{
int id = qRegisterMetaType<GraphicElement>();
}

GraphicElement::GraphicElement(ElementType type, ElementGroup group, const int minInputSize, const int maxInputSize, const int minOutputSize, const int maxOutputSize, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_elementGroup(group)
    , m_elementType(type)
    , m_maxInputSize(maxInputSize)
    , m_maxOutputSize(maxOutputSize)
    , m_minInputSize(minInputSize)
    , m_minOutputSize(minOutputSize)
{
    qCDebug(four) << "Setting flags of elements.";
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);

    qCDebug(four) << "Setting attributes.";
    m_label->hide();
    QFont font("SansSerif");
    font.setBold(true);
    m_label->setFont(font);
    m_label->setPos(0, 64);
    m_label->setParentItem(this);
    m_label->setDefaultTextColor(Qt::black);

    qCDebug(four) << "Including input and output ports.";
    for (int i = 0; i < minInputSize; ++i) {
        addInputPort();
    }
    for (int i = 0; i < minOutputSize; ++i) {
        addOutputPort();
    }
    GraphicElement::updatePorts();
    GraphicElement::updateTheme();
}

QPixmap GraphicElement::pixmap() const
{
    return m_pixmap ? *m_pixmap : QPixmap();
}

ElementType GraphicElement::elementType() const
{
    return m_elementType;
}

ElementGroup GraphicElement::elementGroup() const
{
    return m_elementGroup;
}

void GraphicElement::disable()
{
    m_disabled = true;
}

void GraphicElement::enable()
{
    m_disabled = false;
}

void GraphicElement::setPixmap(const QString &pixmapPath)
{
    if (pixmapPath.isEmpty()) {
        return;
    }

    if (pixmapPath == m_currentPixmapPath) {
        return;
    }

    if (!m_pixmap) {
        m_pixmap = new QPixmap();
    }

    if (!m_pixmap->load(pixmapPath)) {
        m_pixmap->load(m_defaultSkins.first());
        qCDebug(zero) << "Problem loading pixmapPath:" << pixmapPath;
        throw Pandaception(tr("Couldn't load pixmap."));
    }

    m_pixmap = new QPixmap(m_pixmap->scaled(64, 64));
    setTransformOriginPoint(m_pixmap->rect().center());
    update(GraphicElement::boundingRect());

    m_currentPixmapPath = pixmapPath;
}

void GraphicElement::setPixmap(const QString &pixmapPath, const QSize size)
{
    if (pixmapPath == m_currentPixmapPath) {
        return;
    }

    if (!m_pixmap) {
        m_pixmap = new QPixmap();
    }

    if (!m_pixmap->load(pixmapPath)) {
        m_pixmap->load(m_defaultSkins.first());
        qCDebug(zero) << "Problem loading pixmapPath:" << pixmapPath;
        throw Pandaception(tr("Couldn't load pixmap."));
    }

    m_pixmap = new QPixmap(m_pixmap->scaled(size));
    setTransformOriginPoint(m_pixmap->rect().center());
    update(boundingRect());

    m_currentPixmapPath = pixmapPath;
}

QVector<QNEOutputPort *> GraphicElement::outputs() const
{
    return m_outputs;
}

const QNEInputPort *GraphicElement::input(const int pos) const
{
    return m_inputs.at(pos);
}

const QNEOutputPort *GraphicElement::output(const int pos) const
{
    return m_outputs.at(pos);
}

QNEInputPort *GraphicElement::input(const int pos)
{
    return m_inputs.at(pos);
}

QNEOutputPort *GraphicElement::output(const int pos)
{
    return m_outputs.at(pos);
}

void GraphicElement::setOutputs(const QVector<QNEOutputPort *> &outputs)
{
    m_outputs = outputs;
}

void GraphicElement::save(QDataStream &stream) const
{
    qCDebug(four) << "Saving element. Type:" << objectName();
    stream << pos();
    stream << rotation();
    /* <Version1.2> */
    stream << label();
    /* <\Version1.2> */
    /* <Version1.3> */
    stream << m_minInputSize;
    stream << m_maxInputSize;
    stream << m_minOutputSize;
    stream << m_maxOutputSize;
    /* <\Version1.3> */
    /* <Version1.9> */
    stream << m_trigger;
    /* <\Version1.9> */
    stream << static_cast<quint64>(m_inputs.size());
    for (auto *port : m_inputs) {
        stream << reinterpret_cast<quint64>(port);
        stream << port->portName();
        stream << port->portFlags();
    }
    stream << static_cast<quint64>(m_outputs.size());
    for (auto *port : m_outputs) {
        stream << reinterpret_cast<quint64>(port);
        stream << port->portName();
        stream << port->portFlags();
    }
    /* <\Version2.7> */
    stream << static_cast<quint64>(m_defaultSkins.size());
    for (const QString &skinName : m_defaultSkins) {
        stream << skinName;
    }
    qCDebug(four) << "Finished saving element.";
}

void GraphicElement::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    qCDebug(four) << "Loading element. Type:" << objectName();
    loadPos(stream);
    loadAngle(stream);
    /* <Version1.2> */
    loadLabel(stream, version);
    /* <\Version1.2> */
    /* <Version1.3> */
    loadMinMax(stream, version);
    /* <\Version1.3> */
    /* <Version1.9> */
    loadTrigger(stream, version);
    /* <\Version1.9> */
    loadInputPorts(stream, portMap);
    loadOutputPorts(stream, portMap);
    /* <\Version2.7> */
    loadPixmapSkinNames(stream, version);
    qCDebug(four) << "Updating port positions.";
    updatePorts();
    qCDebug(four) << "Finished loading element.";
}

void GraphicElement::loadPos(QDataStream &stream)
{
    QPointF pos;
    stream >> pos;
    setPos(pos);
}

void GraphicElement::loadAngle(QDataStream &stream)
{
    qreal angle;
    stream >> angle;
    setRotation(angle);
}

void GraphicElement::loadLabel(QDataStream &stream, const double version)
{
    if (version >= 1.2) {
        QString labelText;
        stream >> labelText;
        setLabel(labelText);
    }
}

void GraphicElement::loadMinMax(QDataStream &stream, const double version)
{
    if (version >= 1.3) {
        quint64 minInputSize;
        quint64 maxInputSize;
        quint64 minOutputSize;
        quint64 maxOutputSize;
        stream >> minInputSize;
        stream >> maxInputSize;
        stream >> minOutputSize;
        stream >> maxOutputSize;
        // FIXME: Was it a bad decision to store Min and Max input/output sizes?
        /* Version 2.2 ?? fix ?? */
        if (!((m_minInputSize == m_maxInputSize) && (m_minInputSize > maxInputSize))) {
            m_minInputSize = minInputSize;
            m_maxInputSize = maxInputSize;
        }
        if (!((m_minOutputSize == m_maxOutputSize) && (m_minOutputSize > maxOutputSize))) {
            m_minOutputSize = minOutputSize;
            m_maxOutputSize = maxOutputSize;
        }
    }
}

void GraphicElement::loadTrigger(QDataStream &stream, const double version)
{
    if (version >= 1.9) {
        QKeySequence trigger;
        stream >> trigger;
        setTrigger(trigger);
    }
}

void GraphicElement::loadInputPorts(QDataStream &stream, QMap<quint64, QNEPort *> &portMap)
{
    qCDebug(four) << "Loading input ports.";
    quint64 inputSize;
    stream >> inputSize;
    if (inputSize > maximumValidInputSize) {
        throw Pandaception(tr("Corrupted DataStream!"));
    }
    for (size_t port = 0; port < inputSize; ++port) {
        loadInputPort(stream, portMap, static_cast<int>(port));
    }
    removeSurplusInputs(inputSize, portMap);
}

void GraphicElement::loadInputPort(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const int port)
{
    QString name;
    int flags;
    quint64 ptr;
    stream >> ptr;
    stream >> name;
    stream >> flags;
    if (port < m_inputs.size()) {
        if (elementType() == ElementType::IC) {
            m_inputs[port]->setName(name);
        }
        m_inputs[port]->setPortFlags(flags);
        m_inputs[port]->setPtr(ptr);
    } else {
        addPort(name, false, flags, static_cast<int>(ptr));
    }
    portMap[ptr] = m_inputs[port];
}

void GraphicElement::removeSurplusInputs(const quint64 inputSize_, QMap<quint64, QNEPort *> &portMap)
{
    while (inputSize() > static_cast<int>(inputSize_) && inputSize_ >= m_minInputSize) {
        QNEPort *deletedPort = m_inputs.last();
        removePortFromMap(deletedPort, portMap);
        delete deletedPort;
        m_inputs.removeLast();
    }

    updatePorts();
}

void GraphicElement::removeSurplusOutputs(const quint64 outputSize_, QMap<quint64, QNEPort *> &portMap)
{
    while (outputSize() > static_cast<int>(outputSize_) && outputSize_ >= m_minOutputSize) {
        QNEPort *deletedPort = m_outputs.last();
        removePortFromMap(deletedPort, portMap);
        delete deletedPort;
        m_outputs.removeLast();
    }

    updatePorts();
}

void GraphicElement::removePortFromMap(QNEPort *deletedPort, QMap<quint64, QNEPort *> &portMap)
{
    for (auto it = portMap.begin(); it != portMap.end();) {
        if (it.value() == deletedPort) {
            it = portMap.erase(it);
        } else {
            ++it;
        }
    }
}

void GraphicElement::loadOutputPorts(QDataStream &stream, QMap<quint64, QNEPort *> &portMap)
{
    qCDebug(four) << "Loading output ports.";
    quint64 outputSize;
    stream >> outputSize;
    if (outputSize > maximumValidInputSize) {
        throw Pandaception(tr("Corrupted DataStream!"));
    }
    for (size_t port = 0; port < outputSize; ++port) {
        loadOutputPort(stream, portMap, static_cast<int>(port));
    }
    removeSurplusOutputs(outputSize, portMap);
}

void GraphicElement::loadOutputPort(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const int port)
{
    QString name;
    int flags;
    quint64 ptr;
    stream >> ptr;
    stream >> name;
    stream >> flags;
    if (port < m_outputs.size()) {
        if (elementType() == ElementType::IC) {
            m_outputs[port]->setName(name);
        }
        m_outputs[port]->setPortFlags(flags);
        m_outputs[port]->setPtr(ptr);
    } else {
        addPort(name, true, flags, static_cast<int>(ptr));
    }
    portMap[ptr] = m_outputs[port];
}

void GraphicElement::loadPixmapSkinNames(QDataStream &stream, const double version)
{
    if (version >= 2.7) {
        qCDebug(four) << "Loading pixmap skin names.";
        quint64 outputSize;
        stream >> outputSize;
        if (outputSize > maximumValidInputSize) {
            throw Pandaception(tr("Corrupted DataStream!"));
        }
        for (size_t skin = 0; skin < outputSize; ++skin) {
            loadPixmapSkinName(stream, static_cast<int>(skin));
        }
        refresh();
    }
}

void GraphicElement::loadPixmapSkinName(QDataStream &stream, const int skin)
{
    QString name;
    stream >> name;

    if (skin >= m_defaultSkins.size()) {
        qCDebug(zero) << "Could not load some of the skins.";
    }

    if (!name.startsWith(":/")) {
        QDir dir(QFileInfo(GlobalProperties::currentFile).absoluteDir());
        QString fileName(QFileInfo(name).fileName());
        QFileInfo fileInfo(dir, fileName);
        qCDebug(zero) << "Skin fileName:" << fileInfo.absoluteFilePath();
        if (!fileInfo.isFile()) {
            qCDebug(zero) << "Could not load some of the skins.";
        }
        m_defaultSkins[skin] = fileInfo.absoluteFilePath();
    }
}

QVector<QNEInputPort *> GraphicElement::inputs() const
{
    return m_inputs;
}

void GraphicElement::setInputs(const QVector<QNEInputPort *> &inputs)
{
    m_inputs = inputs;
}

QRectF GraphicElement::boundingRect() const
{
    return m_pixmap->rect();
}

void GraphicElement::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    painter->setClipRect(option->exposedRect);
    if (isSelected()) {
        painter->setBrush(m_selectionBrush);
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        painter->drawRoundedRect(boundingRect(), 5, 5);
    }
    painter->drawPixmap(QPoint(0, 0), pixmap());
}

void GraphicElement::addPort(const QString &name, const bool isOutput, const int flags, const int ptr)
{
    qCDebug(four) << "Adding new port.";
    if (isOutput && (static_cast<quint64>(m_outputs.size()) >= m_maxOutputSize)) {
        return;
    }
    if (!isOutput && (static_cast<quint64>(m_inputs.size()) >= m_maxInputSize)) {
        return;
    }
    QNEPort *port = nullptr;
    if (isOutput) {
        m_outputs.push_back(new QNEOutputPort(this));
        port = m_outputs.last();
        port->setIndex(outputSize() - 1);
    } else {
        m_inputs.push_back(new QNEInputPort(this));
        port = m_inputs.last();
        port->setIndex(inputSize() - 1);
    }
    port->setName(name);
    port->setGraphicElement(this);
    port->setPortFlags(flags);
    port->setPtr(ptr);
    port->show();
}

void GraphicElement::addInputPort(const QString &name)
{
    addPort(name, false);
}

void GraphicElement::addOutputPort(const QString &name)
{
    addPort(name, true);
}

void GraphicElement::setPortName(const QString &name)
{
    setObjectName(name);
    // setToolTip(name);
}

void GraphicElement::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_usingDefaultSkin = defaultSkin;
    m_alternativeSkins.first() = fileName;
    setPixmap(defaultSkin ? m_defaultSkins.first() : m_alternativeSkins.first());
}

void GraphicElement::updatePorts()
{
    qCDebug(five) << "Updating port positions that belong to the IC.";
    const int inputPos = m_leftPosition;
    const int outputPos = m_rightPosition;

    if (!m_inputs.isEmpty()) {
        const int step = qMax(32 / m_inputs.size(), 6);
        int y = 32 - m_inputs.size() * step + step;
        for (auto *port : qAsConst(m_inputs)) {
            qCDebug(five) << "Setting input at" << inputPos << "," << y;
            port->setPos(inputPos, y);
            port->update();
            y += step * 2;
        }
    }

    if (!m_outputs.isEmpty()) {
        const int step = qMax(32 / m_outputs.size(), 6);
        int y = 32 - m_outputs.size() * step + step;
        for (auto *port : qAsConst(m_outputs)) {
            qCDebug(five) << "Setting output at" << outputPos << "," << y;
            port->setPos(outputPos, y);
            port->update();
            y += step * 2;
        }
    }
}

void GraphicElement::refresh()
{
    setPixmap(m_usingDefaultSkin ? m_defaultSkins[0] : m_alternativeSkins[0]);
}

QVariant GraphicElement::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    qCDebug(four) << "Align to grid.";
    if ((change == ItemPositionChange) && scene()) {
        QPointF newPos = value.toPointF();
        auto *customScene = qobject_cast<Scene *>(scene());
        if (customScene) {
            int gridSize = customScene->gridSize();
            qreal xV = qRound(newPos.x() / gridSize) * gridSize;
            qreal yV = qRound(newPos.y() / gridSize) * gridSize;
            return QPointF(xV, yV);
        }
        return newPos;
    }
    qCDebug(four) << "Moves wires.";
    if ((change == ItemScenePositionHasChanged) || (change == ItemRotationHasChanged) || (change == ItemTransformHasChanged)) {
        for (auto *port : qAsConst(m_outputs)) {
            port->updateConnections();
        }
        for (auto *port : qAsConst(m_inputs)) {
            port->updateConnections();
        }
    }
    update();

    return QGraphicsItem::itemChange(change, value);
}

bool GraphicElement::hasAudio() const
{
    return m_hasAudio;
}

void GraphicElement::setHasAudio(const bool hasAudio)
{
    m_hasAudio = hasAudio;
}

QKeySequence GraphicElement::trigger() const
{
    return m_trigger;
}

void GraphicElement::setTrigger(const QKeySequence &trigger)
{
    m_trigger = trigger;
    updateLabel();
}

QString GraphicElement::genericProperties()
{
    return {};
}

void GraphicElement::updateLabel()
{
    QString label = m_labelText;
    if ((!hasTrigger()) || (trigger().toString().isEmpty())) {
        m_label->setPlainText(label);
    } else {
        if (!label.isEmpty()) {
            label += " ";
        }
        m_label->setPlainText(label + QString("(%1)").arg(trigger().toString()));
    }
}

void GraphicElement::setLabel(const QString &label)
{
    m_labelText = label;
    updateLabel();
}

QString GraphicElement::label() const
{
    return m_labelText;
}

void GraphicElement::updateTheme()
{
    const ThemeAttributes theme = ThemeManager::attributes();

    m_label->setDefaultTextColor(theme.m_graphicElementLabelColor);
    m_selectionBrush = theme.m_selectionBrush;
    m_selectionPen = theme.m_selectionPen;
    for (auto *input : qAsConst(m_inputs)) {
        input->updateTheme();
    }
    for (auto *output : qAsConst(m_outputs)) {
        output->updateTheme();
    }
    update();
}

bool GraphicElement::isValid()
{
    qCDebug(four) << "Checking if the element has the required signals to compute its value.";
    bool valid = true;
    for (auto *input : qAsConst(m_inputs)) {
        /* Required inputs must have exactly one connection. */
        if (!input->isValid()) {
            valid = false;
            break;
        }
    }
    if (!valid) {
        for (auto *output : qAsConst(m_outputs)) {
            for (auto *conn : output->connections()) {
                conn->setStatus(QNEConnection::Status::Invalid);
                auto *port = conn->otherPort(output);
                if (port) {
                    port->setValue(-1);
                }
            }
        }
    }
    return valid;
}

bool GraphicElement::hasColors() const
{
    return m_hasColors;
}

bool GraphicElement::hasTrigger() const
{
    return m_hasTrigger;
}

void GraphicElement::setColor(const QString &color)
{
    Q_UNUSED(color);
}

QString GraphicElement::color() const
{
    return {};
}

void GraphicElement::setAudio(const QString &audio)
{
    Q_UNUSED(audio);
}

QString GraphicElement::audio() const
{
    return {};
}

void GraphicElement::setHasColors(const bool hasColors)
{
    m_hasColors = hasColors;
}

void GraphicElement::setCanChangeSkin(const bool canChangeSkin)
{
    m_canChangeSkin = canChangeSkin;
}

void GraphicElement::setHasTrigger(const bool hasTrigger)
{
    m_hasTrigger = hasTrigger;
}

bool GraphicElement::hasFrequency() const
{
    return m_hasFrequency;
}

void GraphicElement::setHasFrequency(const bool hasFrequency)
{
    m_hasFrequency = hasFrequency;
}

bool GraphicElement::hasLabel() const
{
    return m_hasLabel;
}

bool GraphicElement::canChangeSkin() const
{
    return m_canChangeSkin;
}

void GraphicElement::setHasLabel(const bool hasLabel)
{
    m_hasLabel = hasLabel;
    m_label->setVisible(hasLabel);
}

bool GraphicElement::isRotatable() const
{
    return m_rotatable;
}

void GraphicElement::setRotatable(const bool rotatable)
{
    m_rotatable = rotatable;
}

int GraphicElement::minOutputSize() const
{
    return static_cast<int>(m_minOutputSize);
}

int GraphicElement::inputSize() const
{
    return m_inputs.size();
}

void GraphicElement::setInputSize(const int size)
{
    if ((size >= minInputSize()) && (size <= maxInputSize())) {
        if (inputSize() < size) {
            while (inputSize() < size) {
                addInputPort();
            }
            updatePorts();
        } else {
            qDeleteAll(m_inputs.begin() + size, m_inputs.end());
            m_inputs.resize(size);
            updatePorts();
        }
    }
}

int GraphicElement::outputSize() const
{
    return m_outputs.size();
}

void GraphicElement::setOutputSize(const int size)
{
    if ((size >= minOutputSize()) && (size <= maxOutputSize())) {
        if (outputSize() < size) {
            for (int port = outputSize(); port < size; ++port) {
                addOutputPort();
            }
            updatePorts();
        } else {
            qDeleteAll(m_outputs.begin() + size, m_outputs.end());
            m_outputs.resize(size);
            updatePorts();
        }
    }
}

float GraphicElement::frequency() const
{
    return 0.0;
}

void GraphicElement::setFrequency(const float freq)
{
    Q_UNUSED(freq);
}

void GraphicElement::setMinOutputSize(const int minOutputSize)
{
    m_minOutputSize = minOutputSize;
}

int GraphicElement::minInputSize() const
{
    return static_cast<int>(m_minInputSize);
}

void GraphicElement::setMinInputSize(const int minInputSize)
{
    m_minInputSize = minInputSize;
}

int GraphicElement::maxOutputSize() const
{
    return static_cast<int>(m_maxOutputSize);
}

void GraphicElement::setMaxOutputSize(const int maxOutputSize)
{
    m_maxOutputSize = maxOutputSize;
}

int GraphicElement::maxInputSize() const
{
    return static_cast<int>(m_maxInputSize);
}

void GraphicElement::setMaxInputSize(const int maxInputSize)
{
    m_maxInputSize = maxInputSize;
}

int GraphicElement::rightPosition() const
{
    return m_rightPosition;
}

void GraphicElement::setRightPosition(const int rightPosition)
{
    m_rightPosition = rightPosition;
    updatePorts();
}

int GraphicElement::leftPosition() const
{
    return m_leftPosition;
}

void GraphicElement::setLeftPosition(const int leftPosition)
{
    m_leftPosition = leftPosition;
    updatePorts();
}

bool GraphicElement::isDisabled() const
{
    return m_disabled;
}

QDataStream &operator<<(QDataStream &stream, const GraphicElement *item)
{
    qDebug(four) << "Writing element.";
    const auto *elm = qgraphicsitem_cast<const GraphicElement *>(item);
    stream << GraphicElement::Type;
    stream << elm->elementType();
    elm->save(stream);
    return stream;
}
