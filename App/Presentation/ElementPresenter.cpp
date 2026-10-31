// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Presentation/ElementPresenter.h"

#include <initializer_list>
#include <utility>

#include <QHash>

#include "App/Element/BodyPainter.h"
#include "App/Element/Element.h"
#include "App/Element/LabelMeasure.h"
#include "App/Presentation/ElementPainter.h"
#include "App/Presentation/ElementPictures.h"
#include "App/Presentation/FootprintPresenters.h"
#include "App/Presentation/LabelMetrics.h"
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

/// The model's LabelMeasure: each element's label is placed by its type's presenter, and its text takes the box the
/// label font gives it (LabelMetrics).
class PresenterLabelMeasure final : public LabelMeasure
{
public:
    LabelShape measure(const Element &element) const override
    {
        const LabelPlacement placement = ElementPresenter::of(element.elementType()).label(element);
        return {placement.anchor, placement.rotation, LabelMetrics::size(element.labelShown())};
    }

    const QFont &font() const override
    {
        return LabelMetrics::font();
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

LabelPlacement ElementPresenter::label(const Element &element) const
{
    Q_UNUSED(element)
    return {QPointF(0, 64)};
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
    static const PresenterLabelMeasure labelMeasure;
    BodyMeasure::install(&measure);
    BodyPainter::install(&ElementPainter::inCurrentTheme());
    LabelMeasure::install(&labelMeasure);
    PictureLibrary::install(&ElementPictures::instance());
}
