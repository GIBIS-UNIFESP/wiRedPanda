// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickElementEditor: QML-facing presenter for the element property panel.
 */

#pragma once

#include <QList>
#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QString>
#include <QVariantList>

#include "App/UI/SelectionCapabilities.h"

class CanvasItem;
class GraphicElement;
class TruthTable;

/// One row in truthTableRows() -- cells holds every column's "0"/"1" text, input columns first
/// then output columns (matching truthTableColumnLabels()'s own ordering), so a delegate can
/// tell them apart via `column < root.editor.truthTableInputCount`. Real QML value type for
/// list model data.
class TruthTableRow
{
    Q_GADGET
    QML_VALUE_TYPE(truthTableRow)

    Q_PROPERTY(QStringList cells READ cells FINAL)

public:
    TruthTableRow() = default;
    explicit TruthTableRow(QStringList cells)
        : m_cells(std::move(cells))
    {
    }

    [[nodiscard]] QStringList cells() const { return m_cells; }

private:
    QStringList m_cells;
};

/// One entry in appearanceStates() -- a multi-state element's per-state appearance tile (e.g.
/// Led's color states, Display7's lit-segment states). previewImageUrl is a base64
/// "data:image/png;base64,..." URL (same technique as QuickICPreview::imageUrl -- a one-off,
/// occasionally-regenerated image, not worth a QQuickImageProvider request-id scheme), built
/// from GraphicElement::appearancePreviewPixmap(index, size). Real QML value type for list
/// model data.
class AppearanceStateOption
{
    Q_GADGET
    QML_VALUE_TYPE(appearanceStateOption)

    Q_PROPERTY(int index READ index FINAL)
    Q_PROPERTY(QString label READ label FINAL)
    Q_PROPERTY(QString previewImageUrl READ previewImageUrl FINAL)

public:
    AppearanceStateOption() = default;
    AppearanceStateOption(int index, QString label, QString previewImageUrl)
        : m_index(index)
        , m_label(std::move(label))
        , m_previewImageUrl(std::move(previewImageUrl))
    {
    }

    [[nodiscard]] int index() const { return m_index; }
    [[nodiscard]] QString label() const { return m_label; }
    [[nodiscard]] QString previewImageUrl() const { return m_previewImageUrl; }

private:
    int m_index = 0;
    QString m_label;
    QString m_previewImageUrl;
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
 * A field left untouched (dirty == false) with hasSame*==false (a real divergent
 * multi-selection) is skipped by apply() exactly like ElementEditor::applyProperty() skips it;
 * touching it applies the new value to every selected element, also matching production.
 *
 * \details Appearance, AudioBox, TruthTable, and embedded-IC blob rename are also ported here --
 * see their own Q_PROPERTY/Q_INVOKABLE groups below.
 *
 * \details Also backs the right-click context menu: shares the exact same m_elements/m_caps
 * this class already tracks reactively (canMorph/hasSelection double as the menu's
 * Morph-submenu/Copy-Cut-Delete visibility, colorOptions doubles as the Colors submenu), plus
 * morphCandidates -- computed from whichever element was right-clicked (prepareContextMenu()),
 * not the whole selection, mirroring ElementContextMenu::exec()'s addElementAction() switch
 * exactly. Change-Appearance/Revert-Appearance context-menu items are not built -- the panel's
 * own "Change Appearance..."/"Reset" buttons cover the same operations; only the context-menu
 * shortcut to them is missing.
 */
class QuickElementEditor : public QObject
{
    Q_OBJECT
    // ElementEditor.qml spells this out as an explicit property type (property
    // QuickElementEditor editor: ...) so qmllint can check member access against it --
    // QML_ELEMENT gives it a name to do that with; QML_UNCREATABLE since QML never
    // constructs one itself (QuickAppController owns the only instance).
    QML_ELEMENT
    QML_UNCREATABLE("Created by QuickAppController")

    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY refreshed FINAL)
    Q_PROPERTY(QString elementTypeTitle READ elementTypeTitle NOTIFY refreshed FINAL)

