// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/Goldens.h"

#include <algorithm>
#include <cstdlib>

#include <QBuffer>
#include <QDataStream>
#include <QDir>
#include <QDirIterator>
#include <QDrag>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <QMetaEnum>
#include <QTest>
#include <QTimer>
#include <QVariant>

#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

namespace TestUtils {

namespace {

/// Writes \a bytes to \a goldenPath, creating its directory; returns an empty string, or why the write failed.
QString writeGolden(const QString &goldenPath, const QByteArray &bytes)
{
    QDir().mkpath(QFileInfo(goldenPath).absolutePath());
    QFile out(goldenPath);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate) || (out.write(bytes) != bytes.size())) {
        return QString("Could not write %1: %2").arg(goldenPath, out.errorString());
    }
    return {};
}

/// Reads the golden at \a goldenPath into \a bytes; returns an empty string, or a message naming the missing file.
QString readGolden(const QString &goldenPath, QByteArray &bytes)
{
    QFile golden(goldenPath);
    if (!golden.open(QIODevice::ReadOnly)) {
        return QString("Golden file missing: %1; run with GENERATE_EXPECTED_SNAPSHOTS=1 to write it").arg(goldenPath);
    }
    bytes = golden.readAll();
    return {};
}

} // namespace

bool regeneratingGoldens()
{
    return qEnvironmentVariableIsSet("GENERATE_EXPECTED_SNAPSHOTS");
}

QString compareWithGolden(const QString &goldenPath, const QString &actual)
{
    if (regeneratingGoldens()) {
        return writeGolden(goldenPath, actual.toUtf8());
    }

    QByteArray bytes;
    if (const QString error = readGolden(goldenPath, bytes); !error.isEmpty()) {
        return error;
    }
    return compareTexts(goldenPath, QString::fromUtf8(bytes), actual);
}

QString compareTexts(const QString &name, const QString &expected, const QString &actual)
{
    if (actual == expected) {
        return {};
    }

    const QStringList expectedLines = expected.split('\n');
    const QStringList actualLines = actual.split('\n');
    qsizetype line = 0;
    while ((line < expectedLines.size()) && (line < actualLines.size()) && (expectedLines.at(line) == actualLines.at(line))) {
        ++line;
    }
    const QString endOfFile("<end of file>");
    return QString("%1 differs at line %2:\n  expected: %3\n  actual:   %4")
        .arg(name, QString::number(line + 1), expectedLines.value(line, endOfFile), actualLines.value(line, endOfFile));
}

QString compareWithGoldenBytes(const QString &goldenPath, const QByteArray &actual)
{
    if (regeneratingGoldens()) {
        return writeGolden(goldenPath, actual);
    }

    QByteArray expected;
    if (const QString error = readGolden(goldenPath, expected); !error.isEmpty()) {
        return error;
    }
    if (actual == expected) {
        return {};
    }

    const qsizetype common = std::min(actual.size(), expected.size());
    qsizetype offset = 0;
    while ((offset < common) && (actual.at(offset) == expected.at(offset))) {
        ++offset;
    }
    return QString("%1 differs: %2 bytes, expected %3 bytes; first difference at byte %4")
        .arg(goldenPath, QString::number(actual.size()), QString::number(expected.size()), QString::number(offset));
}

void addFileRows(const QString &directory, const QStringList &nameFilters, const bool recursive, const QString &tagPrefix)
{
    const QDir dir(directory);
    if (!dir.exists()) {
        QTest::newRow(qPrintable("missing directory " + directory)) << directory;
        return;
    }

    QStringList relativePaths;
    QDirIterator it(directory, nameFilters, QDir::Files,
                    recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    while (it.hasNext()) {
        relativePaths.append(dir.relativeFilePath(it.next()));
    }
    relativePaths.sort();
    for (const QString &relativePath : std::as_const(relativePaths)) {
        QTest::newRow(qPrintable(tagPrefix + relativePath)) << dir.filePath(relativePath);
    }
}

QString requireInputFile(const QString &path)
{
    const QFileInfo info(path);
    if (!info.isFile() || !info.isReadable()) {
        return QString("Input missing or not a readable file: %1").arg(path);
    }
    return {};
}

void loadCircuit(WorkSpace &workspace, const QString &path)
{
    QFile file(path);
    QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(path + ": " + file.errorString()));
    QDataStream stream(&file);
    const QVersionNumber version = Serialization::readPandaHeader(stream);
    workspace.load(stream, version, QFileInfo(path).absolutePath());
}

