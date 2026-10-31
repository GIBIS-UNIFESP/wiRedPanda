// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Abstract base class for all graphical circuit elements.
 */

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <utility>

#include <QBitArray>
#include <QGraphicsItem>
#include <QKeySequence>
#include <QList>

#include "App/Core/Constants.h"
#include "App/Core/Enums.h"
#include "App/Core/ItemWithId.h"
#include "App/Element/BodyMeasure.h"
#include "App/Element/ElementAppearance.h"
#include "App/Element/ElementChange.h"
#include "App/Element/ElementOrientation.h"
#include "App/Element/ElementPorts.h"
#include "App/Element/ElementSimState.h"
#include "App/Element/ElementTypeDescriptor.h"
#include "App/Element/PropertyDescriptor.h"

struct SerializationContext;
struct SerializationOptions;

class Element;
class InputPort;
class OutputPort;
class Port;
class QPainter;
class QStyleOptionGraphicsItem;
class QSvgRenderer;
class QWidget;

/**
 * \class Element
 * \brief Abstract base class for all graphical circuit elements in wiRedPanda.
 *
 * \details Combines a QGraphicsObject (visual representation on the scene) with
 * ItemWithId (stable numeric identity) to form the common interface that every
 * circuit element must implement.  Concrete subclasses cover gates, flip-flops,
 * I/O elements, integrated circuits, and all other element types.
 *
 * Responsibilities handled here:
 * - Port management (input and output Port children).
 * - Pixmap / appearance rendering with default and user-defined appearances.
 * - Serialization to / from a versioned QDataStream (save/load).
 * - Wire-update callbacks via itemChange(); the grid every placing edit snaps to (snapToGrid()).
 * - Label and keyboard-trigger display.
 * - The body's shape, measured by the installed BodyMeasure, and its painting through the installed BodyPainter.
 * - Polymorphic hooks for clock frequency, audio, color, and truth-table
 *   features that only a subset of elements support.
 */
class Element : public QGraphicsObject, public ItemWithId
{
    Q_OBJECT
public:
    // --- Type Info ---

    enum { Type = QGraphicsItem::UserType + 3 };

    /// Returns the custom type identifier for this item.
    int type() const override { return Type; }

    // --- Lifecycle ---

    /// Constructs a graphic element of the given \a type, fetching all properties from its type's descriptor.
    explicit Element(ElementType type, QGraphicsItem *parent = nullptr);

    /// Out-of-line so the unique_ptr to the forward-declared QSvgRenderer can be destroyed.
    ~Element() override;

signals:
    /// Emitted on double-click for a labelable element (hasLabel()) whose type doesn't already
    /// claim double-click for something else (IC/TruthTable override mouseDoubleClickEvent()
    /// directly and this signal never fires for them). The owning Scene listens and drives the
    /// actual inline edit widget -- see InlineLabelEditor.
    void inlineEditRequested(Element *element);

public:
    // --- External file dependencies ---

    /// Returns the list of external (non-resource) file paths this element depends on.
    /// Used by the save infrastructure to copy files into the project directory.
    virtual QStringList externalFiles() const;

    // --- Serialization ---

    /// Saves the graphic element through a binary data stream. \a options has no
    /// default -- every caller must state whether this builds a genuine .panda file
    /// or an in-session snapshot; see SerializationPurpose.
    virtual void save(QDataStream &stream, SerializationOptions options) const;

    /**
     * \brief Loads the graphic element through a binary data stream.
     * \details Reads the element's state with loadState(), and reports it as one change of everything a file holds
     * (see changed()), also when the read throws.
     * \param stream Binary stream positioned at this element's serialized data.
     * \param context carries portMap, version, contextDir and optional copy-operation state.
     */
    void load(QDataStream &stream, SerializationContext &context);

    // --- Port Management ---

    /**
     * \brief Repositions and reconfigures all ports after the port count changes.
     * \details Lays the ports out with layoutPorts(), and reports it as one change of the ports (see changed()).
     * Called after construction, loading, and any input/output size change.
     */
    void updatePortsProperties();

