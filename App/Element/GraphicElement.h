// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Abstract base class for all graphical circuit elements.
 */

#pragma once

#include <memory>

#include <QBitArray>
#include <QGraphicsItem>
#include <QKeySequence>
#include <QList>
#include <QVersionNumber>

#include "App/Core/Enums.h"
#include "App/Core/ItemWithId.h"
#include "App/Element/ElementAppearance.h"
#include "App/Element/ElementSimState.h"
#include "App/Element/PropertyDescriptor.h"

struct SerializationContext;

class GraphicElement;
class QNEInputPort;
class QNEOutputPort;
class QNEPort;
class QPainter;
class QStyleOptionGraphicsItem;
class QSvgRenderer;
class QWidget;

/**
 * \class GraphicElement
 * \brief Abstract base class for all graphical circuit elements in wiRedPanda.
 *
 * \details Combines a QGraphicsObject (visual representation on the scene) with
 * ItemWithId (stable numeric identity) to form the common interface that every
 * circuit element must implement.  Concrete subclasses cover gates, flip-flops,
 * I/O elements, integrated circuits, and all other element types.
 *
 * Responsibilities handled here:
 * - Port management (input and output QNEPort children).
 * - Pixmap / appearance rendering with default and user-defined appearances.
 * - Serialization to / from a versioned QDataStream (save/load).
 * - Grid-snapping and wire-update callbacks via itemChange().
 * - Label and keyboard-trigger display.
 * - Theme-aware selection highlight painting.
 * - Polymorphic hooks for clock frequency, audio, color, and truth-table
 *   features that only a subset of elements support.
 */
class GraphicElement : public QGraphicsObject, public ItemWithId
{
    Q_OBJECT
public:
    // --- Type Info ---

    enum { Type = QGraphicsItem::UserType + 3 };

    /// Returns the custom type identifier for this item.
    int type() const override { return Type; }

    // --- Lifecycle ---

    /// Constructs a graphic element of the given \a type, fetching all properties from the metadata registry.
    explicit GraphicElement(ElementType type, QGraphicsItem *parent = nullptr);

    /// Out-of-line so the unique_ptr to the forward-declared QSvgRenderer can be destroyed.
    ~GraphicElement() override;

    // --- External file dependencies ---

    /// Returns the list of external (non-resource) file paths this element depends on.
    /// Used by the save infrastructure to copy files into the project directory.
    virtual QStringList externalFiles() const;

    // --- Serialization ---

    /// Saves the graphic element through a binary data stream.
    virtual void save(QDataStream &stream) const;

    /**
     * \brief Loads the graphic element through a binary data stream.
     * \param context carries portMap, version, contextDir and optional copy-operation state.
     */
    virtual void load(QDataStream &stream, SerializationContext &context);

    /// Returns the context directory stored during load() for path resolution.
    QString loadContextDir() const { return m_contextDir; }

    // --- Port Management ---

    /**
     * \brief Repositions and reconfigures all ports after the port count changes.
     * \details Distributes input ports vertically on the left edge (x=0) and
     * output ports on the right edge (x=64), spaced by half the scene grid size.
     * Also reapplies rotation for non-rotatable elements.  Called after
     * construction, loading, and any input/output size change.
     */
    virtual void updatePortsProperties();

    /**
     * \brief Returns the wireless routing mode for this element.
     * \details Returns WirelessMode::None for all elements except Node, which overrides
     * this to expose its configurable Tx/Rx mode.
     * \return WirelessMode::None for all base elements.
     */
    virtual WirelessMode wirelessMode() const;

    /**
     * \brief Returns true if this element supports a configurable wireless routing mode.
     * \details Returns false in the base class.  Node overrides this to return true.
     * \return true when the element has a wirelessMode() that can be set.
     */
    virtual bool hasWirelessMode() const;

    // --- Element Type & Identity ---

    /// Returns the group this element belongs to.
    ElementGroup elementGroup() const;

    /// Returns the type identifier for this element.
    ElementType elementType() const;

    // --- Port Access ---

    /// Returns the input port at \a index (default 0).
    QNEInputPort *inputPort(const int index = 0);

    /// Returns the output port at \a index (default 0).
    QNEOutputPort *outputPort(const int index = 0);

