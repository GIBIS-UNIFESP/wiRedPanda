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

private:
    Q_DISABLE_COPY(ElementEditor)

    friend class ElementTabNavigator;

    // --- Helpers ---

    void apply();
    void applyCapabilitiesToUi();
    void applyProperty(GraphicElement *elm, PropertyDescriptor::Type type);
    void defaultSkin();
    void inputIndexChanged(const int index);
    void inputLocked(const bool value);
    void outputIndexChanged(const int index);
    void outputValueChanged(const QString &value);
    void selectionChanged();
    void setCurrentElements(const QList<GraphicElement *> &elements);
    void setTruthTableProposition(const int row, const int column);
    void triggerChanged(const QString &cmd);

    // --- Members ---

    std::unique_ptr<ElementEditor_Ui> m_ui;
    QDialog *m_tableBox             = nullptr;
    QTableWidget *m_table           = nullptr;
    Scene *m_scene                  = nullptr;
    ElementTabNavigator *m_tabNavigator = nullptr;
    QList<GraphicElement *> m_elements;
    SelectionCapabilities m_caps;

    // Skin state
    QString m_skinName;
    bool m_isDefaultSkin  = true;
    bool m_isUpdatingSkin = false;

    // Placeholder strings for multi-selection display
    QString m_manyAudios        = tr("<Many sounds>");
    QString m_manyColors        = tr("<Many colors>");
    QString m_manyDelay         = tr("<Many values>");
    QString m_manyFreq          = tr("<Many values>");
    QString m_manyIS            = tr("<Many values>");
    QString m_manyLabels        = tr("<Many labels>");
    QString m_manyOS            = tr("<Many values>");
    QString m_manyOV            = tr("<Many values>");
    QString m_manyTriggers      = tr("<Many triggers>");
    QString m_manyWirelessModes = tr("<Many modes>");
};
