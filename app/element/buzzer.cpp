// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buzzer.h"

#include "common.h"
#include "qneport.h"

#include <QDebug>
#include <QGraphicsSceneDragDropEvent>
#include <array>

namespace
{
int id = qRegisterMetaType<Buzzer>();
}

Buzzer::Buzzer(QGraphicsItem *parent)
    : GraphicElement(ElementType::Buzzer, ElementGroup::Output, 1, 1, 0, 0, parent)
{
    m_defaultSkins = QStringList{
        ":/output/BuzzerOff.png",
        ":/output/BuzzerOn.png"
    };
    m_alternativeSkins = m_defaultSkins;
    setPixmap(m_defaultSkins.first());

    m_label->setPos(64, 34);
    setRotatable(false);
    setHasAudio(true);
    updatePorts();
    setCanChangeSkin(true);
    setHasLabel(true);
    setPortName("Buzzer");
    setToolTip(m_translatedName);
    setLabel(objectName() + "_" + QString::number(m_labelNumber));
    if (Common::incrementLabel) {
        ++m_labelNumber;
    }
}

void Buzzer::refresh()
{
    if (!isValid()) {
        stopBuzzer();
        return;
    }

    const bool inputValue = m_inputs.first()->value(); // TODO: why only the first input?

    inputValue ? playBuzzer() : stopBuzzer();
}

void Buzzer::setAudio(const QString &note)
{
    m_audio.setSource(QUrl::fromLocalFile(":output/audio/" + note + ".wav"));
    m_audio.setVolume(0.35f);
    m_audio.setLoopCount(QSoundEffect::Infinite);
    m_note = note;
}

QString Buzzer::audio() const
{
    return m_note;
}

void Buzzer::mute(const bool mute)
{
    m_audio.setMuted(mute);
}

void Buzzer::playBuzzer()
{
    if (m_isPlaying) {
        return;
    }

    if (m_audio.source().isEmpty()) {
        setAudio("C6");
    }

    setPixmap(m_usingDefaultSkin ? m_defaultSkins.at(1) : m_alternativeSkins.at(1));
    m_audio.play();
    m_isPlaying = true;
}

void Buzzer::stopBuzzer()
{
    if (!m_isPlaying) {
        return;
    }

    setPixmap(m_usingDefaultSkin ? m_defaultSkins.at(0) : m_alternativeSkins.at(0));
    m_audio.stop();
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
