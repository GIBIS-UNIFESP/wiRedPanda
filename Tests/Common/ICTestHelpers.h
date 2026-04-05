// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFile>

#include "App/Element/IC.h"
#include "App/Element/ICRegistry.h"

namespace ICTestHelpers {

inline QByteArray readFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        return {};
    }
    return f.readAll();
}

/// Stores the raw .panda file bytes as a blob and loads the IC.
inline void embedIC(IC *ic, const QByteArray &pandaBytes, const QString &blobName,
                    const QString &contextDir, ICRegistry *reg)
{
    reg->setBlob(blobName, pandaBytes);
    ic->setBlobName(blobName);
    ic->loadFromBlob(pandaBytes, contextDir);
}

} // namespace ICTestHelpers

