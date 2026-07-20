// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Core/MCPProcessor.h"

#include <QDebug>

// MCPProcessor's real command-processing body (constructor, handler dispatch, stdin plumbing)
// needed MainWindow and the Scene-based handler family (ElementHandler, ConnectionHandler,
// ...), none of which compile anymore now that GraphicElement/Connection stopped being
// QGraphicsItems and the Widgets Scene stopped compiling; QuickMCPProcessor is the real,
// Quick-native replacement. extractStdinLines() is the one piece kept here: a pure, static,
// stdin-agnostic buffer/newline parser (confirmed by reading it -- no MainWindow/Scene coupling
// anywhere in its body) that QuickMCPProcessor reuses unmodified rather than duplicating.
QStringList MCPProcessor::extractStdinLines(QByteArray &buffer, const QByteArray &data)
{
    buffer.append(data);

    QStringList lines;
    qsizetype newline;
    while ((newline = buffer.indexOf('\n')) != -1) {
        lines.append(QString::fromUtf8(buffer.left(newline)));
        buffer.remove(0, newline + 1);
    }

    // A client that never sends '\n' would otherwise grow the buffer without bound —
    // stdin is not a trusted channel in --mcp/--mcp-gui mode.
    if (buffer.size() > kMaxStdinLineBytes) {
        qWarning() << "MCPProcessor: stdin line exceeds" << kMaxStdinLineBytes << "bytes with no newline — dropping it";
        buffer.clear();
    }

    return lines;
}
