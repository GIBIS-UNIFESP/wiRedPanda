// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief The font labels are drawn in, and how their text is laid out and measured.
 */

#pragma once

#include <QRectF>
#include <QSizeF>

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

/// Returns the box \a text takes in font(), a line feed starting a line; empty for no text.
QSizeF size(const QString &text);

} // namespace LabelMetrics
