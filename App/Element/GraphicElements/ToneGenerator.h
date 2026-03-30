// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/// \file
/// \brief QIODevice that generates a continuous sine-wave tone for audio playback.

#pragma once

#include <cmath>

#include <QAudioFormat>
#include <QIODevice>

/// Generates a continuous sine wave at a configurable frequency.
///
/// Intended for use with QAudioSink. The device produces 16-bit mono PCM
/// samples at 44100 Hz with no loop boundary, so playback is seamless.
class ToneGenerator : public QIODevice
{
    Q_OBJECT

public:
    explicit ToneGenerator(QObject *parent = nullptr)
        : QIODevice(parent)
    {
    }

    /// Returns the QAudioFormat matching this generator's output.
    static QAudioFormat format()
    {
        QAudioFormat fmt;
        fmt.setSampleRate(SampleRate);
        fmt.setChannelCount(1);
        fmt.setSampleFormat(QAudioFormat::Int16);
        return fmt;
    }

    /// Sets the tone frequency in Hz.
    void setFrequency(double hz) { m_frequency = hz; }

    /// Returns the current frequency in Hz.
    double frequency() const { return m_frequency; }

    /// This is a sequential (non-seekable) device.
    bool isSequential() const override { return true; }

    /// Reports available data so QAudioSink knows to pull from this device.
    qint64 bytesAvailable() const override { return SampleRate * BytesPerSample; }

    /// Starts the device for reading.
    bool start()
    {
        m_phase = 0.0;
        return open(QIODevice::ReadOnly);
    }

protected:
    /// Fills \a data with \a maxSize bytes of PCM sine-wave samples.
    qint64 readData(char *data, qint64 maxSize) override
    {
        constexpr double twoPi = 2.0 * M_PI;
        const double increment = m_frequency / SampleRate;

        const qint64 samples = maxSize / BytesPerSample;
        auto *ptr = reinterpret_cast<qint16 *>(data);

        for (qint64 i = 0; i < samples; ++i) {
            ptr[i] = static_cast<qint16>(Amplitude * std::sin(twoPi * m_phase));
            m_phase += increment;
            if (m_phase >= 1.0) {
                m_phase -= 1.0;
            }
        }

        return samples * BytesPerSample;
    }

    /// Writing is not supported.
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    static constexpr int SampleRate = 44100;
    static constexpr int BytesPerSample = sizeof(qint16);
    static constexpr double Amplitude = 28000.0;

    double m_frequency = 1047.0; // C6 default
    double m_phase = 0.0;
};