    Q_PROPERTY(bool labelVisible READ isLabelVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY refreshed FINAL)

    Q_PROPERTY(bool colorVisible READ isColorVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY refreshed FINAL)
    Q_PROPERTY(QVariantList colorOptions READ colorOptions CONSTANT FINAL)

    Q_PROPERTY(bool audioVisible READ isAudioVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString audio READ audio WRITE setAudio NOTIFY refreshed FINAL)
    Q_PROPERTY(QVariantList audioOptions READ audioOptions CONSTANT FINAL)

    Q_PROPERTY(bool audioBoxVisible READ isAudioBoxVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString audioBoxFileName READ audioBoxFileName NOTIFY refreshed FINAL)

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
    Q_PROPERTY(QVariantList inputSizeOptions READ inputSizeOptions NOTIFY refreshed FINAL)
    Q_PROPERTY(int inputSize READ inputSize WRITE setInputSize NOTIFY refreshed FINAL)

    Q_PROPERTY(bool outputSizeVisible READ isOutputSizeVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QVariantList outputSizeOptions READ outputSizeOptions NOTIFY refreshed FINAL)
    Q_PROPERTY(int outputSize READ outputSize WRITE setOutputSize NOTIFY refreshed FINAL)

    Q_PROPERTY(bool outputValueVisible READ isOutputValueVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QVariantList outputValueOptions READ outputValueOptions NOTIFY refreshed FINAL)
    Q_PROPERTY(int outputValue READ outputValue WRITE setOutputValue NOTIFY refreshed FINAL)

