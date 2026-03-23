// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElement.h"

#include <cmath>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>

#include "App/Core/Common.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementMetadata.h"
#include "App/GlobalProperties.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Versions.h"

static const int s_graphicElementMetatypeId = qRegisterMetaType<GraphicElement>();

GraphicElement::GraphicElement(ElementType type, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_elementType(type)
{
    const auto &metadata = ElementMetadataRegistry::metadata(type);
    m_defaultSkins = metadata.defaultSkins;
    m_pixmapPath = metadata.pixmapPath();

    // For elements whose pixmapPath is theme-dependent (e.g. memory elements),
    // defaultSkins may be left empty in the metadata to avoid evaluating the
    // theme path during static initialisation (before QApplication exists).
    // Populate it lazily here from the now-correct pixmapPath.
    if (m_defaultSkins.isEmpty() && !m_pixmapPath.isEmpty()) {
        m_defaultSkins = QStringList({m_pixmapPath});
    }

    m_alternativeSkins = metadata.alternativeSkins.isEmpty() ? m_defaultSkins : metadata.alternativeSkins;
    m_titleText = QCoreApplication::translate(metadata.trContext, metadata.titleText);
    m_translatedName = QCoreApplication::translate(metadata.trContext, metadata.translatedName);
    m_elementGroup = metadata.group;
    m_hasColors = metadata.hasColors;
    m_maxInputSize = metadata.maxInputSize;
    m_maxOutputSize = metadata.maxOutputSize;
    m_minInputSize = metadata.minInputSize;
    m_minOutputSize = metadata.minOutputSize;

    qCDebug(four) << "Setting flags of elements.";
    // ItemSendsGeometryChanges is required so itemChange() receives ItemPositionChange and
    // can snap movement to the grid
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);

    qCDebug(four) << "Setting attributes.";
    m_label->setVisible(metadata.hasLabel);
    QFont font("SansSerif");
    font.setBold(true);
    m_label->setFont(font);
    // 64 px below origin keeps the label below the standard 64×64 element body
    m_label->setPos(0, 64);
    m_label->setParentItem(this);
    m_label->setDefaultTextColor(Qt::black);

    setPortName(m_translatedName);
    setToolTip(m_translatedName);

    qCDebug(four) << "Including input and output ports.";
    setInputSize(static_cast<int>(metadata.minInputSize));
    setOutputSize(static_cast<int>(metadata.minOutputSize));

    GraphicElement::updatePortsProperties();
    GraphicElement::updateTheme();

    // DeviceCoordinateCache reuses the rendered pixmap when the device transform changes,
    // giving a large speedup for elements that don't redraw on every pan/zoom
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    if (!m_defaultSkins.isEmpty()) {
        setPixmap(0);
    }
}

ElementType GraphicElement::elementType() const
{
    return m_elementType;
}

ElementGroup GraphicElement::elementGroup() const
{
    return m_elementGroup;
}

QPixmap GraphicElement::pixmap() const
{
    return m_pixmap;
}

void GraphicElement::setPixmap(const int index)
{
    setPixmap(m_usingDefaultSkin ? m_defaultSkins.at(index) : m_alternativeSkins.at(index));
}

void GraphicElement::setPixmap(const QString &pixmapPath)
{
    // Skip if unchanged to avoid redundant loads and cache invalidation
    if (pixmapPath.isEmpty() || (pixmapPath == m_currentPixmapPath)) {
        return;
    }

    QString path = pixmapPath;

    // Qt resource paths start with ":/"; anything else is a filesystem path
    // relative to the project's working directory (where the .panda file lives).
    // Try the path as-is against currentDir first; if not found, fall back to
    // just the filename — handles cross-platform absolute paths from old files.
    if (not path.startsWith(":/")) {
        const QDir dir(GlobalProperties::currentDir);
        const QString resolved = dir.filePath(path);

        if (!QFileInfo::exists(resolved)) {
            path = dir.filePath(QFileInfo(QString(path).replace('\\', '/')).fileName());
        } else {
            path = resolved;
        }
    }

    if (!m_pixmap.load(path)) {
        const QFileInfo info(path);
        const QString reason = !info.exists()
                                   ? tr("File does not exist")
                                   : !info.isReadable()
                                         ? tr("File is not readable")
                                         : tr("Unknown reason");

        // Load the default skin so the element remains renderable before the exception unwinds
        m_pixmap.load(m_defaultSkins.constFirst());
        qCDebug(zero) << "Problem loading pixmapPath: " << path;
        throw PANDACEPTION("Couldn't load pixmap: %1 (%2)", path, reason);
    }

    // The transform origin must be updated whenever the pixmap changes so that
    // rotation and scale operations remain centred on the new image
    setTransformOriginPoint(pixmapCenter());
    update();

    m_currentPixmapPath = pixmapPath;
}

