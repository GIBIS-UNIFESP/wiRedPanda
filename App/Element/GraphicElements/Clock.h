// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "App/Element/GraphicElementInput.h"

class Clock : public GraphicElementInput
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit Clock(QGraphicsItem *parent = nullptr);

    // --- State Queries ---

    bool isOn(const int port = 0) const override;
    float frequency() const override;
    float delay() const override;
    QString genericProperties() override;

    // --- State Setters ---

    void setOn() override;
    void setOff() override;
    void setOn(const bool value, const int port = 0) override;
    void setFrequency(const float freq) override;
    void setDelay(const float delay) override;

    // --- Simulation ---

    void resetClock(const std::chrono::steady_clock::time_point &globalTime);
    void updateClock(const std::chrono::steady_clock::time_point &globalTime);

    // --- Visual ---

    void setSkin(const bool defaultSkin, const QString &fileName) override;

    // --- Serialization ---

    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void save(QDataStream &stream) const override;

private:
    // --- Members ---

    bool m_isOn = false;
    double m_delay = 0;
    double m_frequency = 0;
    std::chrono::microseconds m_interval;
    std::chrono::steady_clock::time_point m_startTime;
};

Q_DECLARE_METATYPE(Clock)
