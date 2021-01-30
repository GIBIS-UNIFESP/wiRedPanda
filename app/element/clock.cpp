// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clock.h"

#include <QDebug>

bool Clock::reset = false;
int Clock::current_id_number = 0;

Clock::~Clock() = default;

Clock::Clock(QGraphicsItem *parent)
    : GraphicElement(0, 0, 1, 1, parent)
{
    pixmapSkinName.append(":/input/clock0.png");
    pixmapSkinName.append(":/input/clock1.png");
    setOutputsOnTop(false);
    setRotatable(false);
    setCanChangeSkin(true);
    /*  connect(&timer,&QTimer::timeout,this,&Clock::updateClock); */
    setFrequency(1.0); // TODO: call to virtual function during construction
    setHasFrequency(true);
    on = false;
    Clock::reset = true;
    setHasLabel(true);
    setPortName("Clock");
    setOn(0);
    setPixmap(pixmapSkinName[0]);
}

void Clock::updateClock()
{
    if (!disabled()) {
        elapsed++;
        if ((elapsed % interval) == 0) {
            setOn(!on);
        }
    }
    setOn(on);
}

bool Clock::getOn() const
{
    return (on);
}

void Clock::setOn(bool value)
{
    on = value;
    if (on) {
        setPixmap(pixmapSkinName[1]);
    } else {
        setPixmap(pixmapSkinName[0]);
    }
    m_outputs.first()->setValue(on);
}

void Clock::save(QDataStream &ds) const
{
    GraphicElement::save(ds);
    ds << getFrequency();
}

void Clock::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
    if (version >= 1.1) {
        float freq;
        ds >> freq;
        setFrequency(freq);
    }
}

float Clock::getFrequency() const
{
    return (static_cast<float>(m_frequency));
}

void Clock::setFrequency(float freq)
{
    /*  qDebug() << "Clock frequency set to " << freq; */
    if (!qFuzzyIsNull(freq)) {
        int auxinterval = 1000 / (freq * GLOBALCLK);
        if (auxinterval > 0) {
            interval = auxinterval;
            m_frequency = static_cast<double>(freq);
            elapsed = 0;
            Clock::reset = true;
            //      qDebug() << "Freq = " << freq <<  " interval = " << interval;
        }
        /*    timer.start( static_cast< int >(1000.0/freq) ); */
    }
}

void Clock::resetClock()
{
    setOn(true);
    elapsed = 0;
}

QString Clock::genericProperties()
{
    return (QString("%1 Hz").arg(static_cast<double>(getFrequency())));
}

void Clock::setSkin(bool defaultSkin, QString filename)
{
    if (defaultSkin) {
        if (!on) {
            pixmapSkinName[0] = ":/input/clock0.png";
            setPixmap(pixmapSkinName[0]);
        } else {
            pixmapSkinName[1] = ":/input/clock1.png";
            setPixmap(pixmapSkinName[1]);
        }
    } else {
        if (!on) {
            pixmapSkinName[0] = filename;
            setPixmap(pixmapSkinName[0]);
        } else {
            pixmapSkinName[1] = filename;
            setPixmap(pixmapSkinName[1]);
        }
    }
}
