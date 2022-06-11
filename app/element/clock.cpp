// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clock.h"

#include "qneport.h"

namespace
{
int id = qRegisterMetaType<Clock>();
}

Clock::Clock(QGraphicsItem *parent)
    : GraphicElementInput(ElementType::Clock, ElementGroup::Input, 0, 0, 1, 1, parent)
{
    if (GlobalProperties::skipInit) { return; }

    m_defaultSkins = QStringList{
        ":/input/clock0.png",
        ":/input/clock1.png"
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    m_locked = false;
    setRotatable(false);
    setCanChangeSkin(true);
    Clock::setFrequency(1.0);
    setHasFrequency(true);
    setHasLabel(true);
    setPortName("Clock");
    setToolTip(m_translatedName);
    Clock::setOn(false);
}

void Clock::updateClock()
{
    if ((!m_locked) && (!isDisabled()) && (!pause)) {
        m_elapsed++;
        if ((m_elapsed % m_interval) == 0) {
            m_elapsed = 0;
            setOn(!m_isOn);
        }
    }
}

bool Clock::on(const int port) const
{
    Q_UNUSED(port);
    return m_isOn;
}

void Clock::setOff()
{
    setOn(false);
}

void Clock::setOn()
{
    setOn(true);
}

void Clock::setOn(const bool value, const int port)
{
    Q_UNUSED(port);
    m_isOn = value;
    setPixmap(m_isOn);
    m_outputs.first()->setValue(static_cast<Status>(m_isOn));
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

    // FIXME: auxInterval gets the same value for many frequencies
    int auxInterval = static_cast<int>(500 / (freq * GlobalProperties::globalClock));
    if (auxInterval <= 0) {
        return;
    }

    m_interval = auxInterval;
    m_frequency = static_cast<double>(freq);
    m_elapsed = 0;
    reset = true;
}

void Clock::resetClock()
{
    setOn(true);
    m_elapsed = 0;
}

QString Clock::genericProperties()
{
    return QString::number(frequency()) + " Hz";
}

void Clock::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_usingDefaultSkin = defaultSkin;
    m_alternativeSkins[m_isOn] = fileName;
    setPixmap(m_isOn);
}
