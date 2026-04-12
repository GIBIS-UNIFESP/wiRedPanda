// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief MIME type string constants for drag-and-drop operations.
 */

#pragma once

/// Named constants for all MIME format strings used in drag-and-drop and clipboard operations.
namespace MimeType {
/// Current drag-and-drop MIME type for element palette → scene drops.
constexpr const char *DragDrop        = "application/x-wiredpanda-dragdrop";
/// Legacy drag-and-drop MIME type retained for backward compatibility.
constexpr const char *DragDropLegacy  = "wpanda/x-dnditemdata";

/// Current MIME type for clone-drag (Ctrl+drag within the scene).
constexpr const char *CloneDrag       = "application/x-wiredpanda-cloneDrag";
/// Legacy clone-drag MIME type retained for backward compatibility.
constexpr const char *CloneDragLegacy = "wpanda/ctrlDragData";

/// Current MIME type for clipboard copy/paste data.
constexpr const char *Clipboard       = "application/x-wiredpanda-clipboard";
/// Legacy clipboard MIME type retained for backward compatibility.
constexpr const char *ClipboardLegacy = "wpanda/copydata";

/// MIME type for the embedded IC blob registry bundled alongside clipboard data.
constexpr const char *BlobRegistry    = "application/x-wiredpanda-blobregistry";
} // namespace MimeType

