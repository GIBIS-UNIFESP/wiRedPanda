// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ElementContextMenu free function for building the element right-click menu.
 */

#pragma once

#include <functional>

#include <QList>
#include <QPoint>

class GraphicElement;
class QComboBox;
class QGraphicsItem;
class QUndoCommand;
class Scene;
struct SelectionCapabilities;

/**
 * \namespace ElementContextMenu
 * \brief Builds and executes the right-click context menu for circuit element selections.
 *
 * \details Separated from ElementEditor so the menu-building logic can be understood
 * and tested independently of the property-editing panel.
 */
namespace ElementContextMenu {

/**
 * \brief Builds and runs the right-click context menu for a set of selected elements.
 *
 * \param screenPos        Screen position where the menu should appear.
 * \param itemAtMouse      Item under the mouse cursor (may be nullptr).
 * \param caps             Pre-computed selection capabilities.
 * \param elements         Currently selected elements.
 * \param colorCombo       The ElementEditor's color combo box (used to populate the Colors submenu).
 * \param scene            The circuit scene (for Copy / Cut / Delete actions).
 * \param sendCommand      Callback that pushes a command onto the undo stack.
 * \param onRename         Callback for the Rename action (focuses the label field).
 * \param onTriggerChange  Callback for the Change Trigger action (focuses the trigger field).
 * \param onAppearanceChange     Callback for the Change Appearance action (opens a file dialog).
 * \param onAppearanceRevert     Callback for the Set Appearance to Default action.
 * \param onFrequencyFocus Callback for the Change Frequency action (focuses the spinbox).
 * \param onEditSubcircuit Callback for the Edit Subcircuit action (opens the IC editor).
 * \param onEmbedSubcircuit Callback for the Embed Subcircuit action (inlines the IC into the parent circuit).
 * \param onExtractToFile Callback for the Extract to File action (saves the IC definition to disk).
 */
void exec(QPoint screenPos,
          QGraphicsItem *itemAtMouse,
          const SelectionCapabilities &caps,
          const QList<GraphicElement *> &elements,
          QComboBox *colorCombo,
          Scene *scene,
          const std::function<void(QUndoCommand *)> &sendCommand,
          const std::function<void()> &onRename,
          const std::function<void()> &onTriggerChange,
          const std::function<void()> &onAppearanceChange,
          const std::function<void()> &onAppearanceRevert,
          const std::function<void()> &onFrequencyFocus,
          const std::function<void()> &onEditSubcircuit = {},
          const std::function<void()> &onEmbedSubcircuit = {},
          const std::function<void()> &onExtractToFile = {});

} // namespace ElementContextMenu

