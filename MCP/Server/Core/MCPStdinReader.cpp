// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief StdinReader's implementation and QuickMCPProcessor::extractStdinLines() -- originally
 * split out of the now-removed Widgets-only MCPProcessor.cpp so this pure line-buffering/
 * threading logic (zero MainWindow dependency) compiled into the portable,
 * Widgets-independent SOURCES list; moved onto QuickMCPProcessor once that was its last
 * surviving dependent (see QuickMCPProcessor.h's own doc comment).
 */

#include "MCP/Server/Core/QuickMCPProcessor.h"

#include <iostream>
#include <string>

#include <QDebug>

#ifdef Q_OS_WIN
// StdinReader implementation (Windows only — see header).
StdinReader::StdinReader(QObject *parent)
    : QThread(parent)
{
}

void StdinReader::requestStop()
{
    m_stopRequested = true;
}

void StdinReader::run()
{
    // Blocks reading one character at a time; QuickMCPProcessor::stopProcessing closes the
    // stdin handle to unblock it, so the loop exits on EOF without QThread::terminate().
    // A manual char loop (rather than std::getline, which has no length bound) lets a
    // line past QuickMCPProcessor::kMaxStdinLineBytes be dropped instead of growing std::string without limit.
    std::string line;
    while (!m_stopRequested) {
        line.clear();
        bool overflowed = false;
        int ch = std::char_traits<char>::eof();

        while (!m_stopRequested) {
            ch = std::cin.get();
            if (ch == std::char_traits<char>::eof() || ch == '\n') {
                break;
            }
            if (static_cast<qint64>(line.size()) < QuickMCPProcessor::kMaxStdinLineBytes) {
                line.push_back(static_cast<char>(ch));
            } else {
                overflowed = true; // keep draining this line without growing it further
            }
        }

        if (std::cin.eof() && line.empty() && !overflowed) {
            break; // clean EOF, nothing pending
        }

        if (overflowed) {
            qWarning() << "QuickMCPProcessor: stdin line exceeds" << QuickMCPProcessor::kMaxStdinLineBytes << "bytes with no newline — dropping it";
        } else if (!line.empty()) {
            emit dataReceived(QString::fromStdString(line));
        }

        if (ch == std::char_traits<char>::eof()) {
            break;
        }
    }
}
#endif

QStringList QuickMCPProcessor::extractStdinLines(QByteArray &buffer, const QByteArray &data)
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
        qWarning() << "QuickMCPProcessor: stdin line exceeds" << kMaxStdinLineBytes << "bytes with no newline — dropping it";
        buffer.clear();
    }

    return lines;
}