    /// Returns a const reference to the vector of all input ports.
    const QVector<QNEInputPort *> &inputs() const;

    /// Returns a const reference to the vector of all output ports.
    const QVector<QNEOutputPort *> &outputs() const;

    /// Returns a combined list of all input and output ports as QNEPort pointers.
    QVector<QNEPort *> allPorts() const;

    /// Returns the current number of input ports.
    int inputSize() const;

    /// Returns the current number of output ports.
    int outputSize() const;

    // --- Port Size Constraints ---

    /// Returns the maximum allowed number of input ports.
    int maxInputSize() const;

    /// Returns the minimum allowed number of input ports.
    int minInputSize() const;

    /// Returns the maximum allowed number of output ports.
    int maxOutputSize() const;

    /// Returns the minimum allowed number of output ports.
    int minOutputSize() const;

    // --- Labeling ---

    /// Returns the user-visible label text for this element.
    QString label() const;

    /// Sets the label text to \a label and refreshes the display.
    void setLabel(const QString &label);

    /// Repositions and updates the label child item to reflect current state.
    void updateLabel();

    /// Returns \c true if this element type supports a user-editable label.
    bool hasLabel() const;

    // --- Embedded IC ---

    /// Returns true if this element is an embedded IC (not file-backed). Base returns false.
    virtual bool isEmbedded() const { return false; }

    /// Returns the blob name for embedded ICs, empty string otherwise. Base returns empty.
    virtual const QString &blobName() const { static const QString empty; return empty; }

    // --- Trigger Control ---

    /// Returns the keyboard shortcut that activates this element.
    QKeySequence trigger() const;

    /// Sets the keyboard shortcut to \a trigger and updates the label.
    void setTrigger(const QKeySequence &trigger);

    /// Returns \c true if this element type supports a keyboard trigger.
    bool hasTrigger() const;

    // --- Audio Properties ---

    /// Returns the name of the audio file currently associated with this element.
    virtual QString audio() const;

    /// Sets the audio file associated with this element to \a audio.
    virtual void setAudio(const QString &audio);

    /// Returns \c true if this element type supports audio output.
    bool hasAudio() const;

    /// Returns \c true if this element type shows an audio selection box.
    bool hasAudioBox() const;

    /// Returns \c true if this element type supports volume control.
    bool hasVolume() const;

    /// Returns the audio playback volume (0.0–1.0).
    virtual float volume() const;

    /// Sets the audio playback volume to \a vol (0.0–1.0).
    virtual void setVolume(float vol);

    // --- Color Properties ---

    /// Returns the name of the color currently applied to this element.
    virtual QString color() const;

    /// Sets the element color to \a color and refreshes the pixmap.
    virtual void setColor(const QString &color);

    /// Returns \c true if this element type supports color selection.
    bool hasColors() const;

    /// Maps a color name ("White","Red","Green","Blue","Purple") to indices 0–4.
    static int colorNameToIndex(const QString &color);

    /// Returns the name of the next color in the element's color list.
    QString nextColor() const;

    /// Returns the name of the previous color in the element's color list.
    QString previousColor() const;

    // --- Frequency & Delay ---

    /// Returns the clock frequency in Hz (overridden by Clock; returns 0 for other elements).
    virtual double frequency() const;

    /// Returns the clock phase delay in seconds (overridden by Clock; returns 0 for other elements).
    virtual double delay() const;

    /// Returns \c true if this element type exposes a configurable clock frequency.
    bool hasFrequency() const;

    /// Returns \c true if this element type exposes a configurable clock phase delay.
    bool hasDelay() const;

    /// Sets the clock frequency to \a freq (overridden by clock elements).
    virtual void setFrequency(const double freq);

    /// Sets the clock phase delay to \a delay (overridden by clock elements).
    virtual void setDelay(const double delay);

    /**
     * \brief Returns the list of editable properties this element exposes in the ElementEditor.
     * \details Derived from metadata flags; no override needed in subclasses.
     * \return Ordered list of PropertyDescriptors for all supported property types.
     */
    QList<PropertyDescriptor> editableProperties() const;

    // --- Appearance Management ---

    /// Returns \c true if the user is allowed to choose a custom appearance for this element.
    bool canChangeAppearance() const;