const QVector<QNEOutputPort *> &GraphicElement::outputs() const
{
    return m_outputPorts;
}

QNEInputPort *GraphicElement::inputPort(const int index)
{
    if (index < 0 || index >= m_inputPorts.size()) {
        return nullptr;
    }
    return m_inputPorts.at(index);
}

QNEOutputPort *GraphicElement::outputPort(const int index)
{
    if (index < 0 || index >= m_outputPorts.size()) {
        return nullptr;
    }
    return m_outputPorts.at(index);
}

void GraphicElement::save(QDataStream &stream) const
{
    qCDebug(four) << "Saving element. Type: " << objectName();

    QMap<QString, QVariant> map;
    map.insert("pos", pos());
    map.insert("rotation", rotation());
    map.insert("label", label());
    map.insert("minInputSize", m_minInputSize);
    map.insert("maxInputSize", m_maxInputSize);
    map.insert("minOutputSize", m_minOutputSize);
    map.insert("maxOutputSize", m_maxOutputSize);
    map.insert("trigger", m_trigger);
    map.insert("priority", m_priority);

    stream << map;

    // -------------------------------------------

    QList<QMap<QString, QVariant>> inputMap;

    for (auto *port : m_inputPorts) {
        QMap<QString, QVariant> tempMap;
        // The raw pointer is serialised as a 64-bit integer and used only as a
        // temporary key when reconstructing connection endpoints during load();
        // it is never dereferenced after the save/load round-trip.
        tempMap.insert("ptr", reinterpret_cast<quint64>(port));
        tempMap.insert("name", port->name());

        inputMap << tempMap;
    }

    stream << inputMap;

    // -------------------------------------------

    QList<QMap<QString, QVariant>> outputMap;

    for (auto *port : m_outputPorts) {
        QMap<QString, QVariant> tempMap;
        tempMap.insert("ptr", reinterpret_cast<quint64>(port));
        tempMap.insert("name", port->name());

        outputMap << tempMap;
    }

    stream << outputMap;

    // -------------------------------------------

    QList<QMap<QString, QVariant>> skinsMap;

    for (const auto &skinName : m_alternativeSkins) {
        QFileInfo fileInfo(skinName);
        QString skinName2 = skinName;

        // When a custom skin lives outside the project directory, copy it alongside
        // the .panda file so the project remains self-contained when moved or shared.
        if (!skinName.startsWith(":/") && (fileInfo.absoluteDir() != GlobalProperties::currentDir)) {
            const QString newFile = GlobalProperties::currentDir + "/" + fileInfo.fileName();
            QFile::copy(skinName, newFile);
            // Store only the bare filename; the project dir is prepended on load.
            skinName2 = fileInfo.fileName();
        }

        // -------------------------------------------

        QMap<QString, QVariant> tempMap;
        tempMap.insert("skinName", skinName2);

        skinsMap << tempMap;
    }

    stream << skinsMap;

    // -------------------------------------------

    qCDebug(four) << "Finished saving element.";
}

void GraphicElement::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    qCDebug(four) << "Loading element. Type: " << objectName();

    // Files before 4.1 used a flat sequential binary format; 4.1+ use a keyed QMap
    // format that tolerates fields being added or reordered in future versions.
    (version < Versions::V_4_1) ? loadOldFormat(stream, portMap, version) : loadNewFormat(stream, portMap);

    qCDebug(four) << "Updating port positions.";
    updatePortsProperties();
    // Apply the deserialized angle after ports are positioned so any non-rotatable element
    // can apply its own rotatePorts() path correctly
    setRotation(m_angle);

    qCDebug(four) << "Finished loading element.";
}

void GraphicElement::loadOldFormat(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    loadPos(stream);
    loadRotation(stream, version);
    /* <Version1.2> */
    loadLabel(stream, version);
    /* <\Version1.2> */
    /* <Version1.3> */
    loadPortsSize(stream, version);
    /* <\Version1.3> */
    /* <Version1.9> */
    loadTrigger(stream, version);
    /* <\Version4.01> */
    loadPriority(stream, version);
    /* <\Version1.9> */
    loadInputPorts(stream, portMap);
    loadOutputPorts(stream, portMap);
    /* <\Version2.7> */
    loadPixmapSkinNames(stream, version);
}

