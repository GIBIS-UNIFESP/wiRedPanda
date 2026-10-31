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
#include "App/Element/ElementAppearance.h"
#include "App/Presentation/ElementPictures.h"

namespace ElementPresenterDetail {

/// Returns the rectangle the picture \a element shows takes, at the origin; empty while it shows none.
QRectF pictureRect(const Element &element)
{
    const PictureVariant *picture = ElementPictures::instance().shown(element);
    return picture ? QRectF(picture->pixmap.rect()) : QRectF();
}

/// The presenter of most types: the body is the picture the element shows, and turns about its centre; the element
/// draws what it has of its own over it (a display's lit segments, a rotary's marks).
class PicturePresenter : public ElementPresenter
{
public:
    BodyShape measure(const Element &element) const override
    {
        const QRectF picture = pictureRect(element);
        return {picture, picture.center()};
    }

    void paintBody(QPainter &painter, const Element &element, const BodyContext &context) const override
    {
        Q_UNUSED(context)
        if (const PictureVariant *picture = ElementPictures::instance().shown(element)) {
            ElementPictures::draw(painter, *picture);
        }
        element.drawBody(painter);
    }
};

/// The presenter of the inputs that show their level (InputButton, InputSwitch, Clock): off, then on.
class LevelPresenter : public PicturePresenter
{
public:
    int pictureSlot(const Element &element) const override
    {
        return element.param<bool>(ParamId::On) ? 1 : 0;
    }
};

/// The presenter of the audio outputs (Buzzer, AudioBox): silent, then playing.
class AudioPresenter : public PicturePresenter
{
public:
    int pictureSlot(const Element &element) const override
    {
        return element.param<bool>(ParamId::Playing) ? 1 : 0;
    }
};

/// The presenter of the LED: the slot its colour and inputs lit at its last refresh.
class LedPresenter : public PicturePresenter
{
public:
    int pictureSlot(const Element &element) const override
    {
        return element.param<int>(ParamId::Lit);
    }
};

/// The presenter of the segment displays: the body is their picture and the 64x64 box their lit segments are drawn in,
/// which a custom picture need not cover; it turns about the picture's centre.
class SegmentDisplayPresenter : public PicturePresenter
{
public:
    BodyShape measure(const Element &element) const override
    {
        const QRectF picture = pictureRect(element);
        return {picture.united(QRectF(0, 0, 64, 64)), picture.center()};
    }
};

/**
 * \brief The presenter of the types whose body is drawn as vectors to fit their ports (IC, Mux, Demux, TruthTable).
 * \details The body is the nominal 64x64 square, grown to wherever the ports land. It turns about the body's centre
 * once the element has built its render pixmap, and about its picture's centre before (an IC with no definition yet).
 * The element draws the whole body.
 */
class FootprintPresenter : public ElementPresenter
{
public:
    BodyShape measure(const Element &element) const override
    {
        const QRectF body = element.renderBodyBounds();
        return {body, element.appearance().hasRenderPixmap() ? body.center() : pictureRect(element).center()};
    }

    void paintBody(QPainter &painter, const Element &element, const BodyContext &context) const override
    {
        Q_UNUSED(context)
        element.drawBody(painter);
    }
};

/// The presenter of Text: the body holds its picture and its child items (the label and the empty-text hint), so a
/// click on the text hits the element; it turns about its picture's centre.
class TextPresenter : public PicturePresenter
{
public:
    BodyShape measure(const Element &element) const override
    {
        const QRectF picture = pictureRect(element);
        return {picture.united(element.childrenBoundingRect()), picture.center()};
    }
};

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
            painter.drawRoundedRect(element.boundingRect(), 5, 5);
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
        static const ElementPresenterDetail::PicturePresenter fallback;
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
    static const SegmentDisplayPresenter segmentDisplay;
    static const FootprintPresenter footprint;
    static const TextPresenter text;

    // Every type an element can have, each named: a type left out has no presenter, which TestElementPresenters finds.
    const std::initializer_list<std::pair<ElementType, const ElementPresenter *>> presenters = {
        {ElementType::And, &picture},
        {ElementType::AudioBox, &audio},
        {ElementType::Buzzer, &audio},
        {ElementType::Clock, &level},
        {ElementType::DFlipFlop, &picture},
        {ElementType::DLatch, &picture},
        {ElementType::Demux, &footprint},
        {ElementType::Display14, &segmentDisplay},
        {ElementType::Display16, &segmentDisplay},
        {ElementType::Display7, &segmentDisplay},
        {ElementType::IC, &footprint},
        {ElementType::InputButton, &level},
        {ElementType::InputGnd, &picture},
        {ElementType::InputRotary, &picture},
        {ElementType::InputSwitch, &level},
        {ElementType::InputVcc, &picture},
        {ElementType::JKFlipFlop, &picture},
        {ElementType::Led, &led},
        {ElementType::Line, &picture},
        {ElementType::Mux, &footprint},
        {ElementType::Nand, &picture},
        {ElementType::Node, &picture},
        {ElementType::Nor, &picture},
        {ElementType::Not, &picture},
        {ElementType::Or, &picture},
        {ElementType::SRFlipFlop, &picture},
        {ElementType::SRLatch, &picture},
        {ElementType::TFlipFlop, &picture},
        {ElementType::Text, &text},
        {ElementType::TruthTable, &footprint},
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