    /// The attributes of one port that its element sets. An unset field is left as the port has it, so an element that
    /// sets the requirement or the default elsewhere (a wireless node, say) keeps doing so.
    struct PortAttributes {
        /// A name derived each time it is asked for, so a translated name follows the language, given with the same
        /// name in the source language. Built by describedPort().
        struct DerivedName {
            std::function<QString()> name;
            QString source;
        };

        std::optional<QString> name = std::nullopt; ///< An explicit name, its own source.
        std::optional<DerivedName> derivedName = std::nullopt; ///< Takes the place of \c name.
        std::optional<bool> required = std::nullopt;
        std::optional<Status> defaultStatus = std::nullopt;
        std::optional<QPointF> position = std::nullopt; ///< Where the port sits; unset, the port is placed in its column.
    };

    /**
     * \brief Returns the attributes of input port \a index, or nothing when the port keeps its own.
     * \details The base layoutPorts() applies the attributes it returns; the base class returns nothing.
     */
    virtual std::optional<PortAttributes> inputAttributes(int index) const;

    /// As inputAttributes(), for output port \a index.
    virtual std::optional<PortAttributes> outputAttributes(int index) const;

    /// The entry of \a table for port \a index, or nothing past its end: a type whose ports are fixed lists them so.
    static std::optional<PortAttributes> attributesAt(const QList<PortAttributes> &table, int index);

    /// Returns the element's wireless routing mode (ParamId::Wireless); WirelessMode::None for a type without one.
    WirelessMode wirelessMode() const;

    /// Sets the wireless routing mode to \a mode (ParamId::Wireless); a mode out of range is None.
    void setWirelessMode(WirelessMode mode);

    /// Returns \c true if this element offers a wireless routing mode (ParamId::Wireless).
    bool hasWirelessMode() const;

    // --- Element Type & Identity ---

    /// Returns the group this element belongs to.
    ElementGroup elementGroup() const;

    /// Returns the type identifier for this element.
    ElementType elementType() const;

    // --- Port Access ---

    /// Returns the input port at \a index (default 0).
    InputPort *inputPort(const int index = 0) const;

    /// Returns the output port at \a index (default 0).
    OutputPort *outputPort(const int index = 0) const;

    /// Returns a const reference to the vector of all input ports.
    const QVector<InputPort *> &inputs() const;

    /// Returns a const reference to the vector of all output ports.
    const QVector<OutputPort *> &outputs() const;

    /// Returns a combined list of all input and output ports as Port pointers.
    QVector<Port *> allPorts() const;

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

    /// Returns the label child item's bounding rect in scene coordinates -- used to position the
    /// inline rename editor over the visible label. Empty when the label has no text yet (a
    /// caller should fall back to the element's own sceneBoundingRect() in that case).
    QRectF labelSceneBoundingRect() const;

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

    // --- Parameters ---

    /// Returns the element's parameters, one for each of its type's (ElementTypeDescriptor::parameters).
    const Parameters &parameters() const { return m_parameters; }

    /// Returns \c true if the element's type has parameter \a id.
    bool hasParam(const ParamId id) const { return m_parameters.has(id); }

    /// Returns \c true if the element offers parameter \a id for editing: its type has it, and offers it in the
    /// element's state (ParamSpec::offered).
    bool offersParam(ParamId id) const;

    /// Returns parameter \a id as a \a T, its kind; asking for one the type does not have, or as another kind, is a
    /// broken invariant (Parameters).
    template<typename T>
    const T &param(const ParamId id) const
    {
        return m_parameters.get<T>(id);
    }

    /**
     * \brief Sets parameter \a id to \a value as its spec takes it (ParamSpec::accept).
     * \details The element follows the new value (parameterChanged()).
     * \returns \c false, changing nothing, when the type has no such parameter, \a value is of another kind, the spec
     * leaves the parameter as it is, or the value it takes is the one the parameter has.
     */
    bool setParam(ParamId id, const ParamValue &value);

