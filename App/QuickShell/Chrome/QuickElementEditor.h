// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickElementEditor: QML-facing presenter for the element property panel.
 */

#pragma once

#include <QList>
#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QVariantList>

#include "App/UI/SelectionCapabilities.h"

class CanvasItem;
class GraphicElement;

/// One entry in colorOptions(). A real QML value type -- not QVariantMap -- so a
/// delegate's `required property colorOption modelData` gives qmlcachegen a concrete
/// field layout to compile against. See project_qml_aot_compilation_fusion_style_pin
/// memory.
class ColorOption
{
    Q_GADGET
    QML_VALUE_TYPE(colorOption)

    Q_PROPERTY(QString name READ name FINAL)
    Q_PROPERTY(QString translatedName READ translatedName FINAL)
    Q_PROPERTY(QString iconSource READ iconSource FINAL)

public:
    ColorOption() = default;
    ColorOption(QString name, QString translatedName, QString iconSource)
        : m_name(std::move(name))
        , m_translatedName(std::move(translatedName))
        , m_iconSource(std::move(iconSource))
    {
    }

    [[nodiscard]] QString name() const { return m_name; }
    [[nodiscard]] QString translatedName() const { return m_translatedName; }
    [[nodiscard]] QString iconSource() const { return m_iconSource; }

private:
    QString m_name;
    QString m_translatedName;
    QString m_iconSource;
};

/// One entry in prepareContextMenu()'s morphCandidates(). Real QML value type, same
/// reasoning as ColorOption above.
class MorphCandidate
{
    Q_GADGET
    QML_VALUE_TYPE(morphCandidate)

    Q_PROPERTY(int type READ type FINAL)
    Q_PROPERTY(QString name READ name FINAL)
    Q_PROPERTY(QString iconSource READ iconSource FINAL)

public:
    MorphCandidate() = default;
    MorphCandidate(int type, QString name, QString iconSource)
        : m_type(type)
        , m_name(std::move(name))
        , m_iconSource(std::move(iconSource))
    {
    }

    [[nodiscard]] int type() const { return m_type; }
    [[nodiscard]] QString name() const { return m_name; }
    [[nodiscard]] QString iconSource() const { return m_iconSource; }

private:
    int m_type = 0;
    QString m_name;
    QString m_iconSource;
};

/// One entry in inputSizeOptions()/outputSizeOptions()/outputValueOptions() -- all
/// three share this shape. Real QML value type, same reasoning as ColorOption above
/// (these three lists aren't currently read via a user-written delegate/modelData, so
/// this doesn't fix any AOT failure today, but keeps the whole class consistently
/// typed instead of leaving a lone QVariantList island once the others convert).
class SizeOption
{
    Q_GADGET
    QML_VALUE_TYPE(sizeOption)

    Q_PROPERTY(QString label READ label FINAL)
    Q_PROPERTY(int value READ value FINAL)

public:
    SizeOption() = default;
    SizeOption(QString label, int value)
        : m_label(std::move(label))
        , m_value(value)
    {
    }

    [[nodiscard]] QString label() const { return m_label; }
    [[nodiscard]] int value() const { return m_value; }

private:
    QString m_label;
    int m_value = 0;
};

