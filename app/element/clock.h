
// Copyright 2015 - 2024, GIBIS-UNIFESP and the wiRedPanda contributors



// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "graphicelementinput.h"

class Clock : public GraphicElementInput
{
    Q_OBJECT

public:
    explicit Clock(QGraphicsItem *parent = nullptr);

    QString genericProperties() override;
    bool isOn(const int port = 0) const override;
    float frequency() const override;
    void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version) override;
    void resetClock();
    void save(QDataStream &stream) const override;
    void setFrequency(const float freq) override;
    void setOff() override;
    void setOn() override;
    void setOn(const bool value, const int port = 0) override;
    void setSkin(const bool defaultSkin, const QString &fileName) override;
    void updateClock();

private:
    bool m_isOn = false;
    bool m_reset = true;
    double m_frequency = 0;
    std::chrono::duration<float, std::milli> m_interval{};
    std::chrono::steady_clock::time_point m_timePoint;
};

Q_DECLARE_METATYPE(Clock)
