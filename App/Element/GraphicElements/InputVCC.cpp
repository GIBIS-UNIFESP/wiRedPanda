// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputVCC.h"

#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"

InputVcc::InputVcc(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputVcc, ElementGroup::StaticInput, ":/Components/Input/1.svg", tr("VCC"), tr("VCC"), 0, 0, 1, 1, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
    if (GlobalProperties::skipInit) {
        return;
    }

    // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    setCanChangeSkin(true);
    // Static inputs are not rotatable as a whole (see InputGND.cpp for explanation).
    setRotatable(false);

    // VCC always outputs logic HIGH; set the port status at construction so the
    // visual wire colour is correct before the first simulation tick runs.
    m_outputPorts.constFirst()->setStatus(Status::Active);
}
