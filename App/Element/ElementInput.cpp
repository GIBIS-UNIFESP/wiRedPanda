// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementInput.h"

#include "App/Wiring/Port.h"

void ElementInput::setOn(const bool value, const int port)
{
    Q_UNUSED(port)
    m_isOn = value;
    // Pixmap index 0 = off SVG, index 1 = on SVG (matches bool → int cast)
    setPixmap(static_cast<int>(m_isOn));
    if (auto *out = outputPort()) {
        out->setStatus(static_cast<Status>(m_isOn));
    }
}

void ElementInput::setWaveformValue(const bool value, const int port)
{
    // Default: a waveform cell drives the port directly to its value.
    setOn(value, port);
}

void ElementInput::setAppearance(const bool defaultAppearance, const QString &fileName)
{
    if (defaultAppearance) {
        m_appearance.resetAlternativeToDefault();
    } else {
        m_appearance.setAlternativeAppearanceAt(static_cast<int>(m_isOn), fileName);
    }

    m_appearance.setUsingDefaultAppearance(defaultAppearance);
    setPixmap(static_cast<int>(m_isOn));
}
