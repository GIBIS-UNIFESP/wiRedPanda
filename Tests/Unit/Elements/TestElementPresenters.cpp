// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementPresenters.h"

#include <memory>

#include <QMetaEnum>
#include <QScopeGuard>
#include <QTest>

#include "App/Element/BodyMeasure.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Presentation/ElementPresenter.h"

void TestElementPresenters::testEveryTypeHasAPresenter()
{
    // Every type an element can be built as has a presenter; one that cannot (Unknown, the retired JK latch) has none.
    const QMetaEnum types = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < types.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(types.value(i));
        QCOMPARE_EQ(ElementPresenter::find(type) != nullptr, ElementFactory::hasCreator(type));
    }
}

void TestElementPresenters::testAnElementsBodyIsItsPresentersMeasure()
{
    const QMetaEnum types = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < types.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(types.value(i));
        if (!ElementFactory::hasCreator(type)) {
            continue;
        }
        const std::unique_ptr<Element> element(ElementFactory::buildElement(type));
        const BodyShape measured = ElementPresenter::of(type).measure(*element);
        QVERIFY2(element->body() == measured, qPrintable(ElementFactory::typeToText(type)));
        QVERIFY2(BodyMeasure::installed().measure(*element) == measured, qPrintable(ElementFactory::typeToText(type)));
        QVERIFY2(element->pixmapCenter() == measured.centre, qPrintable(ElementFactory::typeToText(type)));
    }
}

void TestElementPresenters::testBuildingAnElementMeasuresItsBodyOnce()
{
    // Building sets the ports, the theme and the first pictures, each a change; the constructor measures once, as it
    // ends, beside the first picture's own measure of its pivot
    class CountingMeasure final : public BodyMeasure
    {
    public:
        BodyShape measure(const Element &element) const override
        {
            ++count;
            return m_presenters.measure(element);
        }

        mutable int count = 0;

    private:
        const BodyMeasure &m_presenters = BodyMeasure::installed();
    };
    static CountingMeasure counting;
    counting.count = 0;
    BodyMeasure::install(&counting);
    const auto restore = qScopeGuard([] { installPresenters(); });

    const std::unique_ptr<Element> gate(ElementFactory::buildElement(ElementType::And));
    QCOMPARE(counting.count, 2);
}
