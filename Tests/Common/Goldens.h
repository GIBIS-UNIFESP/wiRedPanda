// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Helpers shared by the characterization goldens: comparing with a golden file or writing it, the input rows,
 * and the printers every dump uses.
 */

#pragma once

#include <QByteArray>
#include <QList>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QVector>

#include "App/Core/Application.h"
#include "App/Core/Enums.h"

class GraphicElement;
class Scene;
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

/// Returns the elements of \a scene in ascending id order.
QVector<GraphicElement *> elementsById(const Scene *scene);

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
};