void GraphicElement::loadNewFormat(QDataStream &stream, QMap<quint64, QNEPort *> &portMap)
{
    QMap<QString, QVariant> map; stream >> map;

    // Check stream integrity after reading element properties map
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading element properties at position %1",
                          stream.device()->pos());
    }

    if (map.contains("pos")) {
        setPos(map.value("pos").toPointF());
    }

    if (map.contains("rotation")) {
        m_angle = map.value("rotation").toReal();
    }

    if (map.contains("label")) {
        setLabel(map.value("label").toString());
    }

    // -------------------------------------------

    const quint64 minInputSize = map.value("minInputSize").toULongLong();
    const quint64 maxInputSize = map.value("maxInputSize").toULongLong();
    const quint64 minOutputSize = map.value("minOutputSize").toULongLong();
    const quint64 maxOutputSize = map.value("maxOutputSize").toULongLong();

    // Only update constraints when the element allows variable port counts OR when
    // the saved value falls within the current allowed range.  Prevents a fixed-port
    // element (min == max) from being silently corrupted by a stale saved value.
    if ((m_minInputSize != m_maxInputSize) || (m_minInputSize <= maxInputSize)) {
        m_minInputSize = minInputSize;
        m_maxInputSize = maxInputSize;
    }

    if ((m_minOutputSize != m_maxOutputSize) || (m_minOutputSize <= maxOutputSize)) {
        m_minOutputSize = minOutputSize;
        m_maxOutputSize = maxOutputSize;
    }

    // -------------------------------------------

    if (map.contains("trigger")) {
        setTrigger(map.value("trigger").toString());
    }

    if (map.contains("priority")) {
        setPriority(static_cast<int>(map.value("priority").toULongLong()));
    }

    // -------------------------------------------

    QList<QMap<QString, QVariant>> inputMap; stream >> inputMap;

    // Check stream integrity after reading input ports map
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading input ports at position %1",
                          stream.device()->pos());
    }

    int port = 0;

    for (const auto &input : std::as_const(inputMap)) {
        const quint64 ptr = input.value("ptr").toULongLong();
        const QString name = input.value("name").toString();

        if (port >= m_inputPorts.size()) {
            addPort(name, false);
        }

        portMap[ptr] = m_inputPorts.value(port);

        ++port;
    }

    removeSurplusInputs(static_cast<quint64>(inputMap.size()), portMap);

    // -------------------------------------------

    QList<QMap<QString, QVariant>> outputMap; stream >> outputMap;

    // Check stream integrity after reading output ports map
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading output ports at position %1",
                          stream.device()->pos());
    }

    port = 0;

    for (const auto &output : std::as_const(outputMap)) {
        const quint64 ptr = output.value("ptr").toULongLong();
        const QString name = output.value("name").toString();

        if (port >= m_outputPorts.size()) {
            addPort(name, true);
        }

        portMap[ptr] = m_outputPorts.value(port);

        ++port;
    }

    removeSurplusOutputs(static_cast<quint64>(outputMap.size()), portMap);

    // -------------------------------------------

    QList<QMap<QString, QVariant>> skinsMap; stream >> skinsMap;

    // Check stream integrity after reading skins map
    if (stream.status() != QDataStream::Ok) {
        throw PANDACEPTION("Stream error reading skins at position %1",
                          stream.device()->pos());
    }

    int skin = 0;

    for (const auto &skinName : std::as_const(skinsMap)) {
        const QString name = skinName.value("skinName").toString();

        if (!name.startsWith(":/")) {
            m_alternativeSkins[skin] = name;
        }

        ++skin;
    }

    // If all alternative skin slots still match the defaults, the user never
    // applied a custom skin — record that so the "Reset to default" action works.
    m_usingDefaultSkin = std::equal(
        m_defaultSkins.begin(), m_defaultSkins.end(),
        m_alternativeSkins.begin(), m_alternativeSkins.end()
        );

    refresh();
}

void GraphicElement::loadPos(QDataStream &stream)
{
    QPointF pos; stream >> pos;
    setPos(pos);
}

void GraphicElement::loadRotation(QDataStream &stream, const QVersionNumber &version)
{
    qreal angle; stream >> angle;
    m_angle = angle;

    // In versions before 4.1 the coordinate system for inputs and outputs was
    // rotated 90° relative to the current convention.  Apply a compensating offset
    // so old files render correctly without migrating every saved angle.
    if (version < Versions::V_4_1) {
        if ((m_elementGroup == ElementGroup::Input) || (m_elementGroup == ElementGroup::StaticInput)) {
            m_angle += 90;
        }

        if ((m_elementGroup == ElementGroup::Output) || (m_elementGroup == ElementGroup::IC) || (m_elementGroup == ElementGroup::Gate)) {
            // Displays and Node never had the old convention applied, so skip them.
            if ((m_elementType == ElementType::Display7) || (m_elementType == ElementType::Display14) || (m_elementType == ElementType::Display16) || (m_elementType == ElementType::Node)) {
                return;
            }

            m_angle -= 90;
        }
    }
}

