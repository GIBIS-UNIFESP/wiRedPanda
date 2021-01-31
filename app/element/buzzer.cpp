// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "buzzer.h"
#include <array>

#include "qneport.h"

#include <QDebug>
#include <QGraphicsSceneDragDropEvent>

int Buzzer::current_id_number = 0;
// macOS doesn't want to compile a constexpr std::array<const char *> for some reason
#ifdef Q_OS_MACOS
static const std::array<const char *, 2> defaultSkins
{
#else
static constexpr std::array<const char *, 2> defaultSkins {
#endif
    ":/output/BuzzerOff.png", ":/output/BuzzerOn.png"
};

Buzzer::Buzzer(QGraphicsItem *parent)
    : GraphicElement(ElementType::BUZZER, ElementGroup::OUTPUT, 1, 1, 0, 0, parent)
{
    //  pixmapSkinName.append( ":/output/BuzzerOff.png" );
    //  pixmapSkinName.append( ":/output/BuzzerOn.png" );
    setOutputsOnTop(true);
    setRotatable(false);
    setHasAudio(true);
    //  setPixmap( pixmapSkinName[ 0 ] );
    setPixmap(defaultSkins[0]);
    this->m_alternativeSkins = QVector<QString>({defaultSkins[0], defaultSkins[1]});
    updatePorts();
    setCanChangeSkin(true);
    setHasLabel(true);
    // Let's reserve space for alternativeSkins :D
    // All is well, rite?
    m_alternativeSkins.resize(2);
    setPortName("Buzzer");
    setLabel(objectName() + "_" + QString::number(Buzzer::current_id_number));
    ++Buzzer::current_id_number;
    usingDefaultSkin = true;
    setAudio("C6");
    m_play = 0;
}

void Buzzer::refresh()
{
    if (!isValid()) {
        stopbuzzer();
        return;
    }

    const bool value = m_inputs.first()->value();
    if (value == 1) {
        playbuzzer();
    } else if (m_play == 1) {
        stopbuzzer();
    }
}

void Buzzer::setAudio(const QString &note)
{
    m_audio.setSource(QUrl::fromLocalFile(QString(":output/audio/%1.wav").arg(note)));
    m_audio.setVolume(0.35);
    m_audio.setLoopCount(QSoundEffect::Infinite);
    m_note = note;
}

QString Buzzer::getAudio() const
{
    return m_note;
}

void Buzzer::mute(bool _mute)
{
    m_audio.setMuted(_mute);
}

void Buzzer::playbuzzer()
{
    if (m_play != 0) {
        return;
    }

    usingDefaultSkin ? setPixmap(defaultSkins[1]) : setPixmap(m_alternativeSkins[1]);
    m_audio.play();
    m_play = 1;
}

void Buzzer::stopbuzzer()
{
    usingDefaultSkin ? setPixmap(defaultSkins[0]) : setPixmap(m_alternativeSkins[0]);
    m_play = 0;
    m_audio.stop();
}

void Buzzer::save(QDataStream &ds) const
{
    GraphicElement::save(ds);
    ds << getAudio();
}

void Buzzer::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
    if (version >= 2.4) {
        QString note;
        ds >> note;
        setAudio(note);
    }
}

void Buzzer::setSkin(bool defaultSkin, const QString &filename)
{
    if (this->m_play > 0) {
        this->m_play = 1;
    }
    if (defaultSkin) {
        usingDefaultSkin = true;
        setPixmap(defaultSkins[m_play]);
    } else {
        usingDefaultSkin = false;
        m_alternativeSkins[m_play] = filename;
        setPixmap(m_alternativeSkins[m_play]);
        //      std::cerr << "Filename: " << alternativeSkins[ play ].toStdString() << '\n';
    }
}
