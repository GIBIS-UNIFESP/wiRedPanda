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

/// Legacy blob registry MIME type (no explicit QDataStream version — kept for
/// reading clipboard data produced by older app versions).
constexpr const char *BlobRegistry    = "application/x-wiredpanda-blobregistry";
/// Versioned blob registry MIME type (QDataStream::Qt_5_12 encoding).
/// New app versions write this alongside BlobRegistry so that older apps can
/// still read the unversioned copy, while newer apps prefer this one.
constexpr const char *BlobRegistryV2  = "application/x-wiredpanda-blobregistry-v2";
} // namespace MimeType

