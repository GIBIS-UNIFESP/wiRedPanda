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
    qCDebug(zero) << "Creating buzzer.";

    m_pixmapSkinName = QStringList{
        ":/output/BuzzerOff.png",
        ":/output/BuzzerOn.png"
    };
    setPixmap(m_pixmapSkinName.first());

    setOutputsOnTop(true);
    setRotatable(false);
    setHasAudio(true);
    m_alternativeSkins = m_pixmapSkinName;
    updatePorts();
    setCanChangeSkin(true);
    setHasLabel(true);
    setPortName("Buzzer");
    setToolTip(m_translatedName);
    setLabel(objectName() + "_" + QString::number(m_labelNumber));
    if (Common::incrementLabel) {
        ++m_labelNumber;
    }
    Buzzer::setAudio("C6");
}

void Buzzer::refresh()
{
    if (!isValid()) {
        stopbuzzer();
        return;
    }

    const bool value = m_inputs.first()->value(); // why only the first input?
    if (value == 1) {
        playbuzzer();
    } else if (m_play) { // move this up?
        stopbuzzer();
    }
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

void Buzzer::playbuzzer()
{
    if (m_play) {
        return;
    }

    setPixmap(m_usingDefaultSkin ? m_pixmapSkinName.at(1) : m_alternativeSkins.at(1));
    m_audio.play();
    m_play = true;
}

void Buzzer::stopbuzzer()
{
    setPixmap(m_usingDefaultSkin ? m_pixmapSkinName.at(0) : m_alternativeSkins.at(0));
    m_audio.stop();
    m_play = false;
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

void Buzzer::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_usingDefaultSkin = defaultSkin;
    m_alternativeSkins[m_play] = fileName;
    setPixmap(defaultSkin ? m_pixmapSkinName.at(m_play) : m_alternativeSkins.at(m_play));
}
