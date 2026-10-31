// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief The font labels are drawn in, and how their text is laid out and measured.
 */

#pragma once

#include <QRectF>
#include <QSizeF>
#include <QtGlobal>

class QFont;
class QString;
class QTextLayout;

/// The font labels are drawn in, and how their text is laid out and measured: as QGraphicsSimpleTextItem, which draws
/// them, lays its text out.
namespace LabelMetrics {

/// Returns the font labels are drawn in: the platform's sans-serif family, in bold.
const QFont &font();

/// Lays \a layout out as QGraphicsSimpleTextItem lays its text out, each line under the last from the origin, and
/// returns the box it takes.
QRectF layOut(QTextLayout &layout);

/// Returns the box \a text takes in font(), a line feed starting a line; empty for no text. Elements share their texts
/// (each element inside an IC has its definition's names), so the boxes measured are kept by their text, those least
/// recently asked for given up past boxesBudget.
QSizeF size(const QString &text);

/// The bytes the boxes size() keeps may take, each charged its text and the rest of its entry: some six thousand labels
/// of ten letters, while a box given up costs one layout to measure again.
constexpr qsizetype boxesBudget = 1024 * 1024;

/// Returns the bytes the boxes size() keeps take, as charged against boxesBudget.
qsizetype boxesHeld();

} // namespace LabelMetrics
