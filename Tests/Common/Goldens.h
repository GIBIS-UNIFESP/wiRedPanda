// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Helpers shared by the characterization goldens: comparing with a golden file or writing it, the input rows,
 * and the printers every dump uses.
 */

#pragma once

#include <functional>
#include <memory>

#include <QByteArray>
#include <QColor>
#include <QEvent>
#include <QImage>
#include <QList>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QTransform>
#include <QVector>

#include "App/Core/Application.h"
#include "App/Core/Enums.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"

class WorkSpace;

namespace TestUtils {

/**
 * \brief Whether the goldens are written instead of compared: GENERATE_EXPECTED_SNAPSHOTS is set, on POSIX even to an
 * empty value, so nothing that runs the suite may export it. Windows keeps no variable with an empty value.
 */
bool regeneratingGoldens();

/**
 * \brief Compares \a actual with the golden file at \a goldenPath, or writes it there, creating its directory, when
 * regeneratingGoldens(), whether or not the golden exists.
 * \details The file is read and written as UTF-8 bytes, so line endings count. A missing golden fails.
 * \return An empty string on a match or a write; otherwise the file and the first line that differs, from each side.
 */
QString compareWithGolden(const QString &goldenPath, const QString &actual);

/**
 * \brief The byte form of compareWithGolden(): \a actual is compared with the golden file's raw bytes, and written
 * there the same way when regeneratingGoldens().
 * \return An empty string on a match or a write; otherwise the file, both sizes and the first byte that differs.
 */
QString compareWithGoldenBytes(const QString &goldenPath, const QByteArray &actual);

/**
 * \brief Adds a QTest data row per file in \a directory matching \a nameFilters, named by \a tagPrefix followed by its
 * path relative to \a directory, in name order, its "path" column holding the file's path; the caller has added that
 * column. The prefix keeps the rows of several directories apart.
 * \details A missing directory adds one row instead, named after the directory, whose path is the directory, so
 * requireInputFile() fails it rather than the directory's files silently going untested.
 */
void addFileRows(const QString &directory, const QStringList &nameFilters, bool recursive = false,
                 const QString &tagPrefix = {});

/// Returns an empty string when \a path is a readable file, or a message naming it otherwise.
QString requireInputFile(const QString &path);

/**
 * \brief Loads the circuit at \a path into \a workspace as WorkSpace::load(QString) does, but without that function's
 * backup and re-save of a file in an older format, which would write beside the inputs.
 * \details A file that cannot be opened fails the running test; the caller checks QTest::currentTestFailed().
 */
void loadCircuit(WorkSpace &workspace, const QString &path);

/// Formats \a value with four decimals, negative zero as zero, so a sign left by a rotation or flip is no difference.
QString snapshotNumber(qreal value);

/// Formats \a point as "x,y", each coordinate as snapshotNumber() writes it.
QString snapshotPoint(const QPointF &point);

/// Formats \a transform as "m11,m12,m21,m22,dx,dy", each as snapshotNumber() writes it.
QString snapshotTransform(const QTransform &transform);

/// Quotes \a text, escaped so it stays on one line and keeps its leading and trailing spaces.
QString snapshotQuoted(const QString &text);

/// Names \a status as its enumerator is spelled ("Unknown", "Inactive", "Active" or "Error").
QString snapshotStatus(Status status);

/// Names \a type as its enumerator is spelled.
QString snapshotType(ElementType type);

/// A quarter turn and the two flips the goldens cross.
struct Orientation {
    int rotation;
    bool flipX;
    bool flipY;
};

/// The sixteen orientations: each quarter turn unflipped, flipped horizontally, vertically and both ways.
QList<Orientation> orientations();

/// Names \a orientation as "r<rotation>" followed by "x" and "y" for its flips.
QString snapshotOrientation(const Orientation &orientation);

/// Applies \a orientation to \a element.
void orient(GraphicElement *element, const Orientation &orientation);

/// Sets the output count of \a element, and its input count unless \a inputs is negative or the element is a Demux,
/// whose inputs follow its outputs.
void applyPortCounts(GraphicElement *element, int inputs, int outputs);

/// Creates an element in \a scene before it is given its ports; the caller owns it.
using ElementBuilder = std::function<std::unique_ptr<GraphicElement>(Scene *scene)>;

/**
 * \brief Describes one configuration in the sixteen orientations: in each, an element built by \a build is given its
 * ports by \a applyPorts and then oriented, and \a describe's result is listed under \a header; a second element is
 * oriented first and then given its ports, and its description is listed after \a differentText only where it differs
 * from the first's, or \a sameText alone where none does. \a describe returns a QString or a QStringList.
 */
template <typename Describe>
QStringList describeInOrientations(Scene *scene, const QString &header, const ElementBuilder &build,
                                   const std::function<void(GraphicElement *)> &applyPorts, Describe describe,
                                   const QString &sameText, const QString &differentText)
{
    QStringList lines{header};
    QStringList otherOrder;
    for (const auto &orientation : orientations()) {
        const auto portsFirst = build(scene);
        applyPorts(portsFirst.get());
        orient(portsFirst.get(), orientation);
        const auto portsFirstText = describe(portsFirst.get());
        scene->removeItem(portsFirst.get());
        lines << portsFirstText;

        const auto orientedFirst = build(scene);
        orient(orientedFirst.get(), orientation);
        applyPorts(orientedFirst.get());
        const auto orientedFirstText = describe(orientedFirst.get());
        scene->removeItem(orientedFirst.get());
        if (orientedFirstText != portsFirstText) {
            otherOrder << orientedFirstText;
        }
    }

    if (otherOrder.isEmpty()) {
        lines << sameText;
    } else {
        lines << differentText << otherOrder;
    }
    return lines;
}

/// Returns the elements of \a scene in ascending id order.
QVector<GraphicElement *> elementsById(const Scene *scene);

/**
 * \brief Sends the scene mouse events a view sends for a pointer: each carries the scene position, the button, the
 * buttons held, which Qt's item drag and the scene's wiring gestures read, and the modifiers, and, under a view, its
 * viewport and the screen position, which the context menu reads. The scene itself records where each button went
 * down for the item that grabs the pointer. A drag an event starts is cancelled where Qt's timers run during it, as on
 * xcb, where QDrag::exec() would otherwise wait for a physical button; not on Windows, where it first waits for a
 * physical mouse move in a loop that runs no Qt timer.
 */
class ScenePointer
{
public:
    explicit ScenePointer(Scene *scene)
        : m_scene(scene)
    {
    }

