// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief Graphic element for the buzzer tone output.

#pragma once

#include "App/Element/GraphicElements/AudioOutputElement.h"

/// Audio output element that plays a continuous tone, at its frequency, while its input is logic-1 (AudioService).
class Buzzer : public AudioOutputElement
{
    Q_OBJECT

public:
    /// Default tone frequency in Hz (C6). Single source of truth: used by the
    /// frequency's spec and serialization's default-elision check.
    static constexpr double kDefaultFrequency = 1047.0;

    /// Default volume; deliberately lower than AudioOutputElement's 0.5 base
    /// default to avoid harsh tones. Single source of truth: used by the
    /// volume's spec and serialization's default-elision check.
    static constexpr double kDefaultVolume = 0.35;

    /// Constructs a Buzzer element (default: 1047 Hz / C6).
    explicit Buzzer(QGraphicsItem *parent = nullptr);

    /// Copy-constructs by delegating to the parent item constructor.
    Buzzer(const Buzzer &other) : Buzzer(other.parentItem()) {}

    // --- Playback Control ---

    /// Sets the tone by note name (e.g. "C6"), as older files hold it; an empty name changes nothing.
    void setNote(const QString &note);

    static int noteToFrequency(const QString &note);
};