QString snapshotNumber(const qreal value)
{
    const QString text = QString::number(value, 'f', 4);
    return (text == QLatin1String("-0.0000")) ? QString("0.0000") : text;
}

QString snapshotPoint(const QPointF &point)
{
    return snapshotNumber(point.x()) + ',' + snapshotNumber(point.y());
}

QString snapshotTransform(const QTransform &transform)
{
    return QStringList({snapshotNumber(transform.m11()), snapshotNumber(transform.m12()), snapshotNumber(transform.m21()),
                        snapshotNumber(transform.m22()), snapshotNumber(transform.dx()), snapshotNumber(transform.dy())})
        .join(',');
}

QString snapshotQuoted(const QString &text)
{
    QString escaped = text;
    escaped.replace('\\', "\\\\").replace('"', "\\\"").replace('\n', "\\n").replace('\r', "\\r").replace('\t', "\\t");
    return '"' + escaped + '"';
}

QString snapshotStatus(const Status status)
{
    return QVariant::fromValue(status).toString();
}

QString snapshotType(const ElementType type)
{
    return QVariant::fromValue(type).toString();
}

QString snapshotPropertyKind(const PropertyDescriptor::Type type)
{
    switch (type) {
    case PropertyDescriptor::Type::Label:                return "Label";
    case PropertyDescriptor::Type::Color:                return "Color";
    case PropertyDescriptor::Type::Frequency:            return "Frequency";
    case PropertyDescriptor::Type::Delay:                return "Delay";
    case PropertyDescriptor::Type::Audio:                return "Audio";
    case PropertyDescriptor::Type::AudioBox:             return "AudioBox";
    case PropertyDescriptor::Type::Trigger:              return "Trigger";
    case PropertyDescriptor::Type::TruthTable:           return "TruthTable";
    case PropertyDescriptor::Type::Appearance:           return "Appearance";
    case PropertyDescriptor::Type::Volume:               return "Volume";
    case PropertyDescriptor::Type::WirelessModeSelector: return "WirelessModeSelector";
    }
    return "Unknown";
}

QList<Orientation> orientations()
{
    QList<Orientation> list;
    for (const int rotation : {0, 90, 180, 270}) {
        for (const int flips : {0, 1, 2, 3}) {
            list.append({rotation, (flips & 1) != 0, (flips & 2) != 0});
        }
    }
    return list;
}

QString snapshotOrientation(const Orientation &orientation)
{
    return 'r' + QString::number(orientation.rotation) + (orientation.flipX ? "x" : "") + (orientation.flipY ? "y" : "");
}

void orient(GraphicElement *element, const Orientation &orientation)
{
    element->setRotation(orientation.rotation);
    element->setFlippedX(orientation.flipX);
    element->setFlippedY(orientation.flipY);
}

void applyPortCounts(GraphicElement *element, const int inputs, const int outputs)
{
    element->setOutputSize(outputs);
    if ((inputs >= 0) && (element->elementType() != ElementType::Demux)) {
        element->setInputSize(inputs);
    }
}

QVector<GraphicElement *> elementsById(const Scene *scene)
{
    QVector<GraphicElement *> elements = scene->unsortedElements();
    std::sort(elements.begin(), elements.end(), [](const GraphicElement *a, const GraphicElement *b) {
        return a->id() < b->id();
    });
    return elements;
}

