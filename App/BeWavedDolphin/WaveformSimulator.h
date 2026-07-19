// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Drives the wiRedPanda circuit simulation column-by-column for the waveform table.
 */

#pragma once

#include <functional>

#include <QVector>

#include "App/Core/Enums.h"

class GraphicElement;
class GraphicElementInput;
class Simulation;

/**
 * \class WaveformSimulator
 * \brief Drives the circuit simulation across the waveform's time columns.
 *
 * \details Extracted from BewavedDolphin: this owns the circuit-coupled sweep logic —
 * resetting sequential state, applying input values per time step, advancing the
 * simulation, and reading back output statuses — while leaving the waveform model and
 * its presentation to the caller via read/write callbacks.
 *
 * \details No longer stores a Scene* (see sweep()'s \a allElements parameter): the only thing
 * this class ever needed from a Scene was elements(), fetched fresh on every sweep() call
 * (never cached — the live element set can change between two sweeps, e.g. the user editing the
 * main canvas while this window is open, and a stale cache would silently skip resetting newly
 * added elements' sequential state). Making the caller pass that same fresh list explicitly
 * removes the Scene coupling entirely rather than trading it for a CanvasItem one -- both
 * Scene::elements() and CanvasItem::elements() already produce the exact QVector<GraphicElement*>
 * this takes, so this class now works unmodified for both apps.
 */
class WaveformSimulator
{
public:
    /// Constructs the driver for \a simulation.
    explicit WaveformSimulator(Simulation *simulation);

    /**
     * \brief Snapshots the live output-port state of every input element.
     * \param inputs     Input elements to snapshot.
     * \param inputPorts Total number of output ports across \a inputs (snapshot size).
     * \return Port-indexed states, to be restored after a sweep via restoreInputs().
     */
    static QVector<Status> captureInputs(const QVector<GraphicElementInput *> &inputs, int inputPorts);

    /// Restores input-port states previously captured by captureInputs().
    static void restoreInputs(const QVector<GraphicElementInput *> &inputs, const QVector<Status> &saved);

    /**
     * \brief Runs the simulation across \a columns time steps.
     * \param inputs      Input elements, in row order.
     * \param outputs     Output elements, in row order.
     * \param allElements Every element in the circuit (Scene::elements()/CanvasItem::elements()),
     *                    fetched fresh by the caller immediately before this call -- reset to
     *                    power-on sequential state before the sweep.
     * \param inputPorts  Number of input rows (offset of the first output row).
     * \param columns     Number of time-step columns to sweep.
     * \param readInput   Returns the input bit at (row, col) from the waveform model.
     * \param writeOutput Stores the computed output bit at (row, col) into the model.
     *
     * \details Resets every element's sequential state before sweeping. Does not restore
     * inputs — the caller pairs this with restoreInputs().
     */
    void sweep(const QVector<GraphicElementInput *> &inputs,
               const QVector<GraphicElement *> &outputs,
               const QVector<GraphicElement *> &allElements,
               int inputPorts, int columns,
               const std::function<bool(int row, int col)> &readInput,
               const std::function<void(int row, int col, int value)> &writeOutput) const;

private:
    Simulation *m_simulation = nullptr;  ///< The simulation engine used for the sweep.
};