    /**
     * \brief Switches the element's appearance.
     * \param defaultAppearance \c true to restore the built-in default appearance.
     * \param fileName    File path of the custom appearance image (used when \a defaultAppearance is \c false).
     */
    virtual void setAppearance(const bool defaultAppearance, const QString &fileName);

    /**
     * \brief Sets a custom appearance at a specific index in the appearance list.
     * \param index Appearance list index.
     * \param fileName File path of the custom appearance image (empty restores default for that index).
     */
    void setAppearanceAt(const int index, const QString &fileName);

    /**
     * \brief Returns the list of editable appearance states for this element.
     * Each pair is (appearance list index, human-readable state description).
     * Override in subclasses with multi-state appearances (e.g., Led).
     */
    virtual QList<QPair<int, QString>> appearanceStates() const;

    /// Loads and applies the pixmap located at \a pixmapPath.
    void setPixmap(const QString &pixmapPath);

    /// Loads and applies the appearance at position \a index in the appearance list.
    void setPixmap(const int index);

    // --- Truth Table ---

    /// Returns \c true if this element type has an editable truth table.
    bool hasTruthTable() const;

    // --- Rotation ---

    /// Returns the current rotation angle of this element in degrees.
    qreal rotation() const;

    /// Rotates the element to \a angle degrees and updates port positions.
    void setRotation(const qreal angle);

    /// Returns \c true if rotating/flipping re-orients this element's graphic. When \c false the
    /// icon stays fixed and only the ports reposition around the centre (the element still rotates).
    bool rotatesGraphic() const;

    /// Re-applies the combined rotation + flip orientation to every port. Used by non-rotatable
    /// elements, which keep their pixmap fixed and move only their ports around the centre.
    void rotatePorts();

    // --- Flip / Mirror ---

    /// Returns \c true if this element is mirrored along the X axis (horizontal flip).
    bool isFlippedX() const { return m_flippedX; }

    /// Returns \c true if this element is mirrored along the Y axis (vertical flip).
    bool isFlippedY() const { return m_flippedY; }

    /// Sets the horizontal mirror state and updates the item transform.
    void setFlippedX(bool flipped);

    /// Sets the vertical mirror state and updates the item transform.
    void setFlippedY(bool flipped);

    // --- Geometric Properties ---

    /// Returns the centre point of the element's pixmap in local coordinates.
    QPointF pixmapCenter() const;

    /// Returns the bounding rectangle of this element in local coordinates.
    QRectF boundingRect() const override;

    // --- State Queries ---

    /// Returns \c true if the element is fully initialised and connected correctly.
    bool isValid();

    // --- Direct Simulation Interface ---

    /**
     * \brief Computes this element's output values from its current inputs.
     * \details Override in subclasses to implement element-specific logic.
     */
    virtual void updateLogic();

    /**
     * \brief Resets all simulation-visible state to power-on defaults.
     * \details The base implementation resets each output slot to its port's
     * defaultStatus().  Sequential elements (flip-flops, latches) override this
     * to also clear their internal edge-detection variables (m_simLastClk, etc.)
     * so that BeWavedDolphin can start every sweep from a known, reproducible state.
     */
    virtual void resetSimState();

    /**
     * \brief Re-evaluates combinational outputs after the synchronous sequential
     * commit, propagating just-committed flip-flop/latch state to downstream
     * logic and IC output boundaries within the same tick.
     * \details Called only on non-sequential elements (the simulation skips
     * ElementGroup::Memory so their edge state is not disturbed). The default
     * recomputes via updateLogic(); IC overrides it to recurse through its
     * internals while skipping its own sequential elements.
     */
    virtual void resettleCombinational() { updateLogic(); }

    /// Returns the four-state signal value on simulation output port \a index.
    inline Status outputValue(const int index = 0) const { return m_sim.outputValue(index); }

    /// Returns the number of simulation output slots.
    qsizetype simOutputSize() const { return m_sim.outputSize(); }

    /// Sets simulation output port \a index to \a value.
    inline void setOutputValue(const int index, const Status value) { m_sim.setOutputValue(index, value); }

    /// Sets simulation output port 0 to \a value.
    inline void setOutputValue(const Status value) { m_sim.setOutputValue(0, value); }