    // --- Audio Properties ---

    /// Returns the sound file the element plays (ParamId::AudioFile), or an empty path for a type without one.
    QString audio() const;

    /// Sets the sound file the element plays to \a audio (ParamId::AudioFile), as its spec takes it.
    void setAudio(const QString &audio);

    /// Returns \c true if the element offers a sound file for editing (ParamId::AudioFile).
    bool hasAudioBox() const;

    /// Returns \c true if the element offers its volume for editing (ParamId::Volume).
    bool hasVolume() const;

    /// Returns the audio playback volume (ParamId::Volume, 0.0–1.0); 0 for a type without one.
    float volume() const;

    /// Sets the audio playback volume to \a vol (ParamId::Volume, 0.0–1.0).
    void setVolume(float vol);

    // --- Color Properties ---

    /// Returns the name of the element's colour (ParamId::Colour), or an empty string for a type without one.
    QString color() const;

    /// Sets the element's colour to \a color, a name; a type without a colour takes nothing.
    void setColor(const QString &color);

    /// Returns \c true if the element offers its colour for editing (an LED with one input, a display).
    bool hasColors() const;

    /// Maps a color name ("White","Red","Green","Blue","Purple") to indices 0–4.
    static int colorNameToIndex(const QString &color);

    /// Returns the name of the next color in the element's color list.
    QString nextColor() const;

    /// Returns the name of the previous color in the element's color list.
    QString previousColor() const;

    // --- Frequency & Delay ---

    /// Returns the frequency in Hz (ParamId::Frequency); 0 for a type without one.
    double frequency() const;

    /// Returns the phase delay as a fraction of the period (ParamId::Delay); 0 for a type without one.
    double delay() const;

    /// Returns \c true if the element offers its frequency for editing (ParamId::Frequency).
    bool hasFrequency() const;

    /// Returns \c true if the element offers its phase delay for editing (ParamId::Delay).
    bool hasDelay() const;

    /// Sets the frequency to \a freq Hz (ParamId::Frequency), as its spec takes it.
    void setFrequency(const double freq);

    /// Returns whether setFrequency() takes \a freq (the spec's rule); one it does not take leaves the frequency as
    /// it was, and a type without a frequency takes none.
    bool acceptsFrequency(const double freq) const;

    /// Sets the phase delay to \a delay (ParamId::Delay), as its spec takes it.
    void setDelay(const double delay);

    /**
     * \brief Returns the list of editable properties this element exposes in the ElementEditor.
     * \details Derived from the descriptor's flags and the parameters the element offers; no override needed in
     * subclasses.
     * \return Ordered list of PropertyDescriptors for all supported property types.
     */
    QList<PropertyDescriptor> editableProperties() const;

    // --- Appearance Management ---

    /// Returns \c true if the user is allowed to choose a custom appearance for this element.
    bool canChangeAppearance() const;

    /**
     * \brief Switches the element's appearance with applyAppearance(), and reports it as one change of a parameter.
     * \param defaultAppearance \c true to restore the built-in default appearance.
     * \param fileName    File path of the custom appearance image (used when \a defaultAppearance is \c false).
     */
    void setAppearance(const bool defaultAppearance, const QString &fileName);

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
    virtual QList<std::pair<int, QString>> appearanceStates() const;

    /// Loads and applies the appearance at position \a index in the appearance list (see ElementAppearance::setPixmap()).
    void setPixmap(const int index);

    /// Throws a Pandaception naming an external file the element uses in a fallback's place: the picture shown last
    /// that did not load (see ElementAppearance::reportFailedPicture()), and in an override the element's own files.
    virtual void reportFailedFiles() const;

    // --- Truth Table ---

    /// Returns \c true if the element offers its truth table for editing (ParamId::Table).
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

