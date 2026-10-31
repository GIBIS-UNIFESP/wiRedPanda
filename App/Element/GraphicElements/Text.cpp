// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Text.h"

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

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = QStringLiteral(":/Components/Misc/text.png");
        descriptor.titleText = QT_TRANSLATE_NOOP("Text", "TEXT");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Text", "Text");
        descriptor.trContext = "Text";
        // Text has two built-in appearance assets, but only index 0 (the transparent
        // placeholder) is ever actually shown -- its presenter shows slot 0, and nothing
        // switches to index 1. The empty-state hint (TextPresenter) is a separate,
        // lighter-weight mechanism layered on top rather than an attempt to wire up this swap.
        descriptor.defaultAppearances = QStringList({
            ":/Components/Misc/no_text.png",
            ":/Components/Misc/text.png",
        });
        // Text is decorative and excluded from simulation.
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Text(); });
        return true;
    }();
};

// Text is a purely decorative annotation element with no ports and no simulation role.
Text::Text(QGraphicsItem *parent)
    : Element(ElementType::Text, parent)
{
}

void Text::languageChanged()
{
    // The hint TextPresenter draws while the label is empty is part of the body, its width the language's
    pictureChanged();
}
