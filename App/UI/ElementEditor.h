// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief ElementEditor widget for inspecting and modifying selected circuit element properties.
 */

#pragma once

#include <memory>

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "App/Element/PropertyDescriptor.h"
#include "App/Scene/Scene.h"
#include "App/UI/ElementEditorUI.h"
#include "App/UI/SelectionCapabilities.h"

class ElementTabNavigator;

/**
 * \class ElementEditor
 * \brief Widget for inspecting and editing the properties of selected circuit elements.
 *
 * \details The ElementEditor dynamically shows controls appropriate for the current
 * selection: label, color, frequency, input/output count, skin, trigger, audio, and
 * truth-table.  When multiple elements are selected it coalesces values, showing
 * placeholder text when values differ.  Property changes are pushed as QUndoCommands.
 */
class ElementEditor : public QWidget
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the element editor with optional \a parent.
    explicit ElementEditor(QWidget *parent = nullptr);
    ~ElementEditor() override;

    // --- Scene Binding ---

    /// Associates the editor with \a scene to read/write selected elements.
    void setScene(Scene *scene);

    // --- UI Refresh ---

    /// Refreshes the editor UI to reflect the current selection.
    void update();
    /// Re-applies translatable strings to all editor widgets.
    void retranslateUi();
    /// Applies the current theme colors to the editor widgets.
    void updateTheme();
    /// Repopulates the color combo box with available color options.
    void fillColorComboBox();

    // --- User Actions ---

    /// Opens an inline editor to rename the selected element(s).
    void renameAction();
    /// Opens a dialog to change the trigger key for the selected element(s).
    void changeTriggerAction();
    /// Opens the audio file selector for the selected element(s).
    void audioBox();
    /// Opens the truth table editor for the selected element(s).
    void truthTable();
    /// Applies the chosen skin to the selected element(s).
    void updateElementSkin();

    // --- Context Menu ---

    /// Shows the element context menu at \a screenPos for the item under \a itemAtMouse.
    void contextMenu(QPoint screenPos, QGraphicsItem *itemAtMouse);

signals:
    /**
     * \brief Emitted when a property change should be pushed onto the undo stack.
     * \param cmd The undo command encapsulating the change.
     */
    void sendCommand(QUndoCommand *cmd);

    /// Emitted when the user requests editing an embedded IC sub-circuit.
    void editSubcircuitRequested(const QString &blobName, int icElementId);

    /// Emitted when the user requests embedding a file-backed IC.
    void embedSubcircuitRequested();

    /// Emitted when the user requests extracting an embedded IC to file.
    void extractToFileRequested();

private:
    Q_DISABLE_COPY(ElementEditor)

    friend class ElementTabNavigator;

    // --- Helpers ---

    /// Pushes undo commands for all pending property changes to the selected elements.
    void apply();
    /// Shows or hides editor widgets based on the combined capabilities of the selection.
    void applyCapabilitiesToUi();
    /// Applies a single property \a type from the UI controls to \a elm.
    void applyProperty(GraphicElement *elm, PropertyDescriptor::Type type);
    /// Restores the default skin on the selected elements.
    void defaultSkin();
    /// Slot: the user changed the desired input count via the input spin box.
    void inputIndexChanged(const int index);
    /// Slot: the user toggled the input-count lock check box.
    void inputLocked(const bool value);
    /// Slot: the user changed the desired output count via the output spin box.
    void outputIndexChanged(const int index);
    /// Slot: the user changed the output value combo box (for truth-table outputs).
    void outputValueChanged(const QString &value);
    /// Slot: the scene selection changed; refreshes the editor state.
    void selectionChanged();
    /// Stores \a elements as the current editing target and refreshes the UI.
    void setCurrentElements(const QList<GraphicElement *> &elements);
    /// Slot: the user toggled a truth table cell at (\a row, \a column).
    void setTruthTableProposition(const int row, const int column);
    /// Slot: the user changed the keyboard trigger via the key-sequence editor.
    void triggerChanged(const QString &cmd);

    // --- Members ---

    std::unique_ptr<ElementEditorUi> m_ui;       ///< Auto-generated editor UI widgets.
    QDialog *m_tableBox             = nullptr;    ///< Dialog that hosts the truth table grid.
    QTableWidget *m_table           = nullptr;    ///< Truth table grid widget (inside m_tableBox).
    Scene *m_scene                  = nullptr;    ///< Scene whose selection this editor reflects.
    ElementTabNavigator *m_tabNavigator = nullptr; ///< Tab-order navigator for focused element cycling.
    QList<GraphicElement *> m_elements;           ///< Currently selected elements being edited.
    SelectionCapabilities m_caps;                 ///< Merged capability flags of the current selection.

    // Skin state
    QString m_skinName;              ///< File path of the active custom skin, if any.
    bool m_isDefaultSkin  = true;    ///< True when the built-in default skin is active.
    bool m_isUpdatingSkin = false;   ///< Guard flag to suppress recursive skin-change signals.

    // Placeholder strings for multi-selection display
    QString m_manyAudios   = tr("<Many sounds>");
    QString m_manyColors   = tr("<Many colors>");
    QString m_manyDelay    = tr("<Many values>");
    QString m_manyFreq     = tr("<Many values>");
    QString m_manyIS       = tr("<Many values>");
    QString m_manyLabels   = tr("<Many labels>");
    QString m_manyOS       = tr("<Many values>");
    QString m_manyOV       = tr("<Many values>");
    QString m_manyTriggers      = tr("<Many triggers>");
    QString m_manyWirelessModes = tr("<Many modes>");
};