    Q_PROPERTY(bool lockedVisible READ isLockedVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(int lockedState READ lockedState WRITE setLockedState NOTIFY refreshed FINAL)

    Q_PROPERTY(bool triggerVisible READ isTriggerVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString trigger READ trigger WRITE setTrigger NOTIFY refreshed FINAL)

    Q_PROPERTY(bool wirelessModeVisible READ isWirelessModeVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(int wirelessMode READ wirelessMode WRITE setWirelessMode NOTIFY refreshed FINAL)
    Q_PROPERTY(QVariantList wirelessModeOptions READ wirelessModeOptions CONSTANT FINAL)

    Q_PROPERTY(bool canMorph READ canMorph NOTIFY refreshed FINAL)
    Q_PROPERTY(QVariantList morphCandidates READ morphCandidates NOTIFY refreshed FINAL)

    Q_PROPERTY(bool appearanceVisible READ isAppearanceVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QList<AppearanceStateOption> appearanceStates READ appearanceStates NOTIFY refreshed FINAL)
    Q_PROPERTY(int appearanceStateIndex READ appearanceStateIndex WRITE setAppearanceStateIndex NOTIFY refreshed FINAL)

    Q_PROPERTY(bool truthTableVisible READ isTruthTableVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QStringList truthTableColumnLabels READ truthTableColumnLabels NOTIFY truthTableChanged FINAL)
    Q_PROPERTY(int truthTableInputCount READ truthTableInputCount NOTIFY truthTableChanged FINAL)
    Q_PROPERTY(QList<TruthTableRow> truthTableRows READ truthTableRows NOTIFY truthTableChanged FINAL)

    Q_PROPERTY(bool blobNameVisible READ isBlobNameVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(QString blobName READ blobName NOTIFY refreshed FINAL)

    // Embed/extract actions for a selected IC -- mirrors
    // ElementEditor::embedSubcircuitRequested()/extractToFileRequested()'s buttons, but this
    // class dispatches straight to AppController.embedSelectedIC()/extractSelectedIC() from QML
    // rather than emitting a signal for MainWindow to relay -- there's no separate Quick-side
    // "app shell relay" layer to mirror that indirection through.
    Q_PROPERTY(bool embedIcVisible READ isEmbedIcVisible NOTIFY refreshed FINAL)
    Q_PROPERTY(bool extractIcVisible READ isExtractIcVisible NOTIFY refreshed FINAL)

    // "Edit sub-circuit" -- ElementContextMenu.cpp's onEditSubcircuit branch, shown for either
    // an embedded or a file-backed IC (unlike embed/extract above, which are each scoped to just
    // one of those two). The action itself dispatches straight to
    // AppController.editSelectedSubcircuit(), same shape as embedSelectedIC()/extractSelectedIC().
    Q_PROPERTY(bool editSubcircuitVisible READ isEditSubcircuitVisible NOTIFY refreshed FINAL)

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
    [[nodiscard]] static QVariantList colorOptions();

    [[nodiscard]] bool isAudioVisible() const { return m_caps.hasAudio; }
    [[nodiscard]] QString audio() const { return m_audio; }
    void setAudio(const QString &value);
    [[nodiscard]] static QVariantList audioOptions();

    [[nodiscard]] bool isAudioBoxVisible() const { return m_caps.hasAudioBox; }
    /// Just the file's base name (matches ElementEditor::applyCapabilitiesToUi()'s
    /// QFileInfo(...).fileName() display, not the full path) -- "<Multiple>" style placeholder
    /// for a multi-selection, mirroring the same unconditional-placeholder rule production uses
    /// here specifically (m_elements.size() > 1 ? m_manyAudios : ...), unlike every other field
    /// in this class, which just shows the first element's value without a consensus check.
    [[nodiscard]] QString audioBoxFileName() const;
    /// Opens a native "select an audio file" dialog and, if a file was picked, sets it directly
    /// on the selected AudioBox element -- mirrors ElementEditor::audioBox() exactly, including
    /// its real production quirk of NOT going through apply()/a CanvasUpdateCommand (setAudio()
    /// on an AudioBox is not undoable in production either; preserved as-is).
    Q_INVOKABLE void pickAudioBoxFile();

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
    [[nodiscard]] QVariantList inputSizeOptions() const { return m_inputSizeOptions; }
    [[nodiscard]] int inputSize() const { return m_inputSize; }
    void setInputSize(int value);

    [[nodiscard]] bool isOutputSizeVisible() const { return m_caps.canChangeOutputSize; }
    [[nodiscard]] QVariantList outputSizeOptions() const { return m_outputSizeOptions; }
    [[nodiscard]] int outputSize() const { return m_outputSize; }
    void setOutputSize(int value);

    [[nodiscard]] bool isOutputValueVisible() const { return m_caps.hasLatchedValue; }
    [[nodiscard]] QVariantList outputValueOptions() const { return m_outputValueOptions; }
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
    [[nodiscard]] QVariantList morphCandidates() const { return m_morphCandidates; }

    [[nodiscard]] bool isAppearanceVisible() const { return m_caps.canChangeAppearance; }
    /// Populated in refresh() only when canChangeAppearance is true AND exactly one element is
    /// selected (mirrors applyCapabilitiesToUi()'s identical "m_elements.size() == 1" gate) --
    /// empty for a single-state element (appearanceStates().size() == 1) too, so QML's own
    /// "appearanceStates.length > 1" check decides whether to show the tile grid at all.
    [[nodiscard]] QList<AppearanceStateOption> appearanceStates() const { return m_appearanceStates; }
    /// Which tile is selected (only meaningful while appearanceStates().size() > 1) -- pure UI
    /// selection state, mirrors rebuildAppearanceStateTiles()'s "default-select the first tile".
    /// No apply() on its own; changeAppearance() reads it at file-pick time.
    [[nodiscard]] int appearanceStateIndex() const { return m_appearanceStateIndex; }
    void setAppearanceStateIndex(int index);
    /// Opens an image file picker (QImageReader::supportedImageFormats() filter, mirrors
    /// ElementEditor::updateElementAppearance() exactly). With a multi-state tile selected,
    /// applies directly to that state via a CanvasUpdateCommand (undoable, mirrors
    /// updateElementAppearance()'s tile-grid branch); otherwise sets the one-shot
    /// m_isUpdatingAppearance/m_appearanceName flags applyProperty()'s Appearance case reads,
    /// then calls apply() (undoable through the normal snapshot path).
    Q_INVOKABLE void changeAppearance();
    /// Restores the built-in default appearance -- mirrors ElementEditor::defaultAppearance().
    Q_INVOKABLE void resetAppearance();

    [[nodiscard]] bool isTruthTableVisible() const { return m_caps.hasTruthTable; }
    [[nodiscard]] QStringList truthTableColumnLabels() const { return m_truthTableColumnLabels; }
    [[nodiscard]] int truthTableInputCount() const { return m_truthTableInputCount; }
    [[nodiscard]] QList<TruthTableRow> truthTableRows() const { return m_truthTableRows; }
    /// (Re)builds truthTableColumnLabels/truthTableInputCount/truthTableRows from the single
    /// selected TruthTable element and emits truthTableRequested() so ElementEditor.qml opens
    /// TruthTableDialog.qml. Mirrors ElementEditor::truthTable()'s "Build/refresh table" block
    /// exactly (column labels A/B/C.../S0/S1..., 2^nInputs rows). No-op if the selection isn't
    /// exactly one TruthTable.
    Q_INVOKABLE void openTruthTable();
    /// Toggles the output bit at (\a row, \a column) -- a no-op for an input column (<
    /// truthTableInputCount, read-only, mirrors setTruthTableProposition()'s identical guard).
    /// Pushes the already-ported CanvasToggleTruthTableOutputCommand, then rebuilds the row data
    /// and re-emits truthTableChanged() so the dialog reflects the new state immediately.
    Q_INVOKABLE void toggleTruthTableCell(int row, int column);

    [[nodiscard]] bool isEmbedIcVisible() const { return m_caps.elementType == ElementType::IC && m_caps.isFileBacked; }
    [[nodiscard]] bool isExtractIcVisible() const { return m_caps.elementType == ElementType::IC && m_caps.isEmbedded; }
    [[nodiscard]] bool isEditSubcircuitVisible() const { return m_caps.elementType == ElementType::IC && (m_caps.isFileBacked || m_caps.isEmbedded); }

    [[nodiscard]] bool isBlobNameVisible() const { return m_caps.isEmbedded; }
    /// Computed live from the first selected element rather than cached in refresh() -- same
    /// "no explicit differs hint for a divergent multi-selection, just show the first element's
    /// value" simplification this class already documents for every other field (see class doc
    /// comment). commitBlobRename()'s own newName==oldBlobName check is what this display value
    /// backs, so it stays live rather than snapshotted.
    [[nodiscard]] QString blobName() const;
    /// Mirrors ElementEditor::blobNameEditingFinished(): trims, rejects an empty/unchanged
    /// value, rejects a collision with an already-registered blob (via a Dialogs::provider()
    /// warning), then pushes a standalone CanvasRenameBlobCommand -- not part of apply()'s
    /// generic UpdateCommand snapshot, since renaming is the shared IC registry's blob key, not
    /// a per-element property. Called from ElementEditor.qml's TextField.onEditingFinished.
    Q_INVOKABLE void commitBlobRename(const QString &newName);

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
    /// Requests keyboard focus land on ElementEditor.qml's frequency field. Mirrors
    /// ElementEditor::contextMenu()'s onFrequencyFocus lambda
    /// (m_ui->doubleSpinBoxFrequency->setFocus() -- no select-all there either).
    Q_INVOKABLE void requestFrequencyFocus() { emit focusFrequencyRequested(); }

    /// Tab/Shift+Tab element-cycling: mirrors ElementTabNavigator::eventFilter() exactly (a
    /// no-op unless exactly one element is currently selected), but as an on-demand call instead
    /// of an installed event filter -- called from ElementEditor.qml's per-field
    /// Keys.onTabPressed/onBacktabPressed handlers, one per editable control, each passing its
    /// own \a fieldKey (one of "label", "color", "audio", "volume", "frequency", "delay",
    /// "inputSize", "outputSize", "outputValue", "locked", "trigger", "wirelessMode" -- matching
    /// ElementTabNavigator's own installed-widget list). Advances the canvas selection to the
    /// next/previous element in readingOrder() that also exposes \a fieldKey, wrapping around;
    /// leaves the selection untouched if no other element does. The caller is expected to
    /// re-focus its own control immediately afterward either way, matching
    /// ElementTabNavigator::eventFilter()'s own widget->setFocus() at the very end regardless of
    /// whether the loop found a different element or fell back to the original.
    ///
    /// Deliberately scans the canvas's whole element list, not just the on-screen viewport
    /// portion ElementTabNavigator::readingOrder(scene->visibleElements()) restricts itself to
    /// -- CanvasItem has no viewport-rect query to mirror that with, and cycling through the
    /// entire circuit (scrolling as needed, see centerOn() below) is arguably the more useful
    /// behavior anyway, not just an accepted gap.
    Q_INVOKABLE void cycleSelection(bool forward, const QString &fieldKey);

    /// Orders \a elements in reading order -- top-to-bottom by Y (primary key), left-to-right by
    /// X within a row (secondary key). Mirrors ElementTabNavigator::readingOrder() exactly;
    /// exposed as a static so the ordering is independently testable, same as the original.
    [[nodiscard]] static QList<GraphicElement *> readingOrder(QList<GraphicElement *> elements);

signals:
    void refreshed();
    void focusLabelRequested();
    void focusTriggerRequested();
    void focusFrequencyRequested();
    /// truthTableColumnLabels/truthTableInputCount/truthTableRows changed -- separate from
    /// refreshed() since rebuilding a truth table's row data is real work (up to 256 rows) that
    /// only needs to happen when the dialog is actually open/being edited, not on every
    /// unrelated field's refresh() call.
    void truthTableChanged();
    /// Emitted by openTruthTable() once the row data is ready; ElementEditor.qml's Connections
    /// block opens TruthTableDialog.qml in response.
    void truthTableRequested();

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
    /// Rebuilds m_truthTableColumnLabels/m_truthTableInputCount/m_truthTableRows from \a table's
    /// current inputSize()/outputSize()/key(). Shared by openTruthTable() (first build) and
    /// toggleTruthTableCell() (rebuild after a toggle) -- mirrors ElementEditor::truthTable()'s
    /// own reuse (called again at the end of setTruthTableProposition()).
    void rebuildTruthTableRows(TruthTable *table);

    QPointer<CanvasItem> m_canvas;
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
    QVariantList m_inputSizeOptions;
    int m_inputSize = 0;
    QVariantList m_outputSizeOptions;
    int m_outputSize = 0;
    QVariantList m_outputValueOptions;
    int m_outputValue = 0;
    bool m_outputValueDirty = false;
    int m_lockedState = 0;
    bool m_lockedDirty = false;
    QString m_trigger;
    bool m_triggerDirty = false;
    int m_wirelessMode = 0;
    bool m_wirelessModeDirty = false;

    QVariantList m_morphCandidates;

    // --- Appearance (single-shot flags, mirroring ElementEditor's identically-named members:
    // set by changeAppearance()/resetAppearance(), read once by applyProperty()'s Appearance
    // case, then reset -- not a per-field "dirty" bool like every other property above, since
    // production's own apply() flow works the same one-shot way here) ---
    bool m_isUpdatingAppearance = false;
    bool m_isDefaultAppearance = true;
    QString m_appearanceName;
    QList<AppearanceStateOption> m_appearanceStates;
    int m_appearanceStateIndex = 0;

    QStringList m_truthTableColumnLabels;
    int m_truthTableInputCount = 0;
    QList<TruthTableRow> m_truthTableRows;
};
