// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ClipboardManager: paste-gating logic shared by every canvas frontend.
 */

#pragma once

class QMimeData;

/**
 * \class ClipboardManager
 * \brief Static paste-gating rule, shared by every canvas frontend.
 *
 * \details Originally also owned the interactive copy/cut/paste/clone-drag workflow for the
 * Qt Widgets Scene; that half is gone now that GraphicElement/Port/Connection aren't
 * QGraphicsItems (CanvasItem.cpp reimplements the equivalent copy/cut/paste/duplicate/
 * clone-drag workflow directly, reusing only this one static rule). Kept as a static-only
 * utility class rather than a free function to avoid a call-site rename.
 */
class ClipboardManager
{
public:
    /**
     * \brief Returns whether \a mimeData carries circuit data a paste action accepts.
     * \details Single source of truth for paste gating (context menu, action enabling):
     * a real paste reads both the current and the legacy mime format, so any gate that
     * checks only one of them silently drifts.
     */
    static bool canPaste(const QMimeData *mimeData);
};
