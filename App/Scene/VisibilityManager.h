// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief VisibilityManager: controls gate and wire visibility in the scene.
 */

#pragma once

class Scene;

/**
 * \class VisibilityManager
 * \brief Controls the visibility of gates, wires, and port handles in the circuit scene.
 *
 * Extracted from Scene to decouple visibility toggle logic from the main
 * circuit scene.  Called by Scene::setCircuitUpdateRequired() and by
 * MainWindow UI toggles.
 */
class VisibilityManager
{
public:
    explicit VisibilityManager(Scene *scene);

    /// Shows or hides gate elements (Input/Output/Other groups are always visible).
    void showGates(bool visible);

    /// Shows or hides connection wires, node elements, and port handles.
    void showWires(bool visible);

    /// Reapplies current visibility state (used after structural changes).
    void reapply();

    [[nodiscard]] bool gatesVisible() const { return m_showGates; }
    [[nodiscard]] bool wiresVisible() const { return m_showWires; }

private:
    Scene *m_scene = nullptr;
    bool m_showGates = true;
    bool m_showWires = true;
};