void ScenePointer::press(const QPointF &pos, const Qt::KeyboardModifiers modifiers, const Qt::MouseButton button)
{
    m_held |= button;
    send(QEvent::GraphicsSceneMousePress, pos, button, modifiers);
}

void ScenePointer::move(const QPointF &pos, const Qt::KeyboardModifiers modifiers)
{
    send(QEvent::GraphicsSceneMouseMove, pos, Qt::NoButton, modifiers);
}

void ScenePointer::release(const QPointF &pos, const Qt::KeyboardModifiers modifiers, const Qt::MouseButton button)
{
    m_held &= ~button;
    send(QEvent::GraphicsSceneMouseRelease, pos, button, modifiers);
}

void ScenePointer::click(const QPointF &pos, const Qt::KeyboardModifiers modifiers, const Qt::MouseButton button)
{
    press(pos, modifiers, button);
    release(pos, modifiers, button);
}

void ScenePointer::drag(const QPointF &from, const QPointF &to)
{
    press(from);
    move((from + to) / 2.0);
    move(to);
    release(to);
}

void ScenePointer::send(const QEvent::Type type, const QPointF &pos, const Qt::MouseButton button,
                        const Qt::KeyboardModifiers modifiers)
{
    QGraphicsSceneMouseEvent event(type);
    event.setScenePos(pos);
    event.setButton(button);
    event.setButtons(m_held);
    event.setModifiers(modifiers);
    if (auto *view = m_scene->view()) {
        event.setWidget(view->viewport());
        event.setScreenPos(view->viewport()->mapToGlobal(view->mapFromScene(pos)));
    }
    // A Ctrl press on a selected element starts a clone-drag. Under offscreen QDrag::exec() returns at once; on xcb it
    // runs an event loop until a physical button changes or the drag is cancelled. The timer fires only inside such a
    // loop, and cancels the drag. On Windows it waits for a physical mouse move first, in a loop that runs no Qt timer.
    QTimer cancelDrag;
    cancelDrag.callOnTimeout(&QDrag::cancel);
    cancelDrag.start(0);
    QCoreApplication::sendEvent(m_scene, &event);
}

int channelDifference(const QRgb a, const QRgb b)
{
    return std::max({std::abs(qRed(a) - qRed(b)), std::abs(qGreen(a) - qGreen(b)), std::abs(qBlue(a) - qBlue(b)),
                     std::abs(qAlpha(a) - qAlpha(b))});
}

QString compareWithGoldenImage(const QString &goldenPath, const QImage &image)
{
    if (regeneratingGoldens()) {
        QByteArray png;
        QBuffer buffer(&png);
        if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "PNG")) {
            return QString("Could not encode %1 as PNG").arg(goldenPath);
        }
        return writeGolden(goldenPath, png);
    }

    QByteArray bytes;
    if (const QString error = readGolden(goldenPath, bytes); !error.isEmpty()) {
        return error;
    }
    const QImage golden = QImage::fromData(bytes, "PNG").convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (golden.isNull()) {
        return QString("%1 is not a PNG image").arg(goldenPath);
    }
    const QImage actual = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (actual.size() != golden.size()) {
        return QString("%1 is %2x%3, the image %4x%5")
            .arg(goldenPath, QString::number(golden.width()), QString::number(golden.height()),
                 QString::number(actual.width()), QString::number(actual.height()));
    }

    qsizetype differing = 0;
    int worst = 0;
    QPoint first;
    for (int y = 0; y < actual.height(); ++y) {
        for (int x = 0; x < actual.width(); ++x) {
            const QRgb actualPixel = actual.pixel(x, y);
            const QRgb goldenPixel = golden.pixel(x, y);
            if ((actualPixel == kMaskColour) || (goldenPixel == kMaskColour)) {
                continue;
            }
            const int difference = channelDifference(actualPixel, goldenPixel);
            if (difference <= kPixelChannelTolerance) {
                continue;
            }
            if (differing == 0) {
                first = QPoint(x, y);
            }
            ++differing;
            worst = std::max(worst, difference);
        }
    }
    if (differing == 0) {
        return {};
    }

    const QString actualPath = QDir::temp().filePath(QFileInfo(goldenPath).completeBaseName() + ".actual.png");
    actual.save(actualPath, "PNG");
    return QString("%1 differs: %2 pixels by more than %3, at most %4, the first at %5,%6; this run's image is %7")
        .arg(goldenPath, QString::number(differing), QString::number(kPixelChannelTolerance), QString::number(worst),
             QString::number(first.x()), QString::number(first.y()), actualPath);
}

