// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Presentation/LabelMetrics.h"

#include <algorithm>

#include <QCache>
#include <QFont>
#include <QString>
#include <QTextLayout>

const QFont &LabelMetrics::font()
{
    // Constructed once, which spares repeated fontconfig lookups
    static const QFont font = [] {
        // The platform's own sans-serif family: a family it lacks ("Sans Serif" outside fontconfig) makes Qt build its
        // whole table of family aliases on the first label
        QFont hint;
        hint.setStyleHint(QFont::SansSerif);
        QFont f(hint.defaultFamily());
        f.setBold(true);
        return f;
    }();
    return font;
}

QRectF LabelMetrics::layOut(QTextLayout &layout)
{
    layout.setCacheEnabled(true);
    layout.beginLayout();
    while (layout.createLine().isValid()) {
    }
    layout.endLayout();
    qreal width = 0;
    qreal y = 0;
    for (int i = 0; i < layout.lineCount(); ++i) {
        QTextLine line = layout.lineAt(i);
        width = std::max(width, line.naturalTextWidth());
        line.setPosition(QPointF(0, y));
        y += line.height();
    }
    return {0, 0, width, y};
}

namespace LabelMetricsDetail {

// Measured from the GUI thread alone, as every label is
QCache<QString, QSizeF> &boxes()
{
    static QCache<QString, QSizeF> cache(LabelMetrics::boxesBudget);
    return cache;
}

// A box's charge: its text's characters, and the 140 bytes of heap measured for the rest of each entry (the box, the
// key's header, QCache's links and hash slot) with ten thousand texts of 10 and 40 characters
qsizetype charge(const QString &text)
{
    constexpr qsizetype entry = 140;
    return (text.size() * qsizetype(sizeof(QChar))) + entry;
}

} // namespace LabelMetricsDetail

QSizeF LabelMetrics::size(const QString &text)
{
    if (text.isEmpty()) {
        return {};
    }
    if (const QSizeF *box = LabelMetricsDetail::boxes().object(text)) {
        return *box;
    }
    QString lines = text;
    lines.replace(u'\n', QChar::LineSeparator);
    QTextLayout layout(lines, font());
    const QSizeF box = layOut(layout).size();
    LabelMetricsDetail::boxes().insert(text, new QSizeF(box), LabelMetricsDetail::charge(text));
    return box;
}

qsizetype LabelMetrics::boxesHeld()
{
    return LabelMetricsDetail::boxes().totalCost();
}