void GraphicElement::loadLabel(QDataStream &stream, const QVersionNumber &version)
{
    if (version >= Versions::V_1_2) {
        QString labelText; stream >> labelText;
        setLabel(labelText);
    }
}

void GraphicElement::loadPortsSize(QDataStream &stream, const QVersionNumber &version)
{
    if (version >= Versions::V_1_3) {
        quint64 minInputSize;  stream >> minInputSize;
        quint64 maxInputSize;  stream >> maxInputSize;
        quint64 minOutputSize; stream >> minOutputSize;
        quint64 maxOutputSize; stream >> maxOutputSize;

        // Only override the compiled-in constraints when the element has a variable port count
        // (min != max) or the saved count is within the allowed range.  This prevents a fixed-port
        // element from being corrupted by a stale value that was written before its constraints
        // were tightened.
        if ((m_minInputSize != m_maxInputSize) || (m_minInputSize <= maxInputSize)) {
            m_minInputSize = minInputSize;
            m_maxInputSize = maxInputSize;
        }

        if ((m_minOutputSize != m_maxOutputSize) || (m_minOutputSize <= maxOutputSize)) {
            m_minOutputSize = minOutputSize;
            m_maxOutputSize = maxOutputSize;
        }
    }
}

void GraphicElement::loadTrigger(QDataStream &stream, const QVersionNumber &version)
{
    if (version >= Versions::V_1_9) {
        QKeySequence trigger; stream >> trigger;
        setTrigger(trigger);
    }
}

void GraphicElement::loadPriority(QDataStream &stream, const QVersionNumber &version)
{
    if (version >= Versions::V_4_0_1) {
        quint64 priority; stream >> priority;
        setPriority(static_cast<int>(priority));
    }
}

void GraphicElement::loadInputPorts(QDataStream &stream, QMap<quint64, QNEPort *> &portMap)
{
    qCDebug(four) << "Loading input ports.";
    quint64 inputSize; stream >> inputSize;

    for (size_t port = 0; port < inputSize; ++port) {
        loadInputPort(stream, portMap, static_cast<int>(port));
    }

    removeSurplusInputs(inputSize, portMap);
}

void GraphicElement::loadInputPort(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const int port)
{
    quint64 ptr;  stream >> ptr;
    QString name; stream >> name;
    int flags;    stream >> flags;

    if (port >= m_inputPorts.size()) {
        addPort(name, false);
    }

    portMap[ptr] = m_inputPorts.value(port);
}

void GraphicElement::removeSurplusInputs(const quint64 inputSize_, QMap<quint64, QNEPort *> &portMap)
{
    // The element may have been constructed with more ports than are stored in the file
    // (e.g. default constructor creates minInputSize ports).  Trim the excess from the end,
    // but never go below minInputSize to avoid leaving an unusable element.
    while ((inputSize() > static_cast<int>(inputSize_)) && (inputSize_ >= m_minInputSize)) {
        auto *deletedPort = m_inputPorts.constLast();
        removePortFromMap(deletedPort, portMap);
        delete deletedPort;
        m_inputPorts.removeLast();
    }
}

void GraphicElement::removeSurplusOutputs(const quint64 outputSize_, QMap<quint64, QNEPort *> &portMap)
{
    // Same trimming logic as removeSurplusInputs, applied to output ports
    while ((outputSize() > static_cast<int>(outputSize_)) && (outputSize_ >= m_minOutputSize)) {
        auto *deletedPort = m_outputPorts.constLast();
        removePortFromMap(deletedPort, portMap);
        delete deletedPort;
        m_outputPorts.removeLast();
    }
}

void GraphicElement::setLogic(LogicElement *newLogic)
{
    m_logic = newLogic;
}

LogicElement *GraphicElement::logic() const
{
    return m_logic;
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
    quint64 outputSize; stream >> outputSize;

    for (size_t port = 0; port < outputSize; ++port) {
        loadOutputPort(stream, portMap, static_cast<int>(port));
    }

    removeSurplusOutputs(outputSize, portMap);
}

void GraphicElement::loadOutputPort(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const int port)
{
    quint64 ptr;  stream >> ptr;
    QString name; stream >> name;
    int flags;    stream >> flags;

    if (port >= m_outputPorts.size()) {
        addPort(name, true);
    }

    portMap[ptr] = m_outputPorts.value(port);
}

