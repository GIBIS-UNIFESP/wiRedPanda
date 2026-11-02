// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ClockTiming: a clock's level in real time.
 */

#pragma once

#include <chrono>

class Element;

/// A clock toggles its level (ParamId::On) every half-period of its frequency (ParamId::Frequency), from a phase its
/// delay (ParamId::Delay) shifts. The phase is the element's simulation state (Element::clockPhase()); the simulation
/// resets, shifts and updates it. The clock is the only element driven by wall-clock time rather than the synchronous
/// simulation cycle.
namespace ClockTiming {

/// The default frequency in Hz, the frequency's spec's.
inline constexpr double kDefaultFrequency = 1.0;

/// The half-period (one HIGH or LOW phase) for \a frequency Hz: the full period is 1/frequency seconds, so each phase
/// lasts 1/(2·frequency). The frequency's spec guards the input; this assumes a positive, finite \a frequency.
constexpr std::chrono::microseconds halfPeriod(const double frequency)
{
    using namespace std::chrono_literals;
    return std::chrono::duration_cast<std::chrono::microseconds>(1s / (2 * frequency));
}

/// Starts the clock of \a element HIGH, its phase at \a globalTime shifted by its delay: the first toggle comes after
/// one half-period.
void reset(Element &element, std::chrono::steady_clock::time_point globalTime);

/// Returns \c true once reset() has given the clock of \a element a phase, which a rebuild of the simulation keeps.
bool hasPhase(const Element &element);

/// Shifts the phase of the clock of \a element forward by \a pause, keeping its level and its place within the period:
/// a resume after a pause, unlike reset(), which starts it HIGH again, keeps the waveform where it stopped and fires no
/// burst of missed toggles.
void shift(Element &element, std::chrono::steady_clock::duration pause);

/// Toggles the clock of \a element once the half-period since its phase has passed at \a globalTime; a locked clock
/// holds its level.
void update(Element &element, std::chrono::steady_clock::time_point globalTime);

} // namespace ClockTiming
