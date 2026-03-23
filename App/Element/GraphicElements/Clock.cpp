// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Clock.h"

#include <chrono>

#include "App/Element/ElementInfo.h"
#include "App/IO/VersionInfo.h"
#include "App/Nodes/QNEPort.h"

using namespace std::chrono_literals;

template<>
struct ElementInfo<Clock> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Clock,
        .group = ElementGroup::Input,
        .minInputSize = 0,
        .maxInputSize = 0,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeSkin = true,
        .hasColors = false,
        .hasAudio = false,
        .hasAudioBox = false,
        .hasTrigger = false,
        .hasFrequency = true,
        .hasDelay = true,
        .hasLabel = true,
        .hasTruthTable = false,
        .rotatable = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Input/clock1.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Clock", "CLOCK SIGNAL");
        meta.translatedName = QT_TRANSLATE_NOOP("Clock", "Clock");
        meta.trContext = "Clock";
        meta.defaultSkins = QStringList({
            ":/Components/Input/clock0.svg",
            ":/Components/Input/clock1.svg",
        });
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Clock(); });
        return true;
    }();
};

Clock::Clock(QGraphicsItem *parent)
    : GraphicElementInput(ElementType::Clock, parent)
{
    m_locked = false;
    Clock::setFrequency(1.0);  // 1 Hz default → 500 ms half-period
    Clock::setDelay(0.0);       // no phase offset by default
    Clock::setOff();             // start LOW; resetClock() will start HIGH when simulation begins
}

void Clock::updateClock(std::chrono::steady_clock::time_point globalTime)
{
    if (m_locked) {
        return;
    }

    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(globalTime - m_startTime);

    // m_interval is the half-period (time per HIGH or LOW phase).
    // Rather than resetting to globalTime we advance by exactly one interval so
    // that accumulated drift doesn't skew the clock frequency over time.
    if (elapsed > m_interval) {
        m_startTime += m_interval;
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
    // Pixmap index 0 = clock-low SVG, index 1 = clock-high SVG (matches bool→int cast)
    setPixmap(static_cast<int>(m_isOn));
    outputPort()->setStatus(static_cast<Status>(m_isOn));
}

void Clock::save(QDataStream &stream) const
{
    GraphicElement::save(stream);

    QMap<QString, QVariant> map;
    map.insert("frequency", frequency());
    map.insert("delay", delay());
    map.insert("locked", m_locked);

    stream << map;
}

void Clock::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if (!VersionInfo::hasClock(context.version)) {
        // Clock serialization was introduced in v1.1; nothing to read in earlier files
        return;
    }

    if (!VersionInfo::hasQMapFormat(context.version)) {
        // v1.1–4.0 stored frequency as a bare float; locked state added in v3.1
        float freq; stream >> freq;
        setFrequency(freq);

        if (VersionInfo::hasLockState(context.version)) {
            stream >> m_locked;
        }
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map so new properties can be added without breaking old files
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("frequency")) {
            setFrequency(map.value("frequency").toFloat());
        }

        if (map.contains("delay")) {
            float delayValue = map.value("delay").toFloat();

            if (!VersionInfo::hasDelayFix(context.version)) {
                // Discard old delay data from versions < 4.3
                // The old implementation was incorrect and incompatible with the new period-fraction format
            } else {
                setDelay(delayValue);
            }
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

    // m_interval is the half-period: at frequency f, the full period is 1/f seconds,
    // so each HIGH/LOW phase lasts 1/(2f) seconds = half-period in microseconds.
    std::chrono::microseconds auxInterval = std::chrono::duration_cast<std::chrono::microseconds>(1s / (2 * freq));

    // Guard against frequencies so high that the half-period rounds to zero
    if (auxInterval.count() <= 0) {
        return;
    }

    m_interval = auxInterval;
    m_frequency = static_cast<double>(freq);
}

float Clock::delay() const
{
    return static_cast<float>(m_delay);
}

void Clock::setDelay(const float delay)
{
    m_delay = static_cast<double>(delay);
}

void Clock::resetClock(std::chrono::steady_clock::time_point globalTime)
{
    // Start clocks in the HIGH state; the first transition happens after one interval
    setOn();
    // m_delay is a fraction of the period (-1 to 1).
    // Negative delays advance the clock (trigger earlier), positive delays delay it.
    // Shifting m_startTime backward by the delay fraction effectively phase-shifts the waveform.
    // Full period is 2 * m_interval (since m_interval is the half-period).
    const auto fullPeriod = 2 * m_interval;
    const auto delayMicroseconds = static_cast<std::chrono::microseconds::rep>(-m_delay * static_cast<double>(fullPeriod.count()));
    m_startTime = globalTime;
    m_startTime -= std::chrono::microseconds(delayMicroseconds);
}

QString Clock::genericProperties()
{
    return QString::number(static_cast<double>(frequency())) + " Hz";
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

