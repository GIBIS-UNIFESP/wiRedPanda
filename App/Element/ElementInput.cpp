// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ElementInput.h"

#include "App/Wiring/Port.h"

ParamSpec ElementInput::levelSpec(const char *key)
{
    return {.id = ParamId::On, .defaultValue = false, .key = key};
}

ParamSpec ElementInput::lockSpec()
{
    return {.id = ParamId::Locked, .defaultValue = false, .key = "locked"};
}

void ElementInput::setOn(const bool value, const int port)
{
    Q_UNUSED(port)
    if (!setParam(ParamId::On, value)) {
        showLevel();
    }
}

void ElementInput::parameterChanged(const ParamId id, const ParamValue &previous)
{
    Element::parameterChanged(id, previous);
    if (id == ParamId::On) {
        showLevel();
    }
}

void ElementInput::showLevel()
{
    const bool on = param<bool>(ParamId::On);
    // Pixmap index 0 = off SVG, index 1 = on SVG (matches bool → int cast)
    setPixmap(static_cast<int>(on));
    if (auto *out = outputPort()) {
        out->setStatus(static_cast<Status>(on));
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
        m_appearance.setAlternativeAppearanceAt(static_cast<int>(isOn()), fileName);
    }

    m_appearance.setUsingDefaultAppearance(defaultAppearance);
    setPixmap(static_cast<int>(isOn()));
}
