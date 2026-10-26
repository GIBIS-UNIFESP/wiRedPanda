// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Fills a directory with the repository's circuits, so the files a fuzz input names resolve.
 *
 * A .panda file names the files of its file-backed ICs, which resolve against its own directory. An input written
 * alone resolves none, and its load stops at the first; written beside every .panda file of the repository's examples
 * and test circuits, the names those files hold resolve, and so do the names mutations carry over from them.
 *
 * Some of those circuits are processors whose few kilobytes name thousands of elements through their nested ICs. A
 * load costs time linear in the elements it loads, about 1.2 ms each under ASan, so a harness that loads its input
 * several times in a unit bounds loadedElementCount() to stay well inside the 30 s timeout of a round.
 */

#pragma once

#include <cstdio>
#include <cstdlib>

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QString>
#include <QVector>

#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"

namespace FuzzCircuitDir {

/// The number of \a elements and of the elements every IC among them loaded, nested ICs' included
inline qsizetype loadedElementCount(const QVector<GraphicElement *> &elements)
{
    qsizetype count = elements.size();
    for (const GraphicElement *element : elements) {
        if (const auto *ic = qobject_cast<const IC *>(element)) {
            count += loadedElementCount(ic->internalElements());
        }
    }
    return count;
}

/// Copies every .panda file of the repository's examples and test circuits into \a dir under its base name, as
/// references resolve, read-only so a migration cannot rewrite them between inputs; aborts \a harness when it finds
/// none. The harness binary sits in the build directory, one level below the repository.
inline void fill(const QString &dir, const char *harness)
{
    const QString root = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("..");
    // Earlier directories win a base name: the examples and integration circuits are current, the backward
    // compatibility ones each version's copy
    for (const char *subdir : {"Examples", "Tests/Integration", "Tests/Fixtures", "Tests/BackwardCompatibility"}) {
        QDirIterator it(root + "/" + subdir, {"*.panda"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QFileInfo source(it.next());
            const QString target = QDir(dir).filePath(source.fileName());
            if (!QFileInfo::exists(target) && QFile::copy(source.absoluteFilePath(), target)) {
                QFile::setPermissions(target, QFileDevice::ReadOwner | QFileDevice::ReadUser);
            }
        }
    }
    if (QDir(dir).entryList({"*.panda"}, QDir::Files).isEmpty()) {
        std::fprintf(stderr, "%s: no .panda file found to depend on below %s\n", harness, qPrintable(root));
        std::abort();
    }
}

} // namespace FuzzCircuitDir
