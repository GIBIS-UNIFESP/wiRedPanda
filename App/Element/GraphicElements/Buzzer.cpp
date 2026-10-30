// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Buzzer.h"

#include <cmath>

#include <QAudioSink>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/GraphicElements/ToneGenerator.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"

template<>
struct ElementInfo<Buzzer> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Buzzer,
        .group = ElementGroup::Output,
        .minInputSize = 1,
        .maxInputSize = 1,
        .canChangeAppearance = true,
        .hasFrequency = true,
        .hasLabel = true,
        .hasVolume = true,
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Output/Buzzer/BuzzerOff.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("Buzzer", "BUZZER");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Buzzer", "Buzzer");
        descriptor.trContext = "Buzzer";
        descriptor.defaultAppearances = QStringList({
            ":/Components/Output/Buzzer/BuzzerOff.svg",
            ":/Components/Output/Buzzer/BuzzerOn.svg",
        });
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Buzzer(); });
        return true;
    }();
};

Buzzer::Buzzer(QGraphicsItem *parent)
    : AudioOutputElement(ElementType::Buzzer, parent, kDefaultVolume)
{
    if (m_hasOutputDevice) {
        m_generator = new ToneGenerator(this);
        m_generator->setFrequency(m_frequency);
        m_sink = new QAudioSink(ToneGenerator::format(), this);
        m_sink->setVolume(static_cast<qreal>(m_volume));
    }
}

Buzzer::~Buzzer()
{
    delete m_sink;
    m_sink = nullptr;
}

double Buzzer::frequency() const
{
    return m_frequency;
}

bool Buzzer::acceptsFrequency(double freq) const
{
    // A .panda file (or any other caller) can supply a non-finite or non-positive value;
    // ToneGenerator::readData() casts a NaN-derived sample to qint16, which is undefined
    // behaviour, and a non-positive frequency has no physical meaning for a buzzer.
    return std::isfinite(freq) && (freq > 0.0);
}

void Buzzer::setFrequency(double freq)
{
    // The one setter every caller (file load, GUI, MCP, undo/redo) goes through refuses what acceptsFrequency() does
    if (!acceptsFrequency(freq)) {
        return;
    }

    m_frequency = freq;

    if (!m_hasOutputDevice) {
        return;
    }

    m_generator->setFrequency(freq);

    // If already playing, restart so the new frequency takes effect immediately
    if (m_isPlaying) {
        m_sink->stop();
        m_generator->start();
        m_sink->start(m_generator);
    }
}

void Buzzer::setAudio(const QString &note)
{
    if (note.isEmpty()) {
        return;
    }

    setFrequency(noteToFrequency(note));
}

void Buzzer::startAudio()
{
    m_generator->start();
    m_sink->start(m_generator);
}

void Buzzer::stopAudio()
{
    m_sink->stop();
    m_generator->close();
}

void Buzzer::applyVolume()
{
    if (!m_muted) {
        m_sink->setVolume(static_cast<qreal>(m_volume));
    }
}

void Buzzer::applyMute()
{
    m_sink->setVolume(m_muted ? 0.0 : static_cast<qreal>(m_volume));
}

int Buzzer::noteToFrequency(const QString &note)
{
    // Backward-compatible aliases: the scale historically listed A7/B7 (an octave above
    // their neighbours); pre-4.x files persist the note *name*, so keep resolving these
    // to their true pitches. Kept on one physical line: gcov misattributes per-entry hit
    // counts across a multi-line brace-init list (see CodeGen's chained-.arg() precedent).
    static const QHash<QString, int> map = {{"C6", 1047}, {"D6", 1175}, {"E6", 1319}, {"F6", 1397}, {"G6", 1568}, {"A6", 1760}, {"B6", 1976}, {"C7", 2093}, {"A7", 3520}, {"B7", 3951}};

    return map.value(note, 1047);
}

void Buzzer::save(QDataStream &stream, SerializationOptions options) const
{
    Element::save(stream, options);

    const bool slim = (options.purpose == SerializationPurpose::PortableFile);

    QMap<QString, QVariant> map;
    // PortableFile streams omit defaults; fresh-loaded elements start there
    // anyway. Snapshots write unconditionally (reloaded into live elements).
    if (!slim || m_frequency != kDefaultFrequency) {
        map.insert("frequency", m_frequency);
    }
    if (!slim || m_volume != kDefaultVolume) {
        map.insert("volume", static_cast<double>(m_volume));
    }

    stream << map;
}

void Buzzer::load(QDataStream &stream, SerializationContext &context)
{
    Element::load(stream, context);

    if (!VersionInfo::hasAudio(context.version)) {
        return;
    }

    if (!VersionInfo::hasQMapFormat(context.version)) {
        // v2.4–4.0 stored the note name as a bare QString
        const QString note = Serialization::readBoundedString(stream);
        setAudio(note);
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        QMap<QString, QVariant> map = Serialization::readBoundedMetadata(stream);

        // New format: frequency in Hz
        if (map.contains("frequency")) {
            setFrequency(map.value("frequency").toDouble());
        } else if (map.contains("note")) {
            // Old format: note name → convert to frequency
            setAudio(map.value("note").toString());
        }

        if (map.contains("volume")) {
            bool ok = false;
            const float vol = map.value("volume").toFloat(&ok);
            if (ok) {
                setVolume(vol);
            }
        }
    }
}
