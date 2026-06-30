// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/SimulationRecordingSuspender.h"

#include "App/Simulation/Simulation.h"

SimulationRecordingSuspender::SimulationRecordingSuspender(Simulation *simulation)
    : m_simulation(simulation)
{
    m_wasRecording = m_simulation->waveformRecorder().isRecording();
    m_simulation->waveformRecorder().setRecording(false);
}

SimulationRecordingSuspender::~SimulationRecordingSuspender()
{
    m_simulation->waveformRecorder().setRecording(m_wasRecording);
}