bool leftOutOfDumps(const ElementType type)
{
    return (type == ElementType::IC) || (type == ElementType::JKLatch) || (type == ElementType::Unknown);
}

QList<ElementType> dumpedTypes()
{
    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    QList<ElementType> types;
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));
        if (!leftOutOfDumps(type)) {
            types.append(type);
        }
    }
    std::sort(types.begin(), types.end(), [](const ElementType a, const ElementType b) {
        return snapshotType(a) < snapshotType(b);
    });
    return types;
}

QString fullAdderIcFile()
{
    return cpuComponentsDir() + "level2_full_adder_1bit.panda";
}

void addSegmentDisplaysAndDisplayIc(CircuitBuilder &builder, const QString &directory)
{
    const QString icFile = QDir(directory).filePath("display.panda");
    {
        WorkSpace definition;
        CircuitBuilder inner(definition.scene());
        auto *segment = new InputSwitch;
        auto *shown = new Display7;
        segment->setLabel("Segment");
        shown->setLabel("Shown");
        inner.add(segment, shown);
        inner.connect(segment, 0, shown, 0);
        definition.save(icFile);
    }

    auto *ic = new IC;
    ic->loadFile(icFile, directory);
    auto *display7 = new Display7;
    auto *display14 = new Display14;
    auto *display16 = new Display16;
    auto *icSegment = new InputSwitch;
    auto *segment7 = new InputSwitch;
    auto *segment14 = new InputSwitch;
    auto *segment16 = new InputSwitch;
    builder.add(icSegment, segment7, segment14, segment16, ic, display7, display14, display16);
    builder.connect(icSegment, 0, ic, 0);
    builder.connect(segment7, 0, display7, 0);
    builder.connect(segment14, 0, display14, 0);
    builder.connect(segment16, 0, display16, 0);
}

} // namespace TestUtils

QString SnapshotAccess::pixmapIdentity(const GraphicElement *element)
{
    const ElementAppearance &appearance = element->m_appearance;
    if (appearance.m_hasCustomRenderPixmap) {
        return "footprint";
    }
    if (appearance.m_pixmap.isNull()) {
        return "null";
    }

    const QSize size = appearance.m_pixmap.size();
    // A file on disk keeps only its name: its directory is wherever the checkout is.
    const QString &path = appearance.m_resolvedPixmapPath;
    const QString source = path.startsWith(':') ? path : QFileInfo(path).fileName();
    const bool oriented = appearance.m_pixmap.cacheKey() != appearance.m_basePixmap.cacheKey();
    return QString::number(size.width()) + 'x' + QString::number(size.height()) + ':' + source + (oriented ? ":oriented" : "");
}

const QGraphicsItem *SnapshotAccess::label(const GraphicElement *element)
{
    return element->m_label;
}

QTransform SnapshotAccess::labelSceneTransform(const GraphicElement *element)
{
    return element->m_label->sceneTransform();
}

QVector<const Port *> SnapshotAccess::icPinPorts(const IC *ic, const bool inputs)
{
    const auto &ports = inputs ? ic->m_internalInputs : ic->m_internalOutputs;
    return QVector<const Port *>(ports.cbegin(), ports.cend());
}
