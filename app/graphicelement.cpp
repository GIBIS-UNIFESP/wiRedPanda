// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include <stdexcept>

#include <qdir.h>
#include <QFileInfo>
#include <QKeyEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPixmap>

#include "common.h"
#include "graphicelement.h"
#include "globalproperties.h"
#include "nodes/qneconnection.h"
#include "nodes/qneport.h"
#include "scene.h"
#include "thememanager.h"

// TODO - WARNING: non-POD static
static QMap<QString, QPixmap> loadedPixmaps;

GraphicElement::GraphicElement(ElementType type, ElementGroup group, int minInputSz, int maxInputSz, int minOutputSz, int maxOutputSz, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_pixmap(nullptr)
    , m_label(new QGraphicsTextItem(this))
    , m_topPosition(0)
    , m_bottomPosition(64)
    , m_maxInputSz(maxInputSz)
    , m_maxOutputSz(maxOutputSz)
    , m_minInputSz(minInputSz)
    , m_minOutputSz(minOutputSz)
    , m_outputsOnTop(true)
    , m_rotatable(true)
    , m_hasLabel(false)
    , m_canChangeSkin(false)
    , m_hasFrequency(false)
    , m_hasColors(false)
    , m_hasTrigger(false)
    , m_hasAudio(false)
    , m_disabled(false)
    , m_elementType(type)
    , m_elementGroup(group)
{
    COMMENT("Setting flags of elements. ", 4);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);

    COMMENT("Setting attributes. ", 4);
    m_label->hide();
    QFont font("SansSerif");
    font.setBold(true);
    m_label->setFont(font);
    m_label->setPos(64, 30);
    m_label->setParentItem(this);
    m_label->setDefaultTextColor(Qt::black);

    COMMENT("Including input and output ports.", 4);
    for (int i = 0; i < minInputSz; i++) {
        addInputPort();
    }
    for (int i = 0; i < minOutputSz; i++) {
        addOutputPort();
    }
    updateTheme();
}

