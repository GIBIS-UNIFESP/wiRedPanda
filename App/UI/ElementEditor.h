// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "App/Scene/Scene.h"
#include "App/UI/ElementEditorUI.h"

/*!
 * @class ElementEditor
 * @brief Widget for editing the element properties of a circuit
 *
 * The ElementEditor class provides a user interface for editing properties
 * of circuit elements. It dynamically adapts to show appropriate controls
 * based on the selected element types, handling both single and multiple
 * element selections.
 */
class ElementEditor : public QWidget
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit ElementEditor(QWidget *parent = nullptr);
    ~ElementEditor() override;

    // --- Scene Binding ---

    void setScene(Scene *scene);

    // --- UI Refresh ---

    void update();
    void retranslateUi();
    void updateTheme();
    void fillColorComboBox();

    // --- User Actions ---

    void renameAction();
    void changeTriggerAction();
    void audioBox();
    void truthTable();
    void updateElementSkin();
    void updatePriorityAction();

    // --- Context Menu ---

    void contextMenu(QPoint screenPos, QGraphicsItem *itemAtMouse);

    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    // --- Signals ---

    void sendCommand(QUndoCommand *cmd);

private:
    Q_DISABLE_COPY(ElementEditor)

    // --- Helpers ---

    void apply();
    void applyCapabilitiesToUi();
    void defaultSkin();
    void selectionChanged();
    void setCurrentElements(const QList<GraphicElement *> &elements);

    // Input/output slot handlers
    void inputIndexChanged(const int index);
    void inputLocked(const bool value);
    void outputIndexChanged(const int index);
    void outputValueChanged(const QString &value);

    // Truth table helpers
    void setTruthTableProposition(const int row, const int column);

    // Trigger handler
    void triggerChanged(const QString &cmd);

    // Skin helpers
    void updateSkins();
    void priorityChanged(const int value);

    // --- Members ---

    // UI and sub-widgets
    std::unique_ptr<ElementEditorUi> m_ui;
    QDialog *m_tableBox = nullptr;
    QTableWidget *m_table = nullptr;

    // Scene and selection state
    Scene *m_scene = nullptr;
    QList<GraphicElement *> m_elements;

    // Skin state
    QString m_skinName;
    bool m_isDefaultSkin = true;
    bool m_isUpdatingSkin = false;

    // Placeholder strings for multi-selection display
    QString m_manyAudios = tr("<Many sounds>");
    QString m_manyColors = tr("<Many colors>");
    QString m_manyDelay = tr("<Many values>");
    QString m_manyFreq = tr("<Many values>");
    QString m_manyIS = tr("<Many values>");
    QString m_manyLabels = tr("<Many labels>");
    QString m_manyOS = tr("<Many values>");
    QString m_manyOV = tr("<Many values>");
    QString m_manyPriorities = tr("<Many priorities>");
    QString m_manyTriggers = tr("<Many triggers>");

    // Capability flags
    bool m_canChangeInputSize = false;
    bool m_canChangeOutputSize = false;
    bool m_canChangeSkin = false;
    bool m_canMorph = false;

    // Property detection flags
    bool m_hasAnyProperty = false;
    bool m_hasAudio = false;
    bool m_hasAudioBox = false;
    bool m_hasColors = false;
    bool m_hasDelay = false;
    bool m_hasElements = false;
    bool m_hasFrequency = false;
    bool m_hasLabel = false;
    bool m_hasOnlyInputs = false;
    bool m_hasRotation = false;
    bool m_hasTrigger = false;
    bool m_hasTruthTable = false;

    // Property matching flags (set when all selected elements have same value)
    bool m_hasSameAudio = false;
    bool m_hasSameColors = false;
    bool m_hasSameDelay = false;
    bool m_hasSameFrequency = false;
    bool m_hasSameInputSize = false;
    bool m_hasSameLabel = false;
    bool m_hasSameOutputSize = false;
    bool m_hasSameOutputValue = false;
    bool m_hasSamePriority = false;
    bool m_hasSameTrigger = false;
    bool m_hasSameType = false;
};

