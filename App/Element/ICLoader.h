// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICLoader: loads an IC's sub-circuit from a file or blob and builds its boundary ports.
 */

#pragma once

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QVector>

#include "App/Element/IC.h"

class GraphicElement;
class QFileInfo;
class QGraphicsItem;
class QVersionNumber;

/**
 * \class ICLoader
 * \brief Loads an IC's sub-circuit (from a .panda file or an in-memory blob), substitutes the
 * boundary Input/Output elements with proxy Nodes, and configures the IC's external ports.
 *
 * \details Extracted from IC (a friend of it) so loading/migration is separate from rendering
 * and simulation. It populates the IC's internal element/connection/port vectors through that
 * friendship and drives ICRenderer for the preview/sizing pixmaps. IC's loadFile()/loadFromBlob()
 * entry points delegate here. The static port-metadata scan used by both loading and the save
 * path lives here too (IC::buildPortMetadata delegates to it).
 */
class ICLoader
{
public:
    /// Loads the sub-circuit from \a fileName (resolved against \a contextDir) into \a ic.
    static void loadFile(IC &ic, const QString &fileName, const QString &contextDir);

    /// Loads the sub-circuit from in-memory \a blob bytes (full .panda format) into \a ic.
    static void loadFromBlob(IC &ic, const QByteArray &blob, const QString &contextDir);

    /// Scans \a elements for Input/Output groups, sorts by Y/X position, and builds pin labels.
    static IC::PortMetadata buildPortMetadata(const QVector<GraphicElement *> &elements);

private:
    static void loadFileDirectly(IC &ic, const QFileInfo &fileInfo);
    static void migrateFile(IC &ic, const QFileInfo &fileInfo, const QList<QGraphicsItem *> &items,
                            const QVersionNumber &version, const QMap<QString, QByteArray> &fileRegistry);
    static void deserializeAndLoad(IC &ic, const QByteArray &bytes, const QString &contextDir);

    static void processLoadedItems(IC &ic, const QList<QGraphicsItem *> &items);
    static void loadBoundaryElement(IC &ic, GraphicElement *elm, bool isInput);
    static void loadBoundaryPorts(IC &ic, bool isInput, const QVector<QString> &labels);
};
