// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/ClipboardManager.h"

#include <QMimeData>

#include "App/Core/MimeTypes.h"

bool ClipboardManager::canPaste(const QMimeData *mimeData)
{
    // Mirrors the formats a real paste reads — copy()/cut() write only the
    // current format, while older app versions wrote the legacy one.
    return mimeData
           && (mimeData->hasFormat(MimeType::Clipboard) || mimeData->hasFormat(MimeType::ClipboardLegacy));
}