    /// Reports that a rotation/flip transform change changed the picture (a rotatable element shows its picture's
    /// labels counter-oriented), and keeps the name label counter-oriented to match. Public because
    /// ElementOrientation (a sibling collaborator, not a subclass) calls it back after
    /// updating the item transform.
    void pictureOrientationChanged() { pictureChanged(); updateLabelOrientation(); }

    /// Re-applies the orientation, and the name label's counter-orientation, about the pixmap centre after the centre
    /// moved. Public because ElementAppearance (a sibling collaborator) calls it after each pixmap change.
    void recentreOrientation()
    {
        if (m_orientation.recentre()) {
            updateLabelOrientation();
        }
    }

    // --- Flip / Mirror ---

    /// Returns \c true if this element is mirrored along the X axis (horizontal flip).
    bool isFlippedX() const { return m_orientation.isFlippedX(); }

    /// Returns \c true if this element is mirrored along the Y axis (vertical flip).
    bool isFlippedY() const { return m_orientation.isFlippedY(); }

    /// Sets the horizontal mirror state and updates the item transform.
    void setFlippedX(bool flipped);

    /// Sets the vertical mirror state and updates the item transform.
    void setFlippedY(bool flipped);

    // --- Geometric Properties ---

    /**
     * \brief Returns \a pos rounded to the nearest multiple of \a step, by default half the scene grid.
     * \details Setting a position is exact, so a load, an undo and a redo put an element where it was; every edit
     * that decides where an element goes (a drag, a nudge, a drop, a paste, a rotate, a flip...) snaps through this.
     */
    static QPointF snapToGrid(const QPointF &pos, int step = Constants::gridSize / 2);

    /// Returns the centre the element turns about in local coordinates, as its body was last measured (BodyShape).
    QPointF pixmapCenter() const { return m_body.centre; }

    /// Returns the shape of the element's body as it was last measured: as each change is reported and as its
    /// picture changes.
    const BodyShape &body() const { return m_body; }

    /**
     * \brief Footprint of a "procedural render body" (IC/Mux/Demux/TruthTable): the nominal
     * 64x64 body unioned with wherever the ports actually land, which extends past it once an
     * instance has enough ports.
     * \details Single source of truth for the body their presenter measures (BodyShape), the sizing
     * pixmap, and the label's anchor on those four element types, so they can never drift out of
     * sync with each other.
     */
    QRectF renderBodyBounds() const;

    /// Returns the element's appearance: its picture and the appearances it can show.
    const ElementAppearance &appearance() const { return m_appearance; }

    /**
     * \brief Returns the bounding rectangle of this element in local coordinates, as Qt was last given it.
     * \details In a scene, these are the bounds Qt indexes and repaints, which may change only after Qt is told
     * (prepareGeometryChange()): they are measured as the element enters the scene and brought up to date by each
     * change (changed(), pictureChanged()). Out of a scene, they are measured whenever asked.
     */
    QRectF boundingRect() const final;

    /// Returns the bounding rectangle of this element in local coordinates, as its state gives it now: its body, as
    /// it was last measured (BodyShape), and its ports.
    QRectF measureBoundingRect() const;

    // --- Changes ---

    /**
     * \brief Reports that \a what changed in the element, once it holds the change.
     * \details Every change of the element's saved state ends here: its placement, a parameter, its label, its ports.
     * The bounds Qt has are brought up to the measured ones, telling Qt first, and a change of them is reported as
     * Body; the element is repainted unless only its placement changed, so a move keeps the item's device cache; the
     * scene's listener hears of it (ElementChangeListener::reportElementChange()). Under a ChangeBatch, the changes are held
     * back and reported as one when the batch ends.
     */
    void changed(ElementChanges what);

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

    /**
     * \brief Appends this element's full simulation state to \a out.
     * \details The exact counterpart of resetSimState(): whatever a subclass clears there, it
     * must save here and put back in restoreSimState(). The base handles the output values;
     * sequential elements add their edge-detection history, and IC recurses into its internals.
     * Used to make a BeWavedDolphin sweep state-neutral for the live circuit -- the sweep resets
     * everything so its own results are reproducible, and must hand the user's circuit back
     * exactly as it found it.
     */
    virtual void saveSimState(QVector<Status> &out) const;