void GraphicElement::loadPixmapSkinNames(QDataStream &stream, const QVersionNumber version)
{
    if (version >= Versions::V_2_7) {
        qCDebug(four) << tr("Loading pixmap skin names.");
        quint64 outputSize; stream >> outputSize;

        if (m_elementType == ElementType::IC) {
            // IC paints itself procedurally and has no skin slots.
            // Drain the saved skin names from old file formats to keep the stream position valid.
            for (quint64 i = 0; i < outputSize; ++i) {
                QString discarded; stream >> discarded;
            }
            return;
        }

        for (size_t skin = 0; skin < outputSize; ++skin) {
            loadPixmapSkinName(stream, static_cast<int>(skin));
        }

        m_usingDefaultSkin = std::equal(
            m_defaultSkins.begin(), m_defaultSkins.end(),
            m_alternativeSkins.begin(), m_alternativeSkins.end()
            );

        refresh();
    }
}

void GraphicElement::loadPixmapSkinName(QDataStream &stream, const int skin)
{
    QString name; stream >> name;

    if (skin >= m_alternativeSkins.size()) {
        throw PANDACEPTION("Skin index %1 out of range (size=%2) for skin name \"%3\" — stream may be corrupt",
                           QString::number(skin), QString::number(m_alternativeSkins.size()), name);
    }

    // Only override the alternative skin if it is a filesystem path; resource
    // paths (":/...") are always available and should not be replaced by the
    // potentially missing saved path from an older project file.
    if (!name.startsWith(":/")) {
        m_alternativeSkins[skin] = name;
    }
}

const QVector<QNEInputPort *> &GraphicElement::inputs() const
{
    return m_inputPorts;
}

void GraphicElement::setInputs(const QVector<QNEInputPort *> &inputs)
{
    m_inputPorts = inputs;
}

QPointF GraphicElement::pixmapCenter() const
{
    return QRectF(pixmap().rect()).center();
}

QRectF GraphicElement::boundingRect() const
{
    return portsBoundingRect().united(pixmap().rect());
}

QRectF GraphicElement::portsBoundingRect() const
{
    QRectF rectChildren;
    const auto children = childItems();

    for (auto *child : children) {
        if (auto *port = qgraphicsitem_cast<QNEPort *>(child)) {
            rectChildren = rectChildren.united(mapRectFromItem(port, port->boundingRect()));
        }
    }

    return rectChildren;
}

void GraphicElement::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    if (isSelected()) {
        painter->save();
        painter->setBrush(m_selectionBrush);
        // 0.5 pen width keeps the selection outline thin regardless of zoom level
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        // Corner radius of 5 matches the visual rounding used on element bodies
        painter->drawRoundedRect(boundingRect(), 5, 5);
        painter->restore();
    }

    // Pixmap origin is always (0,0) in item coordinates; the transform origin
    // (centre of the pixmap) is set separately via setTransformOriginPoint().
    painter->drawPixmap(QPoint(0, 0), pixmap());
}

void GraphicElement::addPort(const QString &name, const bool isOutput)
{
    // Silently ignore the request rather than throwing; callers such as load()
    // may attempt to add more ports than the current constraints allow when
    // opening a file saved with looser constraints.
    if (isOutput && (static_cast<quint64>(m_outputPorts.size()) >= m_maxOutputSize)) {
        return;
    }

    if (!isOutput && (static_cast<quint64>(m_inputPorts.size()) >= m_maxInputSize)) {
        return;
    }

    qCDebug(four) << "New port.";
    QNEPort *port = nullptr;

    if (isOutput) {
        m_outputPorts.push_back(new QNEOutputPort(this));
        port = m_outputPorts.constLast();
        port->setIndex(outputSize() - 1);
    } else {
        m_inputPorts.push_back(new QNEInputPort(this));
        port = m_inputPorts.constLast();
        port->setIndex(inputSize() - 1);
    }

    port->setGraphicElement(this);
    port->setName(name);
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
}

void GraphicElement::setPriority(const int value)
{
    m_priority = static_cast<quint64>(value);
}

void GraphicElement::setRotation(const qreal angle)
{
    // Keep angle in [0, 360) to avoid accumulated floating-point drift across many rotations
    m_angle = std::fmod(angle, 360);
    // Rotatable elements rotate the entire QGraphicsItem (pixmap + ports move together).
    // Non-rotatable elements (inputs/outputs) keep the pixmap fixed and only spin ports
    // around the element centre so connections track the correct positions.
    isRotatable() ? QGraphicsItem::setRotation(m_angle) : rotatePorts(m_angle);
}

