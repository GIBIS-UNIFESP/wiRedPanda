// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Presentation/ElementPainter.h"

#include <QPainter>
#include <QPen>

#include "App/Core/ThemeManager.h"
#include "App/Element/BodyPainter.h"
#include "App/Element/Element.h"
#include "App/Presentation/ElementPresenter.h"

namespace ElementPainterDetail {

/// The model's BodyPainter: an element drawn in the current theme.
class CurrentThemeBodyPainter final : public BodyPainter
{
public:
    void paint(QPainter &painter, const Element &element) const override
    {
        ElementPainter::paint(painter, element, ThemeManager::attributes());
    }
};

} // namespace ElementPainterDetail

void ElementPainter::paint(QPainter &painter, const Element &element, const ThemeAttributes &theme)
{
    if (element.isSelected()) {
        painter.save();
        painter.setBrush(theme.m_selectionBrush);
        // 0.5 pen width keeps the selection outline thin regardless of zoom level
        painter.setPen(QPen(theme.m_selectionPen, 0.5, Qt::SolidLine));
        // Corner radius of 5 matches the visual rounding used on element bodies
        painter.drawRoundedRect(element.outlineRect(), 5, 5);
        painter.restore();
    }

    ElementPresenter::of(element.elementType()).paintBody(painter, element, BodyContext{theme});
}

const BodyPainter &ElementPainter::inCurrentTheme()
{
    static const ElementPainterDetail::CurrentThemeBodyPainter painter;
    return painter;
}
