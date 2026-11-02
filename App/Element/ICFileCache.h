// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ICFileCache: the bytes of the IC files a scene holds, read through an interface by ICLoader.
 */

#pragma once

#include <QByteArray>
#include <QString>

/**
 * \class ICFileCache
 * \brief The bytes of the IC files a scene holds, as ICLoader reads them.
 *
 * \details ICLoader reads through this interface so that it needs neither the scene nor its registry. ICRegistry
 * implements it. A scene gives it to each IC as the IC is added, and the IC drops it when its scene changes.
 */
class ICFileCache
{
public:
    virtual ~ICFileCache() = default;

    /// Returns the bytes of the file at \a path, read on first access and kept while the file is watched; empty when the
    /// file cannot be read.
    virtual const QByteArray &cachedFileBytes(const QString &path) = 0;
};