    /// Convenience overload — converts \c bool to Active/Inactive for port \a index.
    void setOutputValue(const int index, const bool value) { m_sim.setOutputValue(index, value ? Status::Active : Status::Inactive); }

    /// Convenience overload — converts \c bool to Active/Inactive for port 0.
    void setOutputValue(const bool value) { m_sim.setOutputValue(0, value ? Status::Active : Status::Inactive); }

    /// Connects simulation input \a inputIndex to output \a outputPort of \a source element.
    void connectPredecessor(const int inputIndex, GraphicElement *source, const int outputPort);

    /// Returns \c true if any simulation output changed since the flag was
    /// last cleared (it does NOT reset the flag — see clearOutputChanged()).
    bool outputChanged() const { return m_sim.outputChanged(); }

    /// Clears the simulation output-changed flag.
    void clearOutputChanged() { m_sim.clearOutputChanged(); }

    /// Begins a deferred (non-blocking) output commit window; routes subsequent
    /// setOutputValue() calls to a staging buffer so peers read the pre-tick value.
    void beginDeferredCommit() { m_sim.beginDeferredCommit(); }

    /// Ends the deferred-commit window and publishes staged outputs through the
    /// normal change-detecting path so visuals refresh correctly.
    void commitDeferredOutputs() { m_sim.commitDeferredOutputs(); }

    /// Allocates simulation I/O vectors with \a inputs inputs and \a outputs outputs.
    void initSimulationVectors(const int inputCount, const int outputCount);

    /// Polymorphic interface for drag-drop initialization (replaces elementType() == IC checks).
    virtual void loadFromDrop(const QString &fileName, const QString &contextDir);

    // --- Virtual Methods ---

    /// Returns a string encoding element-specific properties for serialization or display.
    virtual QString genericProperties();

    /// Forces a visual refresh of the element's pixmap and ports.
    virtual void refresh();

    // --- Qt Graphics & Display ---

    /**
     * \brief Paints the element onto the scene.
     * \details Draws a rounded selection rectangle when the item is selected,
     * then draws the current pixmap at the item origin.
     * \param painter Painter provided by the graphics view framework.
     * \param option  Style options (unused).
     * \param widget  Target widget (unused).
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /**
     * \brief Updates the translated display name, tooltip, and port object name after a locale change.
     * \details Queries ElementFactory for the new translated name and propagates
     * it to setPortName() and setToolTip().
     */
    void retranslate();

    // --- Setters (Port & Logic Configuration) ---

    /// Adjusts the number of input ports to \a size, adding or removing ports as needed.
    virtual void setInputSize(const int size);

    /// Adjusts the number of output ports to \a size, adding or removing ports as needed.
    virtual void setOutputSize(const int size);

    /// Replaces the input port vector with \a inputs.
    void setInputs(const QVector<QNEInputPort *> &inputs);

    /// Sets the object name of all ports to \a name for identification.
    void setPortName(const QString &name);

    /// Read-only view of the cached simulation input values.
    const QVector<Status> &simInputs() const { return m_sim.inputs(); }

    /// Read-only view of the current simulation output values.
    const QVector<Status> &simOutputs() const { return m_sim.outputs(); }

    // --- Theme ---

    /// Updates the element's visual theme according to the current dark/light palette.
    virtual void updateTheme();

protected:
    // --- Graphics & Rendering ---

    /// Returns the pixmap currently displayed by this element.
    QPixmap pixmap() const;

    /**
     * \brief Returns the bounding rectangle that encompasses all child ports.
     * \details Iterates over childItems(), mapping each QNEPort's bounding rect
     * into the element's local coordinate space.
     * \return Combined bounding QRectF of all port children.
     */
    QRectF portsBoundingRect() const;

    // --- Qt Event Handling ---

    /**
     * \brief Handles item state changes such as position, rotation, and selection.
     * \details Three change types are intercepted:
     * - ItemPositionChange: snaps the new position to half the scene grid size.
     * - ItemScenePositionHasChanged / ItemRotationHasChanged / ItemTransformHasChanged:
     *   calls updateConnections() on all ports to redraw attached wires.
     * - ItemSelectedHasChanged: toggles connection highlight via highlight().
     * \param change The type of change that occurred.
     * \param value  The new value associated with the change.
     * \return The (possibly modified) value to use, or the base class result.
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /**
     * \brief Intercepts mouse-press and mouse-release events to handle Ctrl+click.
     * \details When Ctrl is held during a mouse press or release, the event is
     * consumed here and not forwarded, preventing accidental multi-selection.
     * \param event The scene event to inspect.
     * \return true if the event was consumed; otherwise the base class result.
     */
    bool sceneEvent(QEvent *event) override;