    /// Restores what saveSimState() wrote, reading from \a in at \a cursor and advancing it.
    /// The read order must mirror the write order exactly.
    virtual void restoreSimState(const QVector<Status> &in, int &cursor);

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
    void connectPredecessor(const int inputIndex, Element *source, const int outputPort);

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

    /// Allocates simulation I/O vectors with \a inputCount inputs and \a outputCount outputs, keeping the current
    /// outputs if \a keepOutputs and their count is unchanged (a rebuild), else starting them from the ports' defaults.
    void initSimulationVectors(const int inputCount, const int outputCount, const bool keepOutputs = false);

    /// Polymorphic interface for drag-drop initialization (replaces elementType() == IC checks).
    virtual void loadFromDrop(const QString &fileName, const QString &contextDir);

    // --- Virtual Methods ---

    /// Returns a string encoding element-specific properties for serialization or display.
    virtual QString genericProperties();

    /// Forces a visual refresh of the element's pixmap and ports.
    virtual void refresh();

    // --- Qt Graphics & Display ---

    /**
     * \brief Paints the element onto the scene, through the installed BodyPainter: its selection, then its body.
     * \param painter Painter provided by the graphics view framework.
     * \param option  Style options (unused).
     * \param widget  Target widget (unused).
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) final;

    /// Draws with \a painter, in the element's own coordinates, what the element draws of its own: over its picture
    /// (a display's lit segments, a rotary's marks), or the whole body of a type that draws one (Mux, Demux,
    /// TruthTable, IC). Nothing by default; the presenter draws the picture.
    virtual void drawBody(QPainter &painter) const;

    /// Returns the type's name in the current language ("And", "D-Flip-Flop", ...), looked up each time it is asked,
    /// so it follows a language change.
    QString translatedName() const;

    /// Returns the text shown when the pointer rests on the element: its translatedName(). Scene::helpEvent() asks for
    /// it then, so no element carries a tooltip of its own. Empty for an element that shows something else.
    virtual QString helpText() const;

    /**
     * \brief Measures again what the element draws in the interface language, after a language change.
     * \details Scene::retranslateUi() calls it on each of its elements. Names and help text need nothing, being looked
     * up when asked; an element that draws translated text of its own, which can change its bounds, overrides it.
     */
    virtual void languageChanged();

    // --- Setters (Port & Logic Configuration) ---

    /// Adjusts the number of input ports to \a size, adding or removing ports as needed.
    virtual void setInputSize(const int size);

    /// Adjusts the number of output ports to \a size, adding or removing ports as needed.
    virtual void setOutputSize(const int size);

    /// Replaces the input port vector with \a inputs.
    void setInputs(const QVector<InputPort *> &inputs);

    /// Read-only view of the cached simulation input values.
    const QVector<Status> &simInputs() const { return m_sim.inputs(); }

    /// Read-only view of the current simulation output values.
    const QVector<Status> &simOutputs() const { return m_sim.outputs(); }

    // --- Theme ---

    /// Updates the element's visual theme according to the current dark/light palette.
    virtual void updateTheme();

protected:
    // --- Graphics & Rendering ---

    /// Returns the procedural render pixmap of a type that draws its own body (see ElementAppearance::setRenderPixmap());
    /// null for an element that shows a picture.
    QPixmap renderPixmap() const;

    /**
     * \brief Returns the bounding rectangle that encompasses all child ports.
     * \details Iterates over childItems(), mapping each Port's bounding rect
     * into the element's local coordinate space.
     * \return Combined bounding QRectF of all port children.
     */
    QRectF portsBoundingRect() const;

    /// Sets the label's anchor point in the element's un-rotated local frame, and repositions
    /// it there. \a pos is remembered (m_labelAnchor) so updateLabelOrientation() can always
    /// recompute the label's actual on-screen position from this original value, rather than
    /// from whatever rotation-adjusted position it last computed.
    void setLabelAnchor(const QPointF &pos);

