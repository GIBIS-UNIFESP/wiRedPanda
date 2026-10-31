// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Presentation/ElementPresenter.h"

#include <initializer_list>
#include <utility>

#include <QHash>
#include <QPainter>
#include <QPen>

#include "App/Core/ThemeManager.h"
#include "App/Element/BodyPainter.h"
#include "App/Element/Element.h"
#include "App/Presentation/ElementPictures.h"
#include "App/Presentation/FootprintPresenters.h"
#include "App/Presentation/PicturePresenters.h"

namespace ElementPresenterDetail {

/// The model's BodyMeasure: each element is measured by its type's presenter.
class PresenterBodyMeasure final : public BodyMeasure
{
public:
    BodyShape measure(const Element &element) const override
    {
        return ElementPresenter::of(element.elementType()).measure(element);
    }
};

/// The model's BodyPainter: the selection, drawn the same for every type, then the body, drawn by the type's
/// presenter.
class PresenterBodyPainter final : public BodyPainter
{
public:
    void paint(QPainter &painter, const Element &element) const override
    {
        const ThemeAttributes &theme = ThemeManager::attributes();

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
};

/// The presenter of each type, as installPresenters() registers them.
QHash<ElementType, const ElementPresenter *> &registry()
{
    static QHash<ElementType, const ElementPresenter *> presenters;
    return presenters;
}

} // namespace ElementPresenterDetail

int ElementPresenter::pictureSlot(const Element &element) const
{
    Q_UNUSED(element)
    return 0;
}

const ElementPresenter *ElementPresenter::find(const ElementType type)
{
    return ElementPresenterDetail::registry().value(type, nullptr);
}

const ElementPresenter &ElementPresenter::of(const ElementType type)
{
    const ElementPresenter *presenter = find(type);
    Q_ASSERT_X(presenter, "ElementPresenter::of()", "a type without a presenter: installPresenters() lists every type");
    if (!presenter) {
        static const PicturePresenter fallback;
        return fallback; // LCOV_EXCL_LINE -- a broken invariant, unreachable while installPresenters() lists every type
    }
    return *presenter;
}

void installPresenters()
{
    using namespace ElementPresenterDetail;
    static const PicturePresenter picture;
    static const LevelPresenter level;
    static const AudioPresenter audio;
    static const LedPresenter led;
    static const RotaryPresenter rotary;
    static const NodePresenter node;
    // Each input's segment, as the slot of the display's default appearance list
    static const SegmentDisplayPresenter display7({7, 6, 5, 4, 1, 2, 8, 3});
    static const SegmentDisplayPresenter display14({7, 6, 5, 4, 1, 2, 15, 3, 8, 9, 10, 11, 12, 13, 14});
    static const SegmentDisplayPresenter display16({9, 8, 7, 5, 6, 1, 2, 3, 17, 4, 10, 11, 12, 13, 14, 15, 16});
    static const MuxPresenter mux;
    static const DemuxPresenter demux;
    static const TruthTablePresenter truthTable;
    static const ICPresenter ic;
    static const TextPresenter text;

    // Every type an element can have, each named: a type left out has no presenter, which TestElementPresenters finds.
    const std::initializer_list<std::pair<ElementType, const ElementPresenter *>> presenters = {
        {ElementType::And, &picture},
        {ElementType::AudioBox, &audio},
        {ElementType::Buzzer, &audio},
        {ElementType::Clock, &level},
        {ElementType::DFlipFlop, &picture},
        {ElementType::DLatch, &picture},
        {ElementType::Demux, &demux},
        {ElementType::Display14, &display14},
        {ElementType::Display16, &display16},
        {ElementType::Display7, &display7},
        {ElementType::IC, &ic},
        {ElementType::InputButton, &level},
        {ElementType::InputGnd, &picture},
        {ElementType::InputRotary, &rotary},
        {ElementType::InputSwitch, &level},
        {ElementType::InputVcc, &picture},
        {ElementType::JKFlipFlop, &picture},
        {ElementType::Led, &led},
        {ElementType::Line, &picture},
        {ElementType::Mux, &mux},
        {ElementType::Nand, &picture},
        {ElementType::Node, &node},
        {ElementType::Nor, &picture},
        {ElementType::Not, &picture},
        {ElementType::Or, &picture},
        {ElementType::SRFlipFlop, &picture},
        {ElementType::SRLatch, &picture},
        {ElementType::TFlipFlop, &picture},
        {ElementType::Text, &text},
        {ElementType::TruthTable, &truthTable},
        {ElementType::Xnor, &picture},
        {ElementType::Xor, &picture},
    };
    for (const auto &[type, presenter] : presenters) {
        registry().insert(type, presenter);
    }

    static const PresenterBodyMeasure measure;
    static const PresenterBodyPainter painter;
    BodyMeasure::install(&measure);
    BodyPainter::install(&painter);
    PictureLibrary::install(&ElementPictures::instance());
}