    // --- Capability Setters ---

    /// Sets whether this element type supports color selection.
    void setHasColors(const bool hasColors);

    // --- Port Size Constraint Setters ---

    /// Sets the maximum number of input ports to \a maxInputSize.
    void setMaxInputSize(const int maxInputSize);

    /// Sets the minimum number of input ports to \a minInputSize.
    void setMinInputSize(const int minInputSize);

    /// Sets the maximum number of output ports to \a maxOutputSize.
    void setMaxOutputSize(const int maxOutputSize);

    /// Sets the minimum number of output ports to \a minOutputSize.
    void setMinOutputSize(const int minOutputSize);

    /// All input ports owned by this element (ordered by index).
    QVector<QNEInputPort *> m_inputPorts;

    /// All output ports owned by this element (ordered by index).
    QVector<QNEOutputPort *> m_outputPorts;

    /// Owns the pixmap/SVG appearance, the appearance list, and selection-highlight colors;
    /// this element forwards its rendering and appearance interface here.  See ElementAppearance.
    ElementAppearance m_appearance{this};

    QGraphicsSimpleTextItem *m_label = new QGraphicsSimpleTextItem(this); ///< Child text item that displays the label and optional trigger shortcut.

    // --- Members: Metadata ---

    QString m_titleText;      ///< Translated title text shown in UI panels (from metadata).
    QString m_translatedName; ///< Translated element name used as tooltip and port object name.

    // --- Direct Simulation Helpers ---

    /**
     * \brief Snapshots each predecessor's output into the simulation input cache.
     *
     * If any predecessor output is Unknown or Error, all outputs are set to
     * Unknown and the method returns \c false so that sequential elements
     * (flip-flops, latches) can skip computation with incomplete data.
     * Unconnected inputs (null predecessor) use the corresponding port's
     * defaultStatus(), replacing the old global GND/VCC approach.
     *
     * \return \c true if all inputs are Active or Inactive (simulation can proceed).
     */
    bool simUpdateInputs() { return m_sim.updateInputs(false, m_inputPorts); }

    /**
     * \brief Like simUpdateInputs(), but allows Unknown/Error values through.
     * \details Combinational gates use this so their domination rules
     * (e.g. AND(0, X) = 0) can short-circuit even with unknown inputs.
     * Only a truly unconnected input (null predecessor whose port default
     * is Unknown) triggers an early all-outputs-Unknown return.
     */
    bool simUpdateInputsAllowUnknown() { return m_sim.updateInputs(true, m_inputPorts); }

    /**
     * \brief Decodes \a count select-line statuses from simInputs() into a binary index.
     * \param offset  Index of the first select-line input in simInputs().
     * \param count   Number of select lines to read.
     * \return An integer where bit i is 1 if simInputs()[offset+i] == Active.
     * \details Used by Mux and Demux to convert select-line signals into a data-port index.
     */
    int decodeSelectValue(int offset, int count) const { return m_sim.decodeSelectValue(offset, count); }

private:
    Q_DISABLE_COPY_MOVE(GraphicElement)

    // --- Flip Transform ---

    /// Recomputes the QGraphicsItem transform from the current flip flags.
    void applyFlipTransform();

    /// Orients \a port for the current rotation + flip state (used by non-rotatable elements,
    /// which keep their pixmap fixed). Applies Rotate(centre, m_angle) then Flip about the
    /// pixmap centre to the port, so the port moves to the mirrored/rotated side while the
    /// element graphic stays upright. Recomputed from the flags each call, so it is involutive.
    void applyPortOrientation(QNEPort *port);

    // --- Port Management Helpers ---

    /// Appends a new input port with optional \a name to the input port vector.
    void addInputPort(const QString &name = {});

    /// Appends a new output port with optional \a name to the output port vector.
    void addOutputPort(const QString &name = {});