QPixmap GraphicElement::getPixmap() const {
    if (m_pixmap) {
        return *m_pixmap;
    }
    return {};
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

void GraphicElement::setPixmap(const QString &pixmapName)
{
    QString pixmapPath = pixmapName;
    if (ThemeManager::globalMngr) { // TODO: This has to be changed. Not a good way to do it. Probably better inside the class.
        if (pixmapPath.contains("memory")) {
            switch (ThemeManager::globalMngr->theme()) {
            case Theme::Panda_Light:
                pixmapPath.replace("memory", "memory/light");
                break;
            case Theme::Panda_Dark:
                pixmapPath.replace("memory", "memory/dark");
                break;
            }
        }
    }
    if (pixmapPath != m_currentPixmapName) {
        if (!loadedPixmaps.contains(pixmapPath)) {
            // TODO: use QPixmap::loadFromData() here
            if (!loadedPixmaps[pixmapPath].load(pixmapPath)) {
                std::cerr << "Problem loading pixmapPath = " << pixmapPath.toStdString() << '\n';
                throw std::runtime_error(ERRORMSG("Couldn't load pixmap."));
            }
        }
        m_pixmap = &loadedPixmaps[pixmapPath];
        setTransformOriginPoint(m_pixmap->rect().center());
        update(boundingRect());
    }
    m_currentPixmapName = pixmapName;

   // update bottom position for wires to come in the correct place
   setBottomPosition( getPixmap().rect().height() );
}

void GraphicElement::setPixmap(const QString &pixmapName, QRect size)
{
    QString pixmapPath = pixmapName;
    if (ThemeManager::globalMngr) { // TODO: This has to be changed. Not a good way to do it. Probably better inside the class.
        if (pixmapPath.contains("memory")) {
            switch (ThemeManager::globalMngr->theme()) {
            case Theme::Panda_Light:
                pixmapPath.replace("memory", "memory/light");
                break;
            case Theme::Panda_Dark:
                pixmapPath.replace("memory", "memory/dark");
                break;
            }
        }
    }
    if (pixmapPath != m_currentPixmapName) {
        if (!loadedPixmaps.contains(pixmapPath)) {
            // TODO: use QPixmap::loadFromData() here
            QPixmap pixmap;
            if (!pixmap.load(pixmapPath)) {
                throw std::runtime_error(ERRORMSG("Couldn't load pixmap."));
            }
            loadedPixmaps[pixmapPath] = pixmap.copy(size);
        }
        m_pixmap = &loadedPixmaps[pixmapPath];
        setTransformOriginPoint(m_pixmap->rect().center());
        update(boundingRect());
    }
    m_currentPixmapName = pixmapName;
}

QVector<QNEOutputPort *> GraphicElement::outputs() const
{
    return m_outputs;
}

const QNEInputPort *GraphicElement::input(int pos) const
{
    return m_inputs.at(pos);
}

const QNEOutputPort *GraphicElement::output(int pos) const
{
    return m_outputs.at(pos);
}

QNEInputPort *GraphicElement::input(int pos)
{
    return m_inputs.at(pos);
}

QNEOutputPort *GraphicElement::output(int pos)
{
    return m_outputs.at(pos);
}

void GraphicElement::setOutputs(const QVector<QNEOutputPort *> &outputs)
{
    m_outputs = outputs;
}

void GraphicElement::save(QDataStream &ds) const
{
    COMMENT("Saving element. Type: " << objectName().toStdString(), 4);
    ds << pos();
    ds << rotation();
    /* <Version1.2> */
    ds << getLabel();
    /* <\Version1.2> */
    /* <Version1.3> */
    ds << m_minInputSz;
    ds << m_maxInputSz;
    ds << m_minOutputSz;
    ds << m_maxOutputSz;
    /* <\Version1.3> */
    /* <Version1.9> */
    ds << m_trigger;
    /* <\Version1.9> */
    ds << static_cast<quint64>(m_inputs.size());
    for (QNEPort *port : m_inputs) {
        ds << reinterpret_cast<quint64>(port);
        ds << port->portName();
        ds << port->portFlags();
    }
    ds << static_cast<quint64>(m_outputs.size());
    for (QNEPort *port : m_outputs) {
        ds << reinterpret_cast<quint64>(port);
        ds << port->portName();
        ds << port->portFlags();
    }
    /* <\Version2.7> */
    ds << static_cast<quint64>(m_pixmapSkinName.size());
    for (const QString &skinName : m_pixmapSkinName) {
        ds << skinName;
    }
    COMMENT("Finished saving element.", 4);
}

void GraphicElement::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    COMMENT("Loading element. Type: " << objectName().toStdString(), 4);
    loadPos(ds);
    loadAngle(ds);
    /* <Version1.2> */
    loadLabel(ds, version);
    /* <\Version1.2> */
    /* <Version1.3> */
    loadMinMax(ds, version);
    /* <\Version1.3> */
    /* <Version1.9> */
    loadTrigger(ds, version);
    /* <\Version1.9> */
    loadInputPorts(ds, portMap);
    loadOutputPorts(ds, portMap);
    /* <\Version2.7> */
    loadPixmapSkinNames(ds, version);
    COMMENT("Updating port positions.", 4);
    updatePorts();
    COMMENT("Finished loading element.", 4);
}

void GraphicElement::loadPos(QDataStream &ds)
{
    QPointF p;
    ds >> p;
    setPos(p);
}

void GraphicElement::loadAngle(QDataStream &ds)
{
    qreal angle;
    ds >> angle;
    setRotation(angle);
}

void GraphicElement::loadLabel(QDataStream &ds, double version)
{
    if (version >= 1.2) {
        QString label_text;
        ds >> label_text;
        setLabel(label_text);
    }
}

void GraphicElement::loadMinMax(QDataStream &ds, double version)
{
    if (version >= 1.3) {
        quint64 min_isz, max_isz, min_osz, max_osz;
        ds >> min_isz;
        ds >> max_isz;
        ds >> min_osz;
        ds >> max_osz;
        //     FIXME: Was it a bad decision to store Min and Max input/output sizes?
        /* Version 2.2 ?? fix ?? */
        if (!((m_minInputSz == m_maxInputSz) && (m_minInputSz > max_isz))) {
            m_minInputSz = min_isz;
            m_maxInputSz = max_isz;
        }
        if (!((m_minOutputSz == m_maxOutputSz) && (m_minOutputSz > max_osz))) {
            m_minOutputSz = min_osz;
            m_maxOutputSz = max_osz;
        }
    }
}

void GraphicElement::loadTrigger(QDataStream &ds, double version)
{
    if (version >= 1.9) {
        QKeySequence trigger;
        ds >> trigger;
        setTrigger(trigger);
    }
}