    /// Hook called whenever the label text is set (including on construction and on inline-edit
    /// commit). Base is a no-op; Text overrides it to toggle its empty-state hint.
    virtual void labelContentChanged() {}

    // --- Changes ---

    /**
     * \class ChangeBatch
     * \brief Holds back what the element reports while it lasts: the changes made under it, a nested batch's
     * included, are reported as one when the outermost batch ends, even if it ends by an exception.
     */
    class ChangeBatch
    {
    public:
        /// Starts a batch on \a element that reports at least \a what as it ends.
        explicit ChangeBatch(Element &element, ElementChanges what = {});

        /// Ends the batch, reporting what changed under it if it is the outermost.
        ~ChangeBatch();

    private:
        Q_DISABLE_COPY_MOVE(ChangeBatch)

        Element &m_element;
    };

    /// Reports that the element's picture changed (a new pixmap, a display's segments, the theme's colours): it is
    /// repainted, and a change of its bounds is reported as Body. The picture alone is not a change of the circuit,
    /// so nothing else is reported. Under a ChangeBatch, held back as changed() is.
    void pictureChanged();

    /// Lays the ports out for the current port count. The base distributes the inputs down the left edge (x=0) and the
    /// outputs down the right edge (x=64), spaced by the scene grid size, unless a port's attributes place it; applies
    /// each port's inputAttributes() or outputAttributes(); and orients the ports of a non-rotatable element. An element
    /// whose body follows its ports re-sizes the body here. Called only by updatePortsProperties().
    virtual void layoutPorts();

    /**
     * \brief Reads the element's state from \a stream: the base reads what every element saves (see
     * ElementSerializer), and an element with more reads it after the base. Called only by load().
     * \param stream Binary stream positioned at this element's serialized data.
     * \param context carries portMap, version, contextDir and optional copy-operation state.
     */
    virtual void loadState(QDataStream &stream, SerializationContext &context);

    /// Switches the element's appearance: the built-in default when \a defaultAppearance, else \a fileName's picture in
    /// the slot the element's state shows. The base sets every slot. Called only by setAppearance().
    virtual void applyAppearance(bool defaultAppearance, const QString &fileName);

    // --- Parameters ---

    /// Follows a new value of parameter \a id, which had \a previous: what the element derives from it (a node's
    /// ports). Called by setParam(); nothing by default.
    virtual void parameterChanged(ParamId id, const ParamValue &previous);

    /// Stores \a value, of its kind, as parameter \a id as it is: no rule, no follow-up. For a value the element keeps
    /// in step with its simulation state (a rotary's selection restored with its outputs), which is no change of the
    /// circuit.
    void storeParam(const ParamId id, ParamValue value) { m_parameters.set(id, std::move(value)); }

    // --- Qt Event Handling ---

    /**
     * \brief Handles item state changes such as position, rotation, and selection.
     * \details These change types are intercepted (a position is set exactly; see snapToGrid()):
     * - ItemSceneHasChanged: finds the new scene's listener, and measures the bounds Qt is given there (see
     *   boundingRect()).
     * - ItemPositionHasChanged / ItemRotationHasChanged / ItemTransformHasChanged:
     *   calls updateConnections() on all ports to redraw attached wires, and reports a change of the placement.
     * - ItemSelectedHasChanged: toggles connection highlight via highlight().
     * \param change The type of change that occurred.
     * \param value  The new value associated with the change.
     * \return The (possibly modified) value to use, or the base class result.
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /**
     * \brief Requests inline label editing on double-click.
     * \details Only fires for hasLabel() elements; other types fall through to the base
     * Qt behavior unchanged. IC and TruthTable override this method themselves (to open a
     * sub-circuit / the truth-table editor instead), so this base implementation is never
     * reached for them -- ordinary virtual dispatch, no elementType() check needed here.
     */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * \brief Intercepts mouse-press and mouse-release events to handle Ctrl+click.
     * \details When Ctrl is held during a mouse press or release, the event is
     * consumed here and not forwarded, preventing accidental multi-selection.
     * \param event The scene event to inspect.
     * \return true if the event was consumed; otherwise the base class result.
     */
    bool sceneEvent(QEvent *event) override;