void GraphicElement::rotatePorts(const qreal angle)
{
    for (auto *port : std::as_const(m_inputPorts)) {
        port->setTransformOriginPoint(mapToItem(port, pixmapCenter()));
        port->setRotation(angle);
        port->updateConnections();
    }

    for (auto *port : std::as_const(m_outputPorts)) {
        port->setTransformOriginPoint(mapToItem(port, pixmapCenter()));
        port->setRotation(angle);
        port->updateConnections();
    }
}

qreal GraphicElement::rotation() const
{
    return m_angle;
}

void GraphicElement::setSkin(const bool defaultSkin, const QString &fileName)
{
    if (defaultSkin) {
        m_alternativeSkins = m_defaultSkins;
    } else {
        m_alternativeSkins[0] = fileName;
    }

    m_usingDefaultSkin = defaultSkin;
    setPixmap(0);
}

void GraphicElement::updatePortsProperties()
{
    qCDebug(five) << "Updating port positions that belong to the IC.";

    // gridSize is 16 px; half that (8 px) is the port spacing unit so ports land
    // on sub-grid snap points that wires can reach when snapped to the same grid.
    const int step = GlobalProperties::gridSize / 2;

    if (!m_inputPorts.isEmpty()) {
        // Centre the port column vertically around y=32 (the mid-point of a 64 px body).
        // The first port starts at 32 - (count-1)*step so all ports are symmetrically distributed.
        int y = 32 - (static_cast<int>(m_inputPorts.size()) * step) + step;

        for (auto *port : std::as_const(m_inputPorts)) {
            qCDebug(five) << "Setting input at " << 0 << ", " << y;

            // Non-rotatable elements (e.g. inputs) spin their ports in place rather than rotating
            // the whole item, so reset port rotation before applying the current element angle
            if (!isRotatable()) {
                port->setRotation(0);
            }

            // Inputs are pinned to the left edge (x=0) of the 64 px body
            port->setPos(0, y);

            if (!isRotatable()) {
                port->setTransformOriginPoint(mapToItem(port, pixmapCenter()));
                port->setRotation(m_angle);
            }

            // Ports are spaced 2*step (16 px) apart so they align with the
            // full grid and remain easily connectable with straight wires.
            y += step * 2;
        }
    }

    if (!m_outputPorts.isEmpty()) {
        int y = 32 - (static_cast<int>(m_outputPorts.size()) * step) + step;

        for (auto *port : std::as_const(m_outputPorts)) {
            qCDebug(five) << "Setting output at " << 64 << ", " << y;

            if (!isRotatable()) {
                port->setRotation(0);
            }

            // Outputs are pinned to the right edge (x=64) of the 64 px body
            port->setPos(64, y);

            if (!isRotatable()) {
                port->setTransformOriginPoint(mapToItem(port, pixmapCenter()));
                port->setRotation(m_angle);
            }

            y += step * 2;
        }
    }
}

void GraphicElement::refresh()
{
    // Reload skin index 0, which is the currently active skin for the element's
    // present state (e.g. after a theme change or skin file replacement).
    setPixmap(0);
}

QVariant GraphicElement::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    // Guard against changes fired during construction before the item is added to a scene
    if (!scene()) {
        return QGraphicsItem::itemChange(change, value);
    }

    if (change == ItemPositionChange) {
        qCDebug(four) << "Align to grid.";
        QPointF newPos = value.toPointF();
        // Snap to half-grid (8 px steps) so elements always align with each other
        // and with the port positions computed in updatePortsProperties().
        const int gridSize = GlobalProperties::gridSize / 2;
        const int xV = qRound(newPos.x() / gridSize) * gridSize;
        const int yV = qRound(newPos.y() / gridSize) * gridSize;
        return QPoint(xV, yV);
    }

    // Any geometric change (move, rotate, shear) requires redrawing all connected wires
    if ((change == ItemScenePositionHasChanged) || (change == ItemRotationHasChanged) || (change == ItemTransformHasChanged)) {
        qCDebug(four) << "Moves wires.";
        for (auto *port : std::as_const(m_outputPorts)) {
            port->updateConnections();
        }

        for (auto *port : std::as_const(m_inputPorts)) {
            port->updateConnections();
        }
    }

    if (change == ItemSelectedHasChanged) {
        m_selected = value.toBool();
        // Propagate selection highlight to all connected wires so users see which
        // signals are attached to the selected element
        highlight(m_selected);
    }

    return QGraphicsItem::itemChange(change, value);
}

