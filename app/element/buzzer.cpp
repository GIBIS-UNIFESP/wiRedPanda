// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buzzer.h"

#include "globalproperties.h"
#include "qneport.h"

#include <QAudio>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QAudioDeviceInfo>
#else
#include <QAudioDevice>
#include <QMediaDevices>
#endif
#include <QDebug>

namespace
{
int id = qRegisterMetaType<Buzzer>();
}

Buzzer::Buzzer(QGraphicsItem *parent)
    : GraphicElement(ElementType::Buzzer, ElementGroup::Output, ":/output/BuzzerOff.svg", tr("BUZZER"), tr("Buzzer"), 1, 1, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins = QStringList{
        ":/output/BuzzerOff.svg",
        ":/output/BuzzerOn.svg"
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    m_label->setPos(64, 34);
    setRotatable(false);
    setHasAudio(true);
    updatePortsProperties();
    setCanChangeSkin(true);
    setHasLabel(true);
    setPortName(m_translatedName);
    setToolTip(m_translatedName);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_hasOutputDevice = !QAudioDeviceInfo::defaultOutputDevice().deviceName().isEmpty();

#else
    m_hasOutputDevice = !QMediaDevices::defaultAudioOutput().description().isEmpty();
#endif

    if (m_hasOutputDevice) {
        m_audio = new QSoundEffect(this);
    }
}

void Buzzer::refresh()
{
    if (!isValid()) {
        stopBuzzer();
        return;
    }

    const Status inputValue = m_inputPorts.constFirst()->status(); // TODO: why only the first input?

    (inputValue == Status::Active) ? playBuzzer() : stopBuzzer();
}

void Buzzer::setAudio(const QString &note)
{
    if (note.isEmpty()) {
        return;
    }

    m_note = note;

    if (!m_hasOutputDevice) {
        return;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_audio->setVolume(0.35);
#else
    m_audio->setVolume(0.35f);
#endif

    m_audio->setSource(QUrl::fromLocalFile(":output/audio/" + note + ".wav"));
    m_audio->setLoopCount(QSoundEffect::Infinite); // TODO: fix audio clipping when repeating
}

QString Buzzer::audio() const
{
    return m_note;
}

void Buzzer::mute(const bool mute)
{
    if (!m_hasOutputDevice) {
        return;
    }

    m_audio->setMuted(mute);
}

void Buzzer::playBuzzer()
{
    if (m_isPlaying) {
        return;
    }

    setPixmap(1);

    if (m_hasOutputDevice) {
        if (m_audio->source().isEmpty()) {
            setAudio("C6");
        }

        m_audio->play();
    }

    m_isPlaying = true;
}

void Buzzer::stopBuzzer()
{
    if (!m_isPlaying) {
        return;
    }

    setPixmap(0);

    if (m_hasOutputDevice) {
        m_audio->stop();
    }

    m_isPlaying = false;
}

void Buzzer::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    stream << audio();
}

void Buzzer::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    GraphicElement::load(stream, portMap, version);

    if (version < 2.4) {
        return;
    }

    QString note;
    stream >> note;
    setAudio(note);
}