    // --- Port Size Constraint Setters ---

    /// Sets the maximum number of input ports to \a maxInputSize.
    void setMaxInputSize(const int maxInputSize);

    /// Sets the minimum number of input ports to \a minInputSize.
    void setMinInputSize(const int minInputSize);

    /// Sets the maximum number of output ports to \a maxOutputSize.
    void setMaxOutputSize(const int maxOutputSize);

    /// Sets the minimum number of output ports to \a minOutputSize.
    void setMinOutputSize(const int minOutputSize);

    // --- Port Names ---

    /**
     * \brief Returns a port's attributes named "<letter> (<description>)", as a segment display names its inputs.
     * \details The name is derived: each time it is asked for, it translates \a description in this element's class
     * context; its source keeps \a description as written. Mark \a description with QT_TR_NOOP() in the element's own
     * code, so it is extracted in that context. Both must outlive the element, as string literals do.
     */
    PortAttributes describedPort(const char *letter, const char *description) const;

    /// Owns the input/output port vectors and their creation/resize lifecycle; this element
    /// forwards its port-access interface here.  See ElementPorts.
    ElementPorts m_ports{this};

    /// Owns the appearance lists and which picture the element shows; this element forwards its
    /// appearance interface here.  See ElementAppearance.
    ElementAppearance m_appearance{this};

    QGraphicsSimpleTextItem *m_label = new QGraphicsSimpleTextItem(this); ///< Child text item that displays the label and optional trigger shortcut.
    QPointF m_labelAnchor; ///< The label's intended anchor point in the element's un-rotated local frame; see setLabelAnchor().

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
    bool simUpdateInputs() { return m_sim.updateInputs(false, m_ports.inputs()); }

    /**
     * \brief Like simUpdateInputs(), but allows Unknown/Error values through.
     * \details Combinational gates use this so their domination rules
     * (e.g. AND(0, X) = 0) can short-circuit even with unknown inputs.
     * Only a truly unconnected input (null predecessor whose port default
     * is Unknown) triggers an early all-outputs-Unknown return.
     */
    bool simUpdateInputsAllowUnknown() { return m_sim.updateInputs(true, m_ports.inputs()); }

    /**
     * \brief Decodes \a count select-line statuses from simInputs() into a binary index.
     * \param offset  Index of the first select-line input in simInputs().
     * \param count   Number of select lines to read.
     * \return An integer where bit i is 1 if simInputs()[offset+i] == Active.
     * \details Used by Mux and Demux to convert select-line signals into a data-port index.
     */
    int decodeSelectValue(int offset, int count) const { return m_sim.decodeSelectValue(offset, count); }

private:
    Q_DISABLE_COPY_MOVE(Element)

    /// Reaches ports/orientation/appearance/context-directory state to implement save()/loadState();
    /// see ElementSerializer.
    friend class ElementSerializer;

    /// Reports each new picture through pictureReplaced(), under one ChangeBatch; see
    /// ElementAppearance::setPixmap()/setRenderPixmap().
    friend class ElementAppearance;

    /// Calls the protected sceneEvent() directly to test its Ctrl+click swallow deterministically:
    /// a real QTest::mousePress(..., Qt::ControlModifier, ...) depends on the platform's actual
    /// keyboard-modifier state reaching the synthesized QGraphicsSceneMouseEvent, which this
    /// sandbox's offscreen/X11 setup does not reproduce reliably.
    friend class TestElement;

    /// Reads the displayed picture for the characterization goldens, so no production accessor
    /// exists only for tests; see Tests/Common/Goldens.h.
    friend class SnapshotAccess;

