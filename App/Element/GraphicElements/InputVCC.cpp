// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputVCC.h"

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicSource.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<InputVcc> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::InputVcc,
        .group = ElementGroup::StaticInput,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeSkin = true,
        // Static inputs are not rotatable as a whole (see InputGND.cpp for explanation).
        .rotatable = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Input/1.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("InputVcc", "VCC");
        meta.translatedName = QT_TRANSLATE_NOOP("InputVcc", "VCC");
        meta.trContext = "InputVcc";
        meta.defaultSkins = QStringList({":/Components/Input/1.svg"});
        meta.logicCreator = [](GraphicElement *) { return std::make_shared<LogicSource>(true); };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new InputVcc(); });
        return true;
    }();
};

InputVcc::InputVcc(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputVcc, parent)
{
    // VCC always outputs logic HIGH; set the port status at construction so the
    // visual wire colour is correct before the first simulation tick runs.
    m_outputPorts.constFirst()->setStatus(Status::Active);
}

