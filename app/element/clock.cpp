// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clock.h"

#include "globalproperties.h"
#include "qneport.h"

#include <chrono>

using namespace std::chrono_literals;

namespace
{
int id = qRegisterMetaType<Clock>();
}

Clock::Clock(QGraphicsItem *parent)
    : GraphicElementInput(ElementType::Clock, ElementGroup::Input, ":/input/clock1.svg", tr("<b>CLOCK SIGNAL</b>"), tr("Clock"), 0, 0, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/input/clock0.svg",
        ":/input/clock1.svg"
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    m_locked = false;
    setRotatable(false);
    setCanChangeSkin(true);
    Clock::setFrequency(1.0);
    setHasFrequency(true);
    setHasLabel(true);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);
    Clock::setOff();
}

void Clock::updateClock()
{
    if (m_locked) {
        return;
    }

    const auto duration = std::chrono::duration<float, std::micro>(std::chrono::steady_clock::now() - m_timePoint);

    if (duration > m_interval) {
        m_timePoint = std::chrono::steady_clock::now();
        setOn(!m_isOn);
    }
}

bool Clock::isOn(const int port) const
{
    Q_UNUSED(port);
    return m_isOn;
}

void Clock::setOff()
{
    Clock::setOn(false);
}

void Clock::setOn()
{
    Clock::setOn(true);
}

void Clock::setOn(const bool value, const int port)
{
    Q_UNUSED(port);
    m_isOn = value;
    setPixmap(static_cast<int>(m_isOn));
    m_outputPorts.constFirst()->setStatus(static_cast<Status>(m_isOn));
    update();
}

void Clock::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    stream << frequency();
    stream << m_locked;
}

void Clock::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    GraphicElement::load(stream, portMap, version);

    if (version < 1.1) {
        return;
    }

    float freq;
    stream >> freq;

    if (version >= 3.1) {
        stream >> m_locked;
    }

    setFrequency(freq);
}

float Clock::frequency() const
{
    return static_cast<float>(m_frequency);
}

void Clock::setFrequency(const float freq)
{
    if (qFuzzyIsNull(freq)) {
        return;
    }

    std::chrono::duration<float, std::milli> auxInterval = 1s / (2 * freq);

    if (auxInterval.count() <= 0) {
        return;
    }

    m_interval = auxInterval;
    m_frequency = static_cast<double>(freq);
    m_timePoint = std::chrono::steady_clock::now();
    reset = true;
}

void Clock::resetClock()
{
    setOn(true);
    m_timePoint = std::chrono::steady_clock::now();
}

QString Clock::genericProperties()
{
    return QString::number(frequency()) + " Hz";
}

void Clock::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_usingDefaultSkin = defaultSkin;
    m_alternativeSkins[static_cast<int>(m_isOn)] = fileName;
    setPixmap(static_cast<int>(m_isOn));
}