void GraphicElement::loadInputPorts(QDataStream &ds, QMap<quint64, QNEPort *> &portMap)
{
    COMMENT("Loading input ports.", 4);
    quint64 inputSz;
    ds >> inputSz;
    if (inputSz > MAXIMUMVALIDINPUTSIZE) {
        throw std::runtime_error(ERRORMSG("Corrupted DataStream!"));
    }
    for (size_t port = 0; port < inputSz; ++port) {
        loadInputPort(ds, portMap, port);
    }
    removeSurplusInputs(inputSz, portMap);
}

void GraphicElement::loadInputPort(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, size_t port)
{
    QString name;
    int flags;
    quint64 ptr;
    ds >> ptr;
    ds >> name;
    ds >> flags;
    if ((port < static_cast<size_t>(m_inputs.size()))) {
        if (elementType() == ElementType::IC) {
            m_inputs[port]->setName(name);
        }
        m_inputs[port]->setPortFlags(flags);
        m_inputs[port]->setPtr(ptr);
    } else {
        addPort(name, false, flags, ptr);
    }
    portMap[ptr] = m_inputs[port];
}

void GraphicElement::removeSurplusInputs(quint64 inputSz, QMap<quint64, QNEPort *> &portMap)
{
    while (inputSize() > static_cast<int>(inputSz) && inputSz >= m_minInputSz) {
        QNEPort *deletedPort = m_inputs.back();
        removePortFromMap(deletedPort, portMap);
        delete deletedPort;
        m_inputs.pop_back();
    }
}