    /// Appends a new port with optional \a name; \a isOutput selects the port direction.
    void addPort(const QString &name, const bool isOutput);

    /// Shared implementation for setInputSize() and setOutputSize().
    void setPortSize(const int size, const bool isInput);

    /// Erases \a deletedPort's serial-ID entry from \a portMap during deserialization.
    static void removePortFromMap(QNEPort *deletedPort, QMap<quint64, QNEPort *> &portMap);

    /// Removes input ports beyond \a inputSize_ (used when the loaded port count exceeds current limits).
    void removeSurplusInputs(const quint64 inputSize_, SerializationContext &context);
    /// Removes output ports beyond \a outputSize_ (used when the loaded port count exceeds current limits).
    void removeSurplusOutputs(const quint64 outputSize_, SerializationContext &context);

    // --- Serialization Helpers ---

    /// Deserializes element state from the V4.6+ binary format.
    void loadNewFormat(QDataStream &stream, SerializationContext &context);
    /// Deserializes element state from pre-V4.6 binary formats.
    void loadOldFormat(QDataStream &stream, SerializationContext &context);

    // --- Position Loading ---

    /// Reads and applies the element's scene position from \a stream.
    void loadPos(QDataStream &stream);

    // --- Port Loading ---

    /// Reads a single input port at \a port index from \a stream.
    void loadInputPort(QDataStream &stream, SerializationContext &context, const int port);
    /// Reads all input ports from \a stream.
    void loadInputPorts(QDataStream &stream, SerializationContext &context);
    /// Reads a single output port at \a port index from \a stream.
    void loadOutputPort(QDataStream &stream, SerializationContext &context, const int port);
    /// Reads all output ports from \a stream.
    void loadOutputPorts(QDataStream &stream, SerializationContext &context);
    /// Reads and applies input/output port counts from \a stream, clamped to current constraints.
    void loadPortsSize(QDataStream &stream, const QVersionNumber &version);

    // --- Property Loading ---

    /// Reads the element label from \a stream (format depends on \a version).
    void loadLabel(QDataStream &stream, const QVersionNumber &version);
    /// Reads and applies a single appearance path at the given \a index from \a stream.
    void loadPixmapAppearanceName(QDataStream &stream, const int index, const QString &contextDir);
    /// Reads and applies all appearance paths from \a stream.
    void loadPixmapAppearanceNames(QDataStream &stream, SerializationContext &context);
    /// Reads and applies the element rotation from \a stream.
    void loadRotation(QDataStream &stream, const QVersionNumber &version);
    /// Reads and applies the keyboard trigger from \a stream.
    void loadTrigger(QDataStream &stream, const QVersionNumber &version);

    // --- Display & Interaction ---

    /**
     * \brief Highlights or un-highlights all connections attached to this element.
     * \details Iterates over every port and every connection, toggling the highLight
     * flag to match the element's new selection state.
     * \param isSelected true when the element has just been selected.
     */
    void highlight(const bool isSelected);

    // --- Members: Element Type & Identity ---

    ElementGroup m_elementGroup = ElementGroup::Unknown;
    ElementType m_elementType = ElementType::Unknown;

    // --- Members: Direct Simulation ---

    /// Owns the simulation runtime state (I/O values + connection graph); this element
    /// forwards its direct-simulation interface here.  See ElementSimState.
    ElementSimState m_sim;

    // --- Members: Trigger & Label ---

    QKeySequence m_trigger;
    QString m_labelText;

    // --- Members: Capabilities & Display State ---

    bool m_hasColors = false;
    bool m_selected = false;
    bool m_flippedX = false;
    bool m_flippedY = false;
    qreal m_angle = 0;

    // --- Members: Context Directory ---

    /// Directory of the .panda file from which this element was loaded.
    /// Used by resolveContextDir() for elements not yet added to a scene.
    QString m_contextDir;

    // --- Members: Port Size Constraints ---

    quint64 m_minInputSize = 0;
    quint64 m_maxInputSize = 0;
    quint64 m_minOutputSize = 0;
    quint64 m_maxOutputSize = 0;
};

/// Stream insertion operator that serializes \a item to \a stream via GraphicElement::save().
QDataStream &operator<<(QDataStream &stream, const GraphicElement *item);
