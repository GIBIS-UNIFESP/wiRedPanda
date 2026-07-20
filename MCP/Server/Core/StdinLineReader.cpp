// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Core/StdinLineReader.h"

#include <QDebug>

QStringList StdinLineReader::extractLines(QByteArray &buffer, const QByteArray &data)
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
    if (buffer.size() > kMaxLineBytes) {
        qWarning() << "StdinLineReader: stdin line exceeds" << kMaxLineBytes << "bytes with no newline — dropping it";
        buffer.clear();
    }

    return lines;
}
