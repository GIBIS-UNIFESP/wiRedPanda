// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clock.h"

#include "globalproperties.h"
#include "qneport.h"

#include <chrono>

using namespace std::chrono_literals;

Clock::Clock(QGraphicsItem *parent)
    : GraphicElementInput(ElementType::Clock, ElementGroup::Input, ":/input/clock1.svg", tr("CLOCK SIGNAL"), tr("Clock"), 0, 0, 1, 1, parent)
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

    setCanChangeSkin(true);
    setHasFrequency(true);
    setHasLabel(true);
    setRotatable(false);
    setHasDelay(true);

    Clock::setFrequency(1.0);
    Clock::setDelay(0.0);
    Clock::setOff();
}

void Clock::updateClock(const std::chrono::steady_clock::time_point &globalTime)
{
    if (m_locked) {
        return;
    }

    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(globalTime - m_startTime);

    if (elapsed > m_interval) {
        m_startTime += m_interval;
        setOn(!m_isOn);
    }
}

bool Clock::isOn(const int port) const
{
    Q_UNUSED(port)
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
    Q_UNUSED(port)
    m_isOn = value;
    setPixmap(static_cast<int>(m_isOn));
    outputPort()->setStatus(static_cast<Status>(m_isOn));
}

void Clock::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("frequency", frequency());
    map.insert("delay", delay());
    map.insert("locked", m_locked);

    stream << map;
}

void Clock::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    if (version < VERSION("1.1")) {
        return;
    }

    if (version < VERSION("4.1")) {
        float freq; stream >> freq;
        float delay; stream >> delay;
        setFrequency(freq);
        setDelay(delay);

        if (version >= VERSION("3.1")) {
            stream >> m_locked;
        }
    }

    if (version >= VERSION("4.1")) {
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("frequency")) {
            setFrequency(map.value("frequency").toFloat());
        }

        if (map.contains("delay")) {
            setDelay(map.value("delay").toFloat());
        }

        if (map.contains("locked")) {
            m_locked = map.value("locked").toBool();
        }
    }
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

    std::chrono::microseconds auxInterval = std::chrono::duration_cast<std::chrono::microseconds>(1s / (2 * freq));

    if (auxInterval.count() <= 0) {
        return;
    }

    m_interval = auxInterval;
    m_frequency = static_cast<double>(freq);
}

float Clock::delay() const
{
    return static_cast<float>(m_delay);
}

void Clock::setDelay(const float delay)
{
    m_delay = static_cast<double>(delay);
}

void Clock::resetClock(const std::chrono::steady_clock::time_point &globalTime)
{
    setOn();
    auto delay_ms = static_cast<uint>(m_delay * 1000);
    m_startTime = globalTime;
    m_startTime += std::chrono::milliseconds(delay_ms);
}

QString Clock::genericProperties()
{
    return QString::number(frequency()) + " Hz";
}

void Clock::setSkin(const bool defaultSkin, const QString &fileName)
{
    if (defaultSkin) {
        m_alternativeSkins = m_defaultSkins;
    } else {
        m_alternativeSkins[static_cast<int>(m_isOn)] = fileName;
    }

    m_usingDefaultSkin = defaultSkin;
    setPixmap(static_cast<int>(m_isOn));
}