bool GraphicElement::sceneEvent(QEvent *event)
{
    // Swallow Ctrl+click events so the scene's rubber-band selection logic can handle them
    // without the element intercepting the press and starting a move instead
    if (event->type() == QEvent::GraphicsSceneMousePress || event->type() == QEvent::GraphicsSceneMouseRelease) {
        if (auto mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent *>(event)) {
            if (mouseEvent->modifiers().testFlag(Qt::ControlModifier)) {
                return true;
            }
        }
    }

    return QGraphicsItem::sceneEvent(event);
}

bool GraphicElement::hasAudio() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasAudio;
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

    // If a keyboard trigger is assigned, append it in parentheses so users can see
    // the shortcut directly on the canvas, e.g. "Clock (Space)"
    if (!hasTrigger() || trigger().toString().isEmpty()) {
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

// Color cycle (forward): White → Red → Green → Blue → Purple → White
// These two functions implement previous/next steps in that cycle so the
// property editor can wrap around without knowing the full list
QString GraphicElement::previousColor() const
{
    if (color() == "White") return "Purple";
    if (color() == "Red") return "White";
    if (color() == "Green") return "Red";
    if (color() == "Blue") return "Green";
    if (color() == "Purple") return "Blue";
    return "White"; // Standard
}

QString GraphicElement::nextColor() const
{
    if (color() == "White") return "Red";
    if (color() == "Red") return "Green";
    if (color() == "Green") return "Blue";
    if (color() == "Blue") return "Purple";
    if (color() == "Purple") return "White";
    return "White"; // Standard
}

// Audio note cycle (ascending): C6 D6 E6 F6 G6 A7 B7 C7 (wraps back to C6)
// The jump from G6 to A7 is intentional — the note names follow the piano
// naming scheme used in the buzzer audio assets
QString GraphicElement::previousAudio() const
{
    if (audio() == "C6") return "C7";
    if (audio() == "D6") return "C6";
    if (audio() == "E6") return "D6";
    if (audio() == "F6") return "E6";
    if (audio() == "G6") return "F6";
    if (audio() == "A7") return "G6";
    if (audio() == "B7") return "A7";
    if (audio() == "C7") return "B7";
    return "C6";
}

QString GraphicElement::nextAudio() const
{
    if (audio() == "C6") return "D6";
    if (audio() == "D6") return "E6";
    if (audio() == "E6") return "F6";
    if (audio() == "F6") return "G6";
    if (audio() == "G6") return "A7";
    if (audio() == "A7") return "B7";
    if (audio() == "B7") return "C7";
    if (audio() == "C7") return "C6";
    return "C6";
}

void GraphicElement::updateTheme()
{
    const ThemeAttributes theme = ThemeManager::attributes();

    m_label->setDefaultTextColor(theme.m_graphicElementLabelColor);
    m_selectionBrush = theme.m_selectionBrush;
    m_selectionPen = theme.m_selectionPen;

    for (auto *input : std::as_const(m_inputPorts)) {
        input->updateTheme();
    }

    for (auto *output : std::as_const(m_outputPorts)) {
        output->updateTheme();
    }

    update();
}

bool GraphicElement::isValid()
{
    qCDebug(four) << "Checking if the element has the required signals to compute its value.";
    // An element is valid only when every input port is satisfied (connected or optional)
    const bool valid = std::all_of(m_inputPorts.cbegin(), m_inputPorts.cend(),
                                   [](auto *input) { return input->isValid(); });

    if (!valid) {
        // Propagate invalid status downstream so the visual chain shows where validity breaks
        for (auto *output : std::as_const(m_outputPorts)) {
            for (auto *conn : output->connections()) {
                conn->setStatus(Status::Invalid);

                if (auto *port = conn->otherPort(output)) {
                    port->setStatus(Status::Invalid);
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
    return ElementMetadataRegistry::metadata(m_elementType).hasTrigger;
}

void GraphicElement::setColor(const QString &color)
{
    Q_UNUSED(color)
}

QString GraphicElement::color() const
{
    return {};
}

void GraphicElement::setAudio(const QString &audio)
{
    Q_UNUSED(audio)
}

QString GraphicElement::audio() const
{
    return {};
}

void GraphicElement::setHasColors(const bool hasColors)
{
    m_hasColors = hasColors;
}

bool GraphicElement::hasFrequency() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasFrequency;
}

bool GraphicElement::hasDelay() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasDelay;
}

bool GraphicElement::hasLabel() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasLabel;
}

bool GraphicElement::hasTruthTable() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasTruthTable;
}

bool GraphicElement::hasAudioBox() const
{
    return ElementMetadataRegistry::metadata(m_elementType).hasAudioBox;
}

bool GraphicElement::canChangeSkin() const
{
    return ElementMetadataRegistry::metadata(m_elementType).canChangeSkin;
}

bool GraphicElement::isRotatable() const
{
    return ElementMetadataRegistry::metadata(m_elementType).rotatable;
}

int GraphicElement::minOutputSize() const
{
    return static_cast<int>(m_minOutputSize);
}

int GraphicElement::inputSize() const
{
    return static_cast<int>(m_inputPorts.size());
}

void GraphicElement::setInputSize(const int size)
{
    if ((size >= minInputSize()) && (size <= maxInputSize())) {
        if (size > inputSize()) {
            while (size > inputSize()) {
                addInputPort();
            }
        } else {
            qDeleteAll(m_inputPorts.begin() + size, m_inputPorts.end());
            m_inputPorts.resize(size);
        }

        updatePortsProperties();
    }
}

int GraphicElement::outputSize() const
{
    return static_cast<int>(m_outputPorts.size());
}

int GraphicElement::priority() const
{
    return static_cast<int>(m_priority);
}

void GraphicElement::setOutputSize(const int size)
{
    if ((size >= minOutputSize()) && (size <= maxOutputSize())) {
        if (size > outputSize()) {
            while (size > outputSize()) {
                addOutputPort();
            }
        } else {
            qDeleteAll(m_outputPorts.begin() + size, m_outputPorts.end());
            m_outputPorts.resize(size);
        }

        updatePortsProperties();
    }
}

float GraphicElement::frequency() const
{
    return 0.0;
}

void GraphicElement::setFrequency(const float freq)
{
    Q_UNUSED(freq)
}

float GraphicElement::delay() const
{
    return 0.0;
}

void GraphicElement::setDelay(const float delay)
{
    Q_UNUSED(delay)
}

LogicElement *GraphicElement::getInputLogic(int portIndex) const
{
    Q_UNUSED(portIndex)
    return logic();
}

LogicElement *GraphicElement::getOutputLogic(int portIndex) const
{
    Q_UNUSED(portIndex)
    return logic();
}

int GraphicElement::getInputIndexForPort(int portIndex) const
{
    return portIndex;
}

int GraphicElement::getOutputIndexForPort(int portIndex) const
{
    return portIndex;
}

QVector<std::shared_ptr<LogicElement>> GraphicElement::getLogicElementsForMapping()
{
    // Wrap the raw pointer in a non-owning shared_ptr (no-op deleter) so the logic mapping
    // infrastructure can handle base elements and ICs uniformly without double-freeing
    return {std::shared_ptr<LogicElement>(logic(), [](LogicElement*){})};
}

void GraphicElement::setMinOutputSize(const int minOutputSize)
{
    m_minOutputSize = static_cast<quint64>(minOutputSize);
}

int GraphicElement::minInputSize() const
{
    return static_cast<int>(m_minInputSize);
}

void GraphicElement::setMinInputSize(const int minInputSize)
{
    m_minInputSize = static_cast<quint64>(minInputSize);
}

int GraphicElement::maxOutputSize() const
{
    return static_cast<int>(m_maxOutputSize);
}

void GraphicElement::setMaxOutputSize(const int maxOutputSize)
{
    m_maxOutputSize = static_cast<quint64>(maxOutputSize);
}

int GraphicElement::maxInputSize() const
{
    return static_cast<int>(m_maxInputSize);
}

void GraphicElement::setMaxInputSize(const int maxInputSize)
{
    m_maxInputSize = static_cast<quint64>(maxInputSize);
}

void GraphicElement::highlight(const bool isSelected)
{
    // Collect all ports (both input and output) into a single list to avoid duplicating
    // the inner loop
    QVector<QNEPort *> ports;

    for (auto *port : std::as_const(m_inputPorts)) {
        ports << port;
    }

    for (auto *port : std::as_const(m_outputPorts)) {
        ports << port;
    }

    for (auto *port : std::as_const(ports)) {
        for (auto *connection : port->connections()) {
            // Skip connections already in the desired highlight state to avoid
            // triggering unnecessary repaints
            if (connection->highLight() == isSelected) {
                continue;
            }

            connection->setHighLight(isSelected);
        }
    }
}

void GraphicElement::retranslate()
{
    m_translatedName = ElementFactory::translatedName(m_elementType);

    setPortName(m_translatedName);
    setToolTip(m_translatedName);
}

void GraphicElement::loadFromDrop(const QString &fileName)
{
    Q_UNUSED(fileName)
}

QDataStream &operator<<(QDataStream &stream, const GraphicElement *item)
{
    qCDebug(four) << "Writing element.";
    const auto *elm = qgraphicsitem_cast<const GraphicElement *>(item);
    if (elm) {
        stream << GraphicElement::Type;
        stream << elm->elementType();
        elm->save(stream);
    }
    return stream;
}