    /// Drops and re-enables the item's DeviceCoordinateCache so the next paint re-renders the
    /// whole item. DeviceCoordinateCache invalidates incrementally and keeps the previously
    /// cached device tile when the displayed pixmap swaps to a different size (e.g. a small
    /// built-in SVG replaced by a large custom raster), which would leave a stale image of the
    /// old appearance behind the new one. Call only on an actual size change.
    void invalidateRenderCache();

    // --- Changes ---

    /// Measures the body anew (BodyMeasure); called as each change is reported.
    void measureBody();

    /// Takes a new picture: measures the body anew, drops the render cache if the body's size changed, turns the
    /// element about the body's new centre and reports the picture changed.
    void pictureReplaced();

    /// Brings the bounds Qt has up to the measured ones, telling Qt first (prepareGeometryChange()); \returns
    /// \c true if they changed. Nothing to do while the element is in no scene, where they are measured when asked.
    bool syncBoundingRect();

    /// Reports what changed since the last report (see changed()): called as the outermost ChangeBatch ends, or
    /// at once outside one.
    void flushChanges();

    // --- Port Management Helpers ---

    /// Shared implementation for setInputSize() and setOutputSize(): resizes the port store
    /// (within min/max constraints) then re-lays-out the ports via updatePortsProperties(), as one change of the ports.
    void setPortSize(const int size, const bool isInput);

    // --- Display & Interaction ---

    /**
     * \brief Highlights or un-highlights all connections attached to this element.
     * \details Iterates over every port and every connection, toggling the highLight
     * flag to match the element's new selection state.
     * \param isSelected true when the element has just been selected.
     */
    void highlight(const bool isSelected);

    /// Counter-orients the name label about its own centre for the current rotation + flip
    /// state, so the text reads upright and unmirrored at any element orientation. Recomputed
    /// whenever the orientation or the label text (which moves the pivot) changes.
    void updateLabelOrientation();

    // --- Members: Element Type & Identity ---

    ElementGroup m_elementGroup = ElementGroup::Unknown;
    ElementType m_elementType = ElementType::Unknown;

    /// Cached reference to this type's descriptor (registry entry is a long-lived
    /// singleton). Backs the hasTrigger()/rotatesGraphic()/... facades, replacing a
    /// per-call registry map lookup.
    const ElementTypeDescriptor &m_descriptor;

    /// The element's parameters, one for each of its descriptor's (ElementTypeDescriptor::parameters).
    Parameters m_parameters;

    // --- Members: Direct Simulation ---

    /// Owns the simulation runtime state (I/O values + connection graph); this element
    /// forwards its direct-simulation interface here.  See ElementSimState.
    ElementSimState m_sim;

    // --- Members: Trigger & Label ---

    QKeySequence m_trigger;
    QString m_labelText;

    // --- Members: Display State ---

    bool m_selected = false;

    /// Owns the rotation angle and flip flags, and the transform math that applies them to
    /// the item and its ports; this element forwards its orientation interface here.
    /// See ElementOrientation.
    ElementOrientation m_orientation{this};

    // --- Members: Changes ---

    ElementChangeListener *m_listener = nullptr; ///< The listener of the scene the element is in, found as it enters one; null in none.
    BodyShape m_body;                            ///< The body's shape as it was last measured; see measureBody().
    QRectF m_bounds;                             ///< The bounds Qt was last given; meaningful while m_boundsLatched.
    bool m_boundsLatched = false;                ///< Whether boundingRect() returns m_bounds: while the element is in a scene.
    int m_changeBatchDepth = 0;                  ///< How many ChangeBatch scopes are open on the element.
    ElementChanges m_pendingChanges;             ///< What changed since the last report.
    bool m_pictureChanged = false;               ///< Whether the picture changed since the last report (pictureChanged()).

    // --- Members: Port Size Constraints ---

    quint64 m_minInputSize = 0;
    quint64 m_maxInputSize = 0;
    quint64 m_minOutputSize = 0;
    quint64 m_maxOutputSize = 0;
};
