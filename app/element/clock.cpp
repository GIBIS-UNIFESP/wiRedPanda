// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
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

    Clock::setFrequency(1.0);
    Clock::setOff();
}

void Clock::updateClock()
{
    if (m_locked) {
        return;
    }

    if (m_reset) {
        resetClock();
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
        setFrequency(freq);

        if (version >= VERSION("3.1")) {
            stream >> m_locked;
        }
    }

    if (version >= VERSION("4.1")) {
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("frequency")) {
            setFrequency(map.value("frequency").toFloat());
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

    std::chrono::duration<float, std::milli> auxInterval = 1s / (2 * freq);

    if (auxInterval.count() <= 0) {
        return;
    }

    m_interval = auxInterval;
    m_frequency = static_cast<double>(freq);
    m_timePoint = std::chrono::steady_clock::now();
    m_reset = true;
}

void Clock::resetClock()
{
    setOn();
    m_timePoint = std::chrono::steady_clock::now();
    m_reset = false;
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
