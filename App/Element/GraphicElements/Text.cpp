// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Text.h"

#include <QFont>

#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<Text> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Text,
        .group = ElementGroup::Other,
        .hasLabel = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Misc/text.png"); };
        meta.titleText = QT_TRANSLATE_NOOP("Text", "TEXT");
        meta.translatedName = QT_TRANSLATE_NOOP("Text", "Text");
        meta.trContext = "Text";
        // Text has two built-in appearance assets, but only index 0 (the transparent
        // placeholder) is ever actually shown -- refresh() always reloads index 0, and nothing
        // switches to index 1. The empty-state hint (see labelContentChanged()) is a separate,
        // lighter-weight mechanism layered on top rather than an attempt to wire up this swap.
        meta.defaultAppearances = QStringList({
            ":/Components/Misc/no_text.png",
            ":/Components/Misc/text.png",
        });
        // Text is decorative and excluded from simulation.
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Text(); });
        return true;
    }();
};

// Text is a purely decorative annotation element with no ports and no simulation role.
Text::Text(QObject *parent)
    : GraphicElement(ElementType::Text, parent)
{
    QFont hintFont = m_emptyHint.font();
    hintFont.setItalic(true);
    m_emptyHint.setFont(hintFont);
    // Matches the default label anchor (GraphicElement's own constructor, since Text never
    // calls setLabelAnchor() to override it) -- the hint stands in for the label until there's
    // real content to show.
    m_emptyHint.setPos(QPointF(0, 64));
    m_emptyHint.setText(tr("Double-click to add text"));
    updateTheme();
    labelContentChanged(); // a fresh Text starts with an empty label -- show the hint explicitly
                            // rather than relying on GraphicElementLabel's default visibility
}

QRectF Text::boundingRect() const
{
    QRectF bounds = GraphicElement::boundingRect();

    // Union in the label and empty-hint child items' own local footprints -- mirrors what
    // QGraphicsItem::childrenBoundingRect() used to fold in for free. Exactly one of the two
    // ever has real text at a time (labelContentChanged() toggles between them), and
    // GraphicElementLabel::boundingRect() is already empty for empty text, so no visibility
    // check is needed here.
    for (const GraphicElementLabel *child : {labelItem(), emptyHintItem()}) {
        QTransform t;
        t.translate(child->pos().x(), child->pos().y());
        t *= child->transform();
        bounds = bounds.united(t.mapRect(child->boundingRect()));
    }

    return bounds;
}

void Text::updateTheme()
{
    GraphicElement::updateTheme();

    // A faded, italic variant of the label color -- distinguishable from real content at a
    // glance, but not so loud it reads as an actual (empty) label.
    QColor hintColor = ThemeManager::attributes().m_graphicElementLabelColor;
    hintColor.setAlpha(140);
    m_emptyHint.setBrush(hintColor);
}

void Text::labelContentChanged()
{
    m_emptyHint.setVisible(label().isEmpty());
}