/**
 * \class QuickElementEditor
 * \brief Copy-and-adapted, Widgets-free port of App/UI/ElementEditor.h's property-read/apply
 * logic for the Quick chrome.
 *
 * \details Unlike ElementEditor (a QWidget reading/writing ~20 child widgets directly), this
 * class holds each editable value as plain state exposed through Q_PROPERTYs, so ElementEditor.qml
 * can bind to it declaratively -- one QML section per PropertyDescriptor::Type, shown/hidden by
 * this class's own *Visible properties (computed from SelectionCapabilities exactly as
 * ElementEditor::applyCapabilitiesToUi() does; QML's declarative visibility bindings replace
 * that method's imperative setSection() calls).
 *
 * \details Every setter applies immediately (mirroring each Widgets control's own signal being
 * wired straight to ElementEditor::apply()) and marks that field "dirty" until the next
 * refresh() -- this replaces ElementEditor's fragile "does the widget's current text still
 * equal the magic many-placeholder-string" check with an explicit per-field bool, needed
 * because a QML TextField/Slider has no equivalent placeholder-text state to compare against.
 * A field stays untouched (dirty == false) with hasSame*==false (a real divergent
 * multi-selection) is skipped by apply() exactly like ElementEditor::applyProperty() skips it;
 * touching it applies the new value to every selected element, also matching production.
 *
 * \details Explicitly NOT ported in this pass (real, separately-scoped follow-ups, not
 * oversights): Appearance (custom pixmap file dialog + per-state tile grid), AudioBox (file
 * dialog), TruthTable (the editor grid dialog -- CanvasToggleTruthTableOutputCommand itself is
 * already ported and usable once a QML dialog exists), and embedded-IC blob rename. Each needs
 * its own QML dialog surface this pass doesn't build yet.
 *
 * \details Also backs the right-click context menu (Phase 4 sub-step 6): shares the exact same
 * m_elements/m_caps this class already tracks reactively (canMorph/hasSelection double as the
 * menu's Morph-submenu/Copy-Cut-Delete visibility, colorOptions doubles as the Colors submenu),
 * plus morphCandidates -- computed from whichever element was right-clicked (prepareContextMenu()),
 * not the whole selection, mirroring ElementContextMenu::exec()'s addElementAction() switch
 * exactly. Change-Appearance/Revert-Appearance menu items are not built, matching Appearance's
 * deferral above.
 */