    void press(const QPointF &pos, Qt::KeyboardModifiers modifiers = Qt::NoModifier,
               Qt::MouseButton button = Qt::LeftButton);

    /// A move with the held buttons: none while a wire follows the pointer after a click on a port.
    void move(const QPointF &pos, Qt::KeyboardModifiers modifiers = Qt::NoModifier);

    void release(const QPointF &pos, Qt::KeyboardModifiers modifiers = Qt::NoModifier,
                 Qt::MouseButton button = Qt::LeftButton);

    void click(const QPointF &pos, Qt::KeyboardModifiers modifiers = Qt::NoModifier,
               Qt::MouseButton button = Qt::LeftButton);

    /// A press, moves halfway and to \a to, and a release there.
    void drag(const QPointF &from, const QPointF &to);

private:
    void send(QEvent::Type type, const QPointF &pos, Qt::MouseButton button, Qt::KeyboardModifiers modifiers);

    Scene *m_scene;
    Qt::MouseButtons m_held = Qt::NoButton;
};

/**
 * \brief Turns Application::renderingEnabled on for its lifetime, then restores it. The suite runs with it off, and
 * then a wire builds no path, so a golden of wire paths holds one from before the circuit is loaded.
 */
class ScopedRendering
{
public:
    ScopedRendering() { Application::renderingEnabled = true; }
    ~ScopedRendering() { Application::renderingEnabled = m_previous; }
    ScopedRendering(const ScopedRendering &) = delete;
    ScopedRendering &operator=(const ScopedRendering &) = delete;

private:
    const bool m_previous = Application::renderingEnabled;
};

/**
 * \brief How far two renders of the same pixels may differ in each channel: coverage is portable fixed-point, but
 * blending rounds differently along different paint paths and per-SIMD variant. Two paths on one machine differ by up
 * to 2; CI's renderers differ from the development machine's by up to 3 (i386 by 3 on 354 pixels of one wire image).
 */
inline constexpr int kPixelChannelTolerance = 3;

/// The largest difference between \a a and \a b in any one channel, alpha included.
int channelDifference(QRgb a, QRgb b);

/**
 * \brief Compares \a image with the PNG golden at \a goldenPath, each channel of each pixel within
 * kPixelChannelTolerance, or writes it there as compareWithGolden() does when regeneratingGoldens().
 * \details Both images are compared as premultiplied ARGB32. An image that differs is also saved in the temporary
 * directory, named after the golden with ".actual.png", for inspection.
 * \return An empty string on a match or a write; otherwise the file and both sizes, or the file, how many pixels differ
 * by more than the tolerance, the largest such difference and the first such pixel.
 */
QString compareWithGoldenImage(const QString &goldenPath, const QImage &image);

/// What a dump writes for a type it should cover but cannot create; the golden then fails.
inline constexpr char kNoCreator[] = "NO CREATOR";

/**
 * \brief Whether the dumps leave \a type out on purpose: a bare IC has no file, so ICs are dumped through the circuits
 * that hold them; JKLatch no longer exists and Unknown is not an element, and neither has a creator. Any other type
 * without a creator is written as kNoCreator into its golden, never skipped.
 */
bool leftOutOfDumps(ElementType type);

/// Every element type the dumps cover, that is, every type not leftOutOfDumps(), in the order its name sorts.
QList<ElementType> dumpedTypes();

/// The IC the port layout and label goldens build, the 1-bit full adder: an IC lays out its ports and places its label
/// from its definition rather than from its type.
QString fullAdderIcFile();

} // namespace TestUtils

/**
 * \brief Reads an element's internals for the goldens, as a friend of the element classes, so no production accessor
 * exists only for tests.
 */
class SnapshotAccess
{
public:
    /**
     * \brief Identifies the picture \a element shows by what the code decided rather than by its pixels, which differ
     * by platform (font engine, SIMD blending, Qt version).
     * \return "<width>x<height>:<source>", the source being the resource path, or only the file name of a file on
     * disk, with ":oriented" appended while an oriented variant replaces the base picture; "footprint" for a body drawn
     * to fit the ports; "null" when there is no picture.
     */
    static QString pixmapIdentity(const GraphicElement *element);

    /**
     * \brief The scene transform of \a element's label: where its origin sits and how its glyphs are turned. The
     * label is placed by fixed anchors, never by its text's size, so this does not depend on fonts.
     */
    static QTransform labelSceneTransform(const GraphicElement *element);

    /// The child item that draws \a element's label, so a dump of the scene's items can name it.
    static const QGraphicsItem *label(const GraphicElement *element);
};
