// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clock.h"
#include "common.h"
#include "globalproperties.h"
#include "qneport.h"

#include <QDebug>

bool Clock::reset = false;
bool Clock::pause = false;
int Clock::current_id_number = 0;

Clock::~Clock() = default;

Clock::Clock(QGraphicsItem *parent)
    : GraphicElement(ElementType::CLOCK, ElementGroup::INPUT, 0, 0, 1, 1, parent)
{
    COMMENT("Creating clock.", 0);
    locked = false;
    m_pixmapSkinName = {
        ":/input/clock0.png",
        ":/input/clock1.png"
    };

    setOutputsOnTop(false);
    setRotatable(false);
    setCanChangeSkin(true);
    Clock::setFrequency(1.0);
    setHasFrequency(true);
    m_isOn = false;
    Clock::reset = true;
    Clock::pause = false;
    setHasLabel(true);
    setPortName("Clock");
    Clock::setOn(false);
    setPixmap(m_pixmapSkinName[0]);
}

void Clock::updateClock()
{
    if ((!locked)&&(!disabled())&&(!Clock::pause)) {
        m_elapsed++;
        if ((m_elapsed % m_interval) == 0) {
            setOn(!m_isOn);
            return;
        }
    }
    setOn(m_isOn);
}

bool Clock::getOn(int port) const
{
    Q_UNUSED(port);
    return m_isOn;
}

void Clock::setOn(bool value, int port)
{
    Q_UNUSED(port);
    m_isOn = value;
    setPixmap(m_pixmapSkinName[m_isOn ? 1 : 0]);
    m_outputs.first()->setValue(m_isOn);
}

void Clock::save(QDataStream &ds) const
{
    GraphicElement::save(ds);
    ds << getFrequency();
    ds << locked;
}

void Clock::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
    if (version < 1.1) {
        return;
    }
    float freq;
    ds >> freq;
    if (version >= 3.1) {
        ds >> locked;
    }
    setFrequency(freq);
}

float Clock::getFrequency() const
{
    return static_cast<float>(m_frequency);
}

void Clock::setFrequency(float freq)
{
    if (qFuzzyIsNull(freq)) {
        return;
    }

    int auxinterval = 500 / (freq * GLOBALCLK);
    if (auxinterval <= 0) {
        return;
    }

    m_interval = auxinterval;
    m_frequency = static_cast<double>(freq);
    m_elapsed = 0;
    Clock::reset = true;
}

void Clock::resetClock()
{
    setOn(true);
    m_elapsed = 0;
}

QString Clock::genericProperties()
{
    return QString("%1 Hz").arg(static_cast<double>(getFrequency()));
}

void Clock::setSkin(bool defaultSkin, const QString &filename)
{
    if (!m_isOn) {
        m_pixmapSkinName[0] = defaultSkin ? ":/input/clock0.png" : filename;
        setPixmap(m_pixmapSkinName[0]);
    } else {
        m_pixmapSkinName[1] = defaultSkin ? ":/input/clock1.png" : filename;
        setPixmap(m_pixmapSkinName[1]);
    }
}