class QuickElementEditor : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("QuickElementEditor is only ever exposed via AppController.elementEditor")

    // FINAL throughout: QuickElementEditor is never subclassed, so QML's shadow-check
    // can trust these types for chained lookups (e.g. AppController.elementEditor.
    // labelVisible) instead of falling back to untyped "var" -- see
    // project_qml_aot_compilation_fusion_style_pin memory.
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY refreshed FINAL)
    Q_PROPERTY(QString elementTypeTitle READ elementTypeTitle NOTIFY refreshed FINAL)

    Q_PROPERTY(bool labelVisible READ isLabelVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY refreshed FINAL)

    Q_PROPERTY(bool colorVisible READ isColorVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY refreshed FINAL)
    Q_PROPERTY(QList<ColorOption> colorOptions READ colorOptions CONSTANT FINAL)

    Q_PROPERTY(bool audioVisible READ isAudioVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString audio READ audio WRITE setAudio NOTIFY refreshed FINAL)
    Q_PROPERTY(QVariantList audioOptions READ audioOptions CONSTANT FINAL)

    Q_PROPERTY(bool volumeVisible READ isVolumeVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY refreshed FINAL)

    Q_PROPERTY(bool frequencyVisible READ isFrequencyVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(double frequency READ frequency WRITE setFrequency NOTIFY refreshed FINAL)
    Q_PROPERTY(double frequencyMin READ frequencyMin NOTIFY refreshed FINAL)
    Q_PROPERTY(double frequencyMax READ frequencyMax NOTIFY refreshed FINAL)
    Q_PROPERTY(double frequencyStep READ frequencyStep NOTIFY refreshed FINAL)
    Q_PROPERTY(int frequencyDecimals READ frequencyDecimals NOTIFY refreshed FINAL)

    Q_PROPERTY(bool delayVisible READ isDelayVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(int delaySteps READ delaySteps WRITE setDelaySteps NOTIFY refreshed FINAL)

    Q_PROPERTY(bool inputSizeVisible READ isInputSizeVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QList<SizeOption> inputSizeOptions READ inputSizeOptions NOTIFY refreshed FINAL)
    Q_PROPERTY(int inputSize READ inputSize WRITE setInputSize NOTIFY refreshed FINAL)

    Q_PROPERTY(bool outputSizeVisible READ isOutputSizeVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QList<SizeOption> outputSizeOptions READ outputSizeOptions NOTIFY refreshed FINAL)
    Q_PROPERTY(int outputSize READ outputSize WRITE setOutputSize NOTIFY refreshed FINAL)

    Q_PROPERTY(bool outputValueVisible READ isOutputValueVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QList<SizeOption> outputValueOptions READ outputValueOptions NOTIFY refreshed FINAL)
    Q_PROPERTY(int outputValue READ outputValue WRITE setOutputValue NOTIFY refreshed FINAL)

    Q_PROPERTY(bool lockedVisible READ isLockedVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(int lockedState READ lockedState WRITE setLockedState NOTIFY refreshed FINAL)

    Q_PROPERTY(bool triggerVisible READ isTriggerVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString trigger READ trigger WRITE setTrigger NOTIFY refreshed FINAL)

    Q_PROPERTY(bool wirelessModeVisible READ isWirelessModeVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(int wirelessMode READ wirelessMode WRITE setWirelessMode NOTIFY refreshed FINAL)
    Q_PROPERTY(QVariantList wirelessModeOptions READ wirelessModeOptions CONSTANT FINAL)

    Q_PROPERTY(bool canMorph READ canMorph NOTIFY refreshed FINAL)
    Q_PROPERTY(QList<MorphCandidate> morphCandidates READ morphCandidates NOTIFY refreshed FINAL)

public:
    explicit QuickElementEditor(QObject *parent = nullptr);

    /// Binds this editor to \a canvas's selectionChanged(); passing nullptr unbinds (mirrors
    /// ElementEditor::setScene(nullptr) on tab change). Immediately refreshes from the new
    /// canvas's current selection.
    void setCanvas(CanvasItem *canvas);

    [[nodiscard]] bool hasSelection() const { return !m_elements.isEmpty(); }
    [[nodiscard]] QString elementTypeTitle() const;

    [[nodiscard]] bool isLabelVisible() const { return m_caps.hasLabel; }
    [[nodiscard]] QString label() const { return m_label; }
    void setLabel(const QString &value);

    [[nodiscard]] bool isColorVisible() const { return m_caps.hasColors; }
    [[nodiscard]] QString color() const { return m_color; }
    void setColor(const QString &value);
    [[nodiscard]] static QList<ColorOption> colorOptions();

    [[nodiscard]] bool isAudioVisible() const { return m_caps.hasAudio; }
    [[nodiscard]] QString audio() const { return m_audio; }
    void setAudio(const QString &value);
    [[nodiscard]] static QVariantList audioOptions();

    [[nodiscard]] bool isVolumeVisible() const { return m_caps.hasVolume; }
    [[nodiscard]] int volume() const { return m_volume; }
    void setVolume(int value);

    [[nodiscard]] bool isFrequencyVisible() const { return m_caps.hasFrequency; }
    [[nodiscard]] double frequency() const { return m_frequency; }
    void setFrequency(double value);
    [[nodiscard]] double frequencyMin() const { return m_frequencyMin; }
    [[nodiscard]] double frequencyMax() const { return m_frequencyMax; }
    [[nodiscard]] double frequencyStep() const { return m_frequencyStep; }
    [[nodiscard]] int frequencyDecimals() const { return m_frequencyDecimals; }

    [[nodiscard]] bool isDelayVisible() const { return m_caps.hasDelay; }
    /// -4..4, matching ElementEditor::sliderDelay -- converted to/from the real -0.5..0.5
    /// period-fraction in refresh()/apply().
    [[nodiscard]] int delaySteps() const { return m_delaySteps; }
    void setDelaySteps(int value);

    [[nodiscard]] bool isInputSizeVisible() const { return m_caps.canChangeInputSize; }
    [[nodiscard]] QList<SizeOption> inputSizeOptions() const { return m_inputSizeOptions; }
    [[nodiscard]] int inputSize() const { return m_inputSize; }
    void setInputSize(int value);

    [[nodiscard]] bool isOutputSizeVisible() const { return m_caps.canChangeOutputSize; }
    [[nodiscard]] QList<SizeOption> outputSizeOptions() const { return m_outputSizeOptions; }
    [[nodiscard]] int outputSize() const { return m_outputSize; }
    void setOutputSize(int value);

    [[nodiscard]] bool isOutputValueVisible() const { return m_caps.hasLatchedValue; }
    [[nodiscard]] QList<SizeOption> outputValueOptions() const { return m_outputValueOptions; }
    [[nodiscard]] int outputValue() const { return m_outputValue; }
    void setOutputValue(int value);

    [[nodiscard]] bool isLockedVisible() const { return m_caps.hasOnlyInputs; }
    /// Qt::CheckState int (0 Unchecked / 1 PartiallyChecked / 2 Checked), matching
    /// ElementEditor::checkBoxLocked's tri-state.
    [[nodiscard]] int lockedState() const { return m_lockedState; }
    void setLockedState(int value);

    [[nodiscard]] bool isTriggerVisible() const { return m_caps.hasTrigger; }
    [[nodiscard]] QString trigger() const { return m_trigger; }
    void setTrigger(const QString &value);

    [[nodiscard]] bool isWirelessModeVisible() const { return m_caps.hasWirelessMode; }
    [[nodiscard]] int wirelessMode() const { return m_wirelessMode; }
    void setWirelessMode(int value);
    [[nodiscard]] static QVariantList wirelessModeOptions();

    [[nodiscard]] bool canMorph() const { return m_caps.canMorph; }
    [[nodiscard]] QList<MorphCandidate> morphCandidates() const { return m_morphCandidates; }

    /// Recomputes morphCandidates() from \a item's element group/input size, mirroring
    /// ElementContextMenu::exec()'s per-ElementGroup switch exactly (Gate/StaticInput+Input/
    /// Memory/Output candidate lists, each excluding \a item's own current type via the same
    /// "hasSameType" rule addElementAction() uses). Called from Main.qml's onElementContextMenuRequested.
    Q_INVOKABLE void prepareContextMenu(GraphicElement *item);
    /// Morphs every selected element to \a type. Mirrors the "Morph to..." submenu action.
    Q_INVOKABLE void morphSelectionTo(int type);

    /// Requests keyboard focus land on ElementEditor.qml's label field. Mirrors
    /// ElementEditor::renameAction() (focuses + selects lineEditElementLabel).
    Q_INVOKABLE void requestRenameFocus() { emit focusLabelRequested(); }
    /// Requests keyboard focus land on ElementEditor.qml's trigger field. Mirrors
    /// ElementEditor::changeTriggerAction().
    Q_INVOKABLE void requestTriggerFocus() { emit focusTriggerRequested(); }

signals:
    void refreshed();
    void focusLabelRequested();
    void focusTriggerRequested();

private:
    /// Slot: the canvas's selection changed; re-reads it and refreshes every field. Mirrors
    /// ElementEditor::selectionChanged()/setCurrentElements().
    void onSelectionChanged();
    /// Recomputes m_caps and every field's display value from m_elements, clearing every
    /// field's dirty flag. Mirrors ElementEditor::applyCapabilitiesToUi().
    void refresh();
    /// Pushes a CanvasUpdateCommand for every dirty field, mirroring ElementEditor::apply() --
    /// including its duplicate-wireless-Tx-channel guard and connection-severing macro.
    /// Called at the end of every setter, matching each Widgets control's signal being wired
    /// straight to apply().
    void apply();
    /// Applies one property to \a elm if it's not a skipped (mixed, untouched) field. Mirrors
    /// ElementEditor::applyProperty()'s per-type switch, scoped to this pass's property set.
    void applyProperty(GraphicElement *elm);

    CanvasItem *m_canvas = nullptr;
    QList<GraphicElement *> m_elements;
    SelectionCapabilities m_caps;

    QString m_label;
    bool m_labelDirty = false;
    QString m_color;
    bool m_colorDirty = false;
    QString m_audio;
    bool m_audioDirty = false;
    int m_volume = 100;
    bool m_volumeDirty = false;
    double m_frequency = 1.0;
    double m_frequencyMin = 0.1;
    double m_frequencyMax = 50.0;
    double m_frequencyStep = 0.1;
    int m_frequencyDecimals = 1;
    bool m_frequencyDirty = false;
    int m_delaySteps = 0;
    bool m_delayDirty = false;
    QList<SizeOption> m_inputSizeOptions;
    int m_inputSize = 0;
    QList<SizeOption> m_outputSizeOptions;
    int m_outputSize = 0;
    QList<SizeOption> m_outputValueOptions;
    int m_outputValue = 0;
    bool m_outputValueDirty = false;
    int m_lockedState = 0;
    bool m_lockedDirty = false;
    QString m_trigger;
    bool m_triggerDirty = false;
    int m_wirelessMode = 0;
    bool m_wirelessModeDirty = false;

    QList<MorphCandidate> m_morphCandidates;
};
