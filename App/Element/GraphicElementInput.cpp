// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElementInput.h"

#include "App/Nodes/QNEPort.h"

void GraphicElementInput::setOn(const bool value, const int port)
{
    Q_UNUSED(port)
    m_isOn = value;
    // Pixmap index 0 = off SVG, index 1 = on SVG (matches bool → int cast)
    setPixmap(static_cast<int>(m_isOn));
    outputPort()->setStatus(static_cast<Status>(m_isOn));
}

void GraphicElementInput::setAppearance(const bool defaultAppearance, const QString &fileName)
{
    if (defaultAppearance) {
        m_alternativeAppearances = m_defaultAppearances;
    } else {
        m_alternativeAppearances[static_cast<int>(m_isOn)] = fileName;
    }

    m_usingDefaultAppearance = defaultAppearance;
    setPixmap(static_cast<int>(m_isOn));
}

