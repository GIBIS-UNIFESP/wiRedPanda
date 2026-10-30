// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Clock.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"

using namespace std::chrono_literals;

template<>
struct ElementInfo<Clock> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Clock,
        .group = ElementGroup::Input,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeAppearance = true,
        .hasFrequency = true,
        .hasDelay = true,
        .hasLabel = true,
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Input/clock1.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("Clock", "CLOCK SIGNAL");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Clock", "Clock");
        descriptor.trContext = "Clock";
        descriptor.defaultAppearances = QStringList({
            ":/Components/Input/clock0.svg",
            ":/Components/Input/clock1.svg",
        });
        // A clock's level follows the simulation's time: no state of the circuit
        descriptor.parameters = {
            ElementInput::levelSpec(nullptr),
            ElementInput::lockSpec(),
            {
                // A frequency so high that the half-period rounds to zero is refused too
                .id = ParamId::Frequency,
                .defaultValue = Clock::kDefaultFrequency,
                .key = "frequency",
                .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                    const double frequency = std::get<double>(value);
                    const bool takes = std::isfinite(frequency) && !qFuzzyIsNull(frequency)
                                       && (Clock::halfPeriod(frequency).count() > 0);
                    return takes ? std::optional<ParamValue>(value) : std::nullopt;
                },
            },
            {
                // A fraction of the period in [-1, 1]: a finite value beyond it would multiply with the period in
                // resetClock() to a magnitude beyond microseconds::rep, which is undefined on the cast
                .id = ParamId::Delay,
                .defaultValue = 0.0,
                .key = "delay",
                .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                    const double delay = std::get<double>(value);
                    if (!std::isfinite(delay)) {
                        return std::nullopt;
                    }
                    return ParamValue(std::clamp(delay, -1.0, 1.0));
                },
            },
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Clock(); });
        return true;
    }();
};

Clock::Clock(QGraphicsItem *parent)
    : ElementInput(ElementType::Clock, parent)
{
    // Frequency, interval, delay, and locked state are all covered by member
    // initializers; only the output state needs behavior, not initialization:
    Clock::setOff(); // start LOW; resetClock() will start HIGH when simulation begins
}

void Clock::updateClock(std::chrono::steady_clock::time_point globalTime)
{
    if (isLocked()) {
        return;
    }

    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(globalTime - m_startTime);

    // A gap this large means the wall clock jumped far ahead without an intervening
    // Simulation::stop()/start() cycle (which already resyncs via resetClock() — see its call
    // site) — e.g. the OS suspended the process while the simulation's QTimer stayed "active".
    // Left to the single-step path below, replaying the backlog would toggle once per
    // subsequent 1 ms tick until caught up: for a long gap, a burst of thousands of rapid,
    // unrealistic toggles feeding into any connected sequential logic. Resync immediately
    // instead, on this very first tick after the gap, so the burst never starts. The threshold
    // is a fixed wall-clock duration, not a multiple of m_interval — scaling it to the
    // half-period would fire this path on entirely ordinary jitter for high-frequency clocks
    // (e.g. a 100 Hz clock's 5 ms interval means even a benign 20+ ms hiccup would look like a
    // "large" multiple), which is not the gap this guards against.
    constexpr auto kMaxCatchUpGap = std::chrono::seconds(5);
    if (elapsed > kMaxCatchUpGap) {
        resetClock(globalTime);
        return;
    }

    // m_interval is the half-period (time per HIGH or LOW phase).
    // Rather than resetting to globalTime we advance by exactly one interval so
    // that accumulated drift doesn't skew the clock frequency over time.
    if (elapsed > m_interval) {
        m_startTime += m_interval;
        setOn(!isOn());
    }
}

bool Clock::isOn(const int port) const
{
    Q_UNUSED(port)
    return param<bool>(ParamId::On);
}

void Clock::setOff()
{
    Clock::setOn(false);
}

