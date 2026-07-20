// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief StdinLineReader: buffers and splits raw stdin bytes into newline-terminated lines.
 */

#pragma once

#include <QByteArray>
#include <QStringList>

/**
 * \namespace StdinLineReader
 * \brief Pure, stdin-agnostic buffering/line-splitting logic shared by every MCP processor.
 *
 * \details Extracted from MCPProcessor -- the class itself (constructor, handler dispatch,
 * stdin plumbing) needed MainWindow and the Scene-based handler family, none of which compile
 * anymore now that GraphicElement/Connection stopped being QGraphicsItems and the Widgets
 * Scene stopped compiling. This piece never depended on either (confirmed by reading it: no
 * MainWindow/Scene coupling anywhere in its body), so it's kept here as a standalone utility
 * for QuickMCPProcessor to reuse unmodified rather than duplicate.
 */
namespace StdinLineReader {

/// Maximum bytes buffered while waiting for a newline-terminated JSON-RPC command — see
/// extractLines(). Public so tests can reference the real limit instead of a duplicated magic
/// number.
inline constexpr qint64 kMaxLineBytes = 16 * 1024 * 1024; // 16 MB

/// Appends \a data to \a buffer and returns every complete ('\n'-terminated) line found,
/// removing them from \a buffer. If more than kMaxLineBytes accumulates with no newline,
/// clears \a buffer instead of growing it without bound — stdin is not a trusted channel in
/// --mcp/--mcp-gui mode (any local process that can pipe into wiRedPanda's stdin can write it).
/// A pure function, independent of any live file descriptor, so the buffering/cap logic is
/// directly unit-testable.
QStringList extractLines(QByteArray &buffer, const QByteArray &data);

} // namespace StdinLineReader
