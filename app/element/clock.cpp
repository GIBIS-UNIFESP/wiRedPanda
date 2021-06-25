// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clock.h"
#include "common.h"
#include "globalproperties.h"
#include "qneport.h"

#include <QDebug>

bool Clock::reset = false;
int Clock::current_id_number = 0;

Clock::~Clock() = default;

Clock::Clock(QGraphicsItem *parent)
    : GraphicElement(ElementType::CLOCK, ElementGroup::INPUT, 0, 0, 1, 1, parent)
{
    COMMENT("Creating clock.", 0);
    m_pixmapSkinName = {
        ":/input/clock0.png",
        ":/input/clock1.png"
    };

    setOutputsOnTop(false);
    setRotatable(false);
    setCanChangeSkin(true);
    /*  connect(&timer,&QTimer::timeout,this,&Clock::updateClock); */
    setFrequency(1.0); // TODO: call to virtual function during construction
    setHasFrequency(true);
    m_isOn = false;
    Clock::reset = true;
    setHasLabel(true);
    setPortName("Clock");
    setOn(false);
    setPixmap(m_pixmapSkinName[0]);
}

void Clock::updateClock()
{
    if (!disabled()) {
        m_elapsed++;
        if ((m_elapsed % m_interval) == 0) {
            setOn(!m_isOn);
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
}

void Clock::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
    if (version < 1.1) {
        return;
    }
    float freq;
    ds >> freq;
    setFrequency(freq);
}

float Clock::getFrequency() const
{
    return static_cast<float>(m_frequency);
}

void Clock::setFrequency(float freq)
{
    /*  qDebug() << "Clock frequency set to " << freq; */
    if (qFuzzyIsNull(freq)) {
        return;
    }

    int auxinterval = 1000 / (freq * GLOBALCLK);
    if (auxinterval <= 0) {
        return;
    }

    m_interval = auxinterval;
    m_frequency = static_cast<double>(freq);
    m_elapsed = 0;
    Clock::reset = true;
    //      qDebug() << "Freq = " << freq <<  " interval = " << interval;
    /*    timer.start( static_cast< int >(1000.0/freq) ); */
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