void Clock::setOn()
{
    Clock::setOn(true);
}

void Clock::save(QDataStream &stream, SerializationOptions options) const
{
    Element::save(stream, options);

    const bool slim = (options.purpose == SerializationPurpose::PortableFile);

    QMap<QString, QVariant> map;
    // PortableFile streams omit defaults; fresh-loaded elements start there
    // anyway. Snapshots write unconditionally (reloaded into live elements).
    if (!slim || frequency() != kDefaultFrequency) {
        map.insert("frequency", frequency());
    }
    if (!slim || delay() != 0.0) {
        map.insert("delay", delay());
    }
    if (!slim || isLocked()) {
        map.insert("locked", isLocked());
    }

    stream << map;
}

void Clock::load(QDataStream &stream, SerializationContext &context)
{
    Element::load(stream, context);

    if (!VersionInfo::hasClock(context.version)) {
        // Clock serialization was introduced in v1.1; nothing to read in earlier files
        return;
    }

    if (!VersionInfo::hasQMapFormat(context.version)) {
        // v1.1–4.0 stored frequency as a bare float; locked state added in v3.1
        float freq; stream >> freq;
        setFrequency(static_cast<double>(freq));

        if (VersionInfo::hasLockState(context.version)) {
            bool locked = false;
            stream >> locked;
            setLocked(locked);
        }
    }

    if (VersionInfo::hasQMapFormat(context.version)) {
        // v4.1+ uses a key-value map so new properties can be added without breaking old files
        QMap<QString, QVariant> map = Serialization::readBoundedMetadata(stream);

        if (map.contains("frequency")) {
            setFrequency(map.value("frequency").toDouble());
        }

        if (map.contains("delay")) {
            bool ok = false;
            double delayValue = map.value("delay").toDouble(&ok);
            if (!ok) {
                delayValue = 0.0;
            }

            if (!VersionInfo::hasDelayFix(context.version)) {
                // Discard old delay data from versions < 4.3
                // The old implementation was incorrect and incompatible with the new period-fraction format
            } else {
                setDelay(delayValue);
            }
        }

        if (map.contains("locked")) {
            setLocked(map.value("locked").toBool());
        }
    }
}

void Clock::parameterChanged(const ParamId id, const ParamValue &previous)
{
    ElementInput::parameterChanged(id, previous);
    if (id == ParamId::Frequency) {
        m_interval = halfPeriod(frequency());
    }
}

void Clock::resetClock(std::chrono::steady_clock::time_point globalTime)
{
    // Start clocks in the HIGH state; the first transition happens after one interval
    setOn();
    // The delay is a fraction of the period (-1 to 1).
    // Negative delays advance the clock (trigger earlier), positive delays delay it.
    // Shifting m_startTime backward by the delay fraction effectively phase-shifts the waveform.
    // Full period is 2 * m_interval (since m_interval is the half-period).
    const auto fullPeriod = 2 * m_interval;
    const auto delayMicroseconds = static_cast<std::chrono::microseconds::rep>(-delay() * static_cast<double>(fullPeriod.count()));
    m_startTime = globalTime;
    m_startTime -= std::chrono::microseconds(delayMicroseconds);
    m_hasPhase = true;
}

void Clock::shiftClock(std::chrono::steady_clock::duration pause)
{
    // Resume-after-pause: the wall clock advanced by `pause` while the simulation was
    // stopped, so move the phase reference forward by the same amount. updateClock() then
    // sees the same elapsed-within-interval it saw at the moment of the pause — the output
    // level is untouched (no forced HIGH, no injected edge) and no missed toggles fire in
    // a burst.
    m_startTime += pause;
}

QString Clock::genericProperties()
{
    return QString::number(frequency()) + " Hz";
}

QList<std::pair<int, QString>> Clock::appearanceStates() const
{
    return {{0, tr("Low")}, {1, tr("High")}};
}