void GraphicElement::removeSurplusOutputs(quint64 outputSz, QMap<quint64, QNEPort *> &portMap)
{
    while (outputSize() > static_cast<int>(outputSz) && outputSz >= m_minOutputSz) {
        QNEPort *deletedPort = m_outputs.back();
        removePortFromMap(deletedPort, portMap);
        delete deletedPort;
        m_outputs.pop_back();
    }
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

void GraphicElement::loadOutputPorts(QDataStream &ds, QMap<quint64, QNEPort *> &portMap)
{
    COMMENT("Loading output ports.", 4);
    quint64 outputSz;
    ds >> outputSz;
    if (outputSz > MAXIMUMVALIDINPUTSIZE) {
        throw std::runtime_error(ERRORMSG("Corrupted DataStream!"));
    }
    for (size_t port = 0; port < outputSz; ++port) {
        loadOutputPort(ds, portMap, port);
    }
    removeSurplusOutputs(outputSz, portMap);
}

void GraphicElement::loadOutputPort(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, size_t port)
{
    QString name;
    int flags;
    quint64 ptr;
    ds >> ptr;
    ds >> name;
    ds >> flags;
    if ((port < static_cast<size_t>(m_outputs.size()))) {
        if (elementType() == ElementType::IC) {
            m_outputs[port]->setName(name);
        }
        m_outputs[port]->setPortFlags(flags);
        m_outputs[port]->setPtr(ptr);
    } else {
        addPort(name, true, flags, ptr);
    }
    portMap[ptr] = m_outputs[port];
}

void GraphicElement::loadPixmapSkinNames(QDataStream &ds, double version)
{
    if (version >= 2.7) {
        COMMENT("Loading pixmap skin names.", 4);
        quint64 outputSz;
        ds >> outputSz;
        if (outputSz > MAXIMUMVALIDINPUTSIZE) {
            throw std::runtime_error(ERRORMSG("Corrupted DataStream!"));
        }
        for (size_t port = 0; port < outputSz; ++port) {
            loadPixmapSkinName(ds, port);
        }
        refresh();
    }
}

void GraphicElement::loadPixmapSkinName(QDataStream &ds, size_t skin)
{
    QString name;
    ds >> name;
    if ((skin < static_cast<size_t>(m_pixmapSkinName.size()))) {
        if (name[0] != ":") {
            QDir dir(QFileInfo(GlobalProperties::currentFile).absoluteDir());
            QString filename(QFileInfo(name).fileName());
            QFileInfo fileInfo(dir, filename);
            COMMENT("Skin filename: " << fileInfo.absoluteFilePath().toStdString(), 0);
            if (fileInfo.isFile()) {
                m_pixmapSkinName[skin] = fileInfo.absoluteFilePath();
            } else {
                std::cout << "Could not load some of the skins." << std::endl;
            }
        }
    } else {
        std::cout << "Could not load some of the skins." << std::endl;
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
    Q_UNUSED(widget)
    painter->setClipRect(option->exposedRect);
    if (isSelected()) {
        painter->setBrush(m_selectionBrush);
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        painter->drawRoundedRect(boundingRect(), 5, 5);
    }
    painter->drawPixmap(QPoint(0, 0), getPixmap());
}

QNEPort *GraphicElement::addPort(const QString &name, bool isOutput, int flags, int ptr)
{
    COMMENT("Adding new port.", 4);
    if (isOutput && (static_cast<quint64>(m_outputs.size()) >= m_maxOutputSz)) {
        return nullptr;
    }
    if (!isOutput && (static_cast<quint64>(m_inputs.size()) >= m_maxInputSz)) {
        return nullptr;
    }
    QNEPort *port = nullptr;
    if (isOutput) {
        m_outputs.push_back(new QNEOutputPort(this));
        port = dynamic_cast<QNEPort *>(m_outputs.last());
        port->setIndex(outputSize() - 1);
    } else {
        m_inputs.push_back(new QNEInputPort(this));
        port = dynamic_cast<QNEPort *>(m_inputs.last());
        port->setIndex(inputSize() - 1);
    }
    port->setName(name);
    port->setGraphicElement(this);
    port->setPortFlags(flags);
    port->setPtr(ptr);
    COMMENT("Updating new port.", 4);
    GraphicElement::updatePorts();
    port->show();
    return port;
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
    setToolTip(name);
}

void GraphicElement::setSkin(bool defaultSkin, const QString &filename)
{
    Q_UNUSED(defaultSkin);
    Q_UNUSED(filename);
}

void GraphicElement::updatePorts()
{
    COMMENT("Updating port positions that belong to the IC.", 5);
    int inputPos = m_topPosition;
    int outputPos = m_bottomPosition;
    if (m_outputsOnTop) {
        inputPos = m_bottomPosition;
        outputPos = m_topPosition;
    }

    const QRect& rect = getPixmap().rect();
    int width = rect.width()/2;

    if (!m_outputs.isEmpty()) {
        int step = qMax(width / m_outputs.size(), 6);
        int x = width - m_outputs.size() * step + step;
        foreach (QNEPort *port, m_outputs) {
            COMMENT("Setting output at " << x << ", " << outputPos, 5);
            port->setPos(x, outputPos);
            port->update();
            x += step * 2;
        }
    }
    if (!m_inputs.isEmpty()) {
        int step = qMax(width / m_inputs.size(), 6);
        int x = width - m_inputs.size() * step + step;
        foreach (QNEPort *port, m_inputs) {
            COMMENT("Setting input at " << x << ", " << inputPos, 5);
            port->setPos(x, inputPos);
            port->update();
            x += step * 2;
        }
    }
}

void GraphicElement::refresh()
{
    setPixmap(m_pixmapSkinName[0]);
}

QVariant GraphicElement::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    COMMENT("Align to grid.", 4);
    if ((change == ItemPositionChange) && scene()) {
        QPointF newPos = value.toPointF();
        auto *customScene = dynamic_cast<Scene *>(scene());
        if (customScene) {
            int gridSize = customScene->gridSize();
            qreal xV = qRound(newPos.x() / gridSize) * gridSize;
            qreal yV = qRound(newPos.y() / gridSize) * gridSize;
            return QPointF(xV, yV);
        }
        return newPos;
    }
    COMMENT("Moves wires.", 4);
    if ((change == ItemScenePositionHasChanged) || (change == ItemRotationHasChanged) || (change == ItemTransformHasChanged)) {
        foreach (QNEPort *port, m_outputs) {
            port->updateConnections();
        }
        foreach (QNEPort *port, m_inputs) {
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

void GraphicElement::setHasAudio(bool hasaudio)
{
    m_hasAudio = hasaudio;
}

QKeySequence GraphicElement::getTrigger() const
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
    return QString();
}

void GraphicElement::updateLabel()
{
    QString label = m_labelText;
    if ((!hasTrigger()) || (getTrigger().toString().isEmpty())) {
        m_label->setPlainText(label);
    } else {
        if (!label.isEmpty()) {
            label += " ";
        }
        m_label->setPlainText(label + QString("(%1)").arg(getTrigger().toString()));
    }
}

void GraphicElement::setLabel(const QString &label)
{
    m_labelText = label;
    updateLabel();
}

QString GraphicElement::getLabel() const
{
    return m_labelText;
}

void GraphicElement::updateTheme()
{
    if (ThemeManager::globalMngr) {
        const ThemeAttrs attrs = ThemeManager::globalMngr->getAttrs();

        m_label->setDefaultTextColor(attrs.graphicElement_labelColor);
        m_selectionBrush = attrs.selectionBrush;
        m_selectionPen = attrs.selectionPen;
        for (QNEInputPort *input : qAsConst(m_inputs)) {
            input->updateTheme();
        }
        for (QNEOutputPort *output : qAsConst(m_outputs)) {
            output->updateTheme();
        }
        //updateThemeLocal();
        setPixmap(m_currentPixmapName);
        update();
    }
}

void GraphicElement::updateThemeLocal()
{
}

bool GraphicElement::isValid()
{
    COMMENT("Checking if the element has the required signals to comput its value.", 4);
    bool valid = true;
    for (QNEInputPort *input : qAsConst(m_inputs)) {
        /* Required inputs must have exactly one connection. */
        if (!input->isValid()) {
            valid = false;
            break;
        }
    }
    if (!valid) {
        for (QNEOutputPort *output : qAsConst(m_outputs)) {
            for (QNEConnection *conn : output->connections()) {
                conn->setStatus(QNEConnection::Status::Invalid);
                QNEInputPort *port = conn->otherPort(output);
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

QString GraphicElement::getColor() const
{
    return QString();
}

void GraphicElement::setAudio(const QString &audio)
{
    Q_UNUSED(audio);
}

QString GraphicElement::getAudio() const
{
    return QString();
}

void GraphicElement::setHasColors(bool hasColors)
{
    m_hasColors = hasColors;
}

void GraphicElement::setCanChangeSkin(bool canChangeSkin)
{
    m_canChangeSkin = canChangeSkin;
}

void GraphicElement::setHasTrigger(bool hasTrigger)
{
    m_hasTrigger = hasTrigger;
}

bool GraphicElement::hasFrequency() const
{
    return m_hasFrequency;
}

void GraphicElement::setHasFrequency(bool hasFrequency)
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

void GraphicElement::setHasLabel(bool hasLabel)
{
    m_hasLabel = hasLabel;
    m_label->setVisible(hasLabel);
}

bool GraphicElement::rotatable() const
{
    return m_rotatable;
}

void GraphicElement::setRotatable(bool rotatable)
{
    m_rotatable = rotatable;
}

int GraphicElement::minOutputSz() const
{
    return m_minOutputSz;
}

int GraphicElement::inputSize() const
{
    return m_inputs.size();
}

void GraphicElement::setInputSize(int size)
{
    if ((size >= minInputSz()) && (size <= maxInputSz())) {
        if (inputSize() < size) {
            while (inputSize() < size) {
                addInputPort();
            }
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
    if ((size >= minOutputSz()) && (size <= maxOutputSz())) {
        if (outputSize() < size) {
            for (int port = outputSize(); port < size; ++port) {
                addOutputPort();
            }
        } else {
            qDeleteAll(m_outputs.begin() + size, m_outputs.end());
            m_outputs.resize(size);
        }
    }
}

float GraphicElement::getFrequency() const
{
    return 0.0;
}

void GraphicElement::setFrequency(float)
{
}

void GraphicElement::setMinOutputSz(int minOutputSz)
{
    m_minOutputSz = minOutputSz;
}

int GraphicElement::minInputSz() const
{
    return m_minInputSz;
}

void GraphicElement::setMinInputSz(const int minInputSz)
{
    m_minInputSz = minInputSz;
}

int GraphicElement::maxOutputSz() const
{
    return m_maxOutputSz;
}

void GraphicElement::setMaxOutputSz(int maxOutputSz)
{
    m_maxOutputSz = maxOutputSz;
}

int GraphicElement::maxInputSz() const
{
    return m_maxInputSz;
}

void GraphicElement::setMaxInputSz(int maxInputSz)
{
    m_maxInputSz = maxInputSz;
}

int GraphicElement::bottomPosition() const
{
    return m_bottomPosition;
}

void GraphicElement::setBottomPosition(int bottomPosition)
{
    m_bottomPosition = bottomPosition;
    updatePorts();
}

bool GraphicElement::hasCustomConfig() const
{
    return m_hasCustomConfig;
}

void GraphicElement::setHasCustomConfig(bool hasCustomConfig)
{
    m_hasCustomConfig = hasCustomConfig;
}

int GraphicElement::topPosition() const
{
    return m_topPosition;
}

void GraphicElement::setTopPosition(int topPosition)
{
    m_topPosition = topPosition;
    updatePorts();
}

bool GraphicElement::outputsOnTop() const
{
    return m_outputsOnTop;
}

void GraphicElement::setOutputsOnTop(bool outputsOnTop)
{
    m_outputsOnTop = outputsOnTop;
    updatePorts();
}

bool GraphicElement::disabled()
{
    return m_disabled;
}
