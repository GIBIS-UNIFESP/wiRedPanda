// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Presentation/LabelMetrics.h"

#include <algorithm>

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

QSizeF LabelMetrics::size(const QString &text)
{
    if (text.isEmpty()) {
        return {};
    }
    QString lines = text;
    lines.replace(u'\n', QChar::LineSeparator);
    QTextLayout layout(lines, font());
    return layOut(layout).size();
}
