// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Abstract base class for all graphical circuit elements.
 */

#pragma once

#include <QBitArray>
#include <QGraphicsItem>
#include <QKeySequence>
#include <QList>
#include <QVersionNumber>

#include "App/Core/Enums.h"
#include "App/Core/ItemWithId.h"
#include "App/Element/PropertyDescriptor.h"

struct SerializationContext;

class GraphicElement;
class QNEInputPort;
class QNEOutputPort;
class QNEPort;
class QPainter;
class QStyleOptionGraphicsItem;
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
 * - Pixmap / skin rendering with default and user-defined skins.
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

    /// Returns the name of the next audio file in the element's audio list.
    QString nextAudio() const;

    /// Returns the name of the previous audio file in the element's audio list.
    QString previousAudio() const;

    // --- Color Properties ---

    /// Returns the name of the color currently applied to this element.
    virtual QString color() const;

    /// Sets the element color to \a color and refreshes the pixmap.
    virtual void setColor(const QString &color);

    /// Returns \c true if this element type supports color selection.
    bool hasColors() const;

    /// Returns the name of the next color in the element's color list.
    QString nextColor() const;

    /// Returns the name of the previous color in the element's color list.
    QString previousColor() const;

    // --- Frequency & Delay ---

    /// Returns the clock frequency in Hz (overridden by Clock; returns 0 for other elements).
    virtual float frequency() const;

    /// Returns the clock phase delay in seconds (overridden by Clock; returns 0 for other elements).
    virtual float delay() const;

    /// Returns \c true if this element type exposes a configurable clock frequency.
    bool hasFrequency() const;

    /// Returns \c true if this element type exposes a configurable clock phase delay.
    bool hasDelay() const;

    /// Sets the clock frequency to \a freq (overridden by clock elements).
    virtual void setFrequency(const float freq);

    /// Sets the clock phase delay to \a delay (overridden by clock elements).
    virtual void setDelay(const float delay);

    /**
     * \brief Returns the list of editable properties this element exposes in the ElementEditor.
     * \details Derived from metadata flags; no override needed in subclasses.
     * \return Ordered list of PropertyDescriptors for all supported property types.
     */
    QList<PropertyDescriptor> editableProperties() const;

    // --- Skin Management ---

    /// Returns \c true if the user is allowed to choose a custom skin for this element.
    bool canChangeSkin() const;

    /**
     * \brief Switches the element's skin.
     * \param defaultSkin \c true to restore the built-in default skin.
     * \param fileName    File path of the custom skin image (used when \a defaultSkin is \c false).
     */
    virtual void setSkin(const bool defaultSkin, const QString &fileName);

    /// Loads and applies the pixmap located at \a pixmapPath.
    void setPixmap(const QString &pixmapPath);

    /// Loads and applies the skin at position \a index in the skin list.
    void setPixmap(const int index);

    // --- Truth Table ---

    /// Returns \c true if this element type has an editable truth table.
    bool hasTruthTable() const;

    // --- Rotation ---

    /// Returns the current rotation angle of this element in degrees.
    qreal rotation() const;

    /// Rotates the element to \a angle degrees and updates port positions.
    void setRotation(const qreal angle);

    /// Returns \c true if the user is allowed to rotate this element.
    bool isRotatable() const;

    /// Rotates all ports by \a angle degrees around the element centre.
    void rotatePorts(const qreal angle);

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

    /// Returns the four-state signal value on simulation output port \a index.
    Status outputValue(const int index = 0) const;

    /// Returns the simulation input value on port \a index.
    bool inputValue(const int index = 0) const;

    /// Returns the number of simulation output slots.
    int simOutputSize() const;

    /// Sets simulation output port \a index to \a value.
    void setOutputValue(const int index, const Status value);

    /// Sets simulation output port 0 to \a value.
    void setOutputValue(const Status value);

    /// Convenience overload — converts \c bool to Active/Inactive for port \a index.
    void setOutputValue(const int index, const bool value) { setOutputValue(index, value ? Status::Active : Status::Inactive); }

    /// Convenience overload — converts \c bool to Active/Inactive for port 0.
    void setOutputValue(const bool value) { setOutputValue(value ? Status::Active : Status::Inactive); }

    /// Connects simulation input \a inputIndex to output \a outputPort of \a source element.
    void connectPredecessor(const int inputIndex, GraphicElement *source, const int outputPort);

    /// Returns \c true if any simulation output changed since the last query (and resets the flag).
    bool outputChanged() const { return m_simOutputChanged; }

    /// Clears the simulation output-changed flag.
    void clearOutputChanged() { m_simOutputChanged = false; }

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

    /// Path to all default skins. The default skin is in a resource file.
    QStringList m_defaultSkins;

    /// Path to all custom skins. Custom skin names are system file paths defined by the user.
    QStringList m_alternativeSkins;

    /// All input ports owned by this element (ordered by index).
    QVector<QNEInputPort *> m_inputPorts;

    /// All output ports owned by this element (ordered by index).
    QVector<QNEOutputPort *> m_outputPorts;

    /// Current pixmap displayed for this GraphicElement.
    QPixmap m_pixmap;

    QColor m_selectionBrush; ///< Fill color used to draw the selection highlight rectangle.
    QColor m_selectionPen;   ///< Border color used to draw the selection highlight rectangle.
    QGraphicsTextItem *m_label = new QGraphicsTextItem(this); ///< Child text item that displays the label and optional trigger shortcut.

    // --- Members: Metadata ---

    QString m_pixmapPath;     ///< Resource or file path of the element's default pixmap (from metadata).
    QString m_titleText;      ///< Translated title text shown in UI panels (from metadata).
    QString m_translatedName; ///< Translated element name used as tooltip and port object name.
    bool m_usingDefaultSkin = true; ///< True when the active skin matches the built-in default skins.

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
    bool simUpdateInputs();

    /**
     * \brief Like simUpdateInputs(), but allows Unknown/Error values through.
     * \details Combinational gates use this so their domination rules
     * (e.g. AND(0, X) = 0) can short-circuit even with unknown inputs.
     * Only a truly unconnected input (null predecessor whose port default
     * is Unknown) triggers an early all-outputs-Unknown return.
     */
    bool simUpdateInputsAllowUnknown();

    /// Read-only view of the cached simulation input values.
    const QVector<Status> &simInputs() const { return m_simInputValues; }

    /// Read-only view of the current simulation output values.
    const QVector<Status> &simOutputs() const { return m_simOutputValues; }

private:
    // --- Port Management Helpers ---

    /// Appends a new input port with optional \a name to the input port vector.
    void addInputPort(const QString &name = {});

    /// Appends a new output port with optional \a name to the output port vector.
    void addOutputPort(const QString &name = {});

    /// Appends a new port with optional \a name; \a isOutput selects the port direction.
    void addPort(const QString &name, const bool isOutput);

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
    /// Reads and applies a single skin path at index \a skin from \a stream.
    void loadPixmapSkinName(QDataStream &stream, const int skin, const QString &contextDir);
    /// Reads and applies all skin paths from \a stream.
    void loadPixmapSkinNames(QDataStream &stream, SerializationContext &context);
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

    /**
     * \brief Describes a single simulation-graph edge: which element and output
     * port feeds into one of this element's input slots.
     * \details Used by simUpdateInputs() to traverse the simulation graph.
     */
    struct SimInputConnection {
        GraphicElement *sourceElement = nullptr;
        int sourceOutputIndex = 0;
    };

    QVector<SimInputConnection> m_simInputConnections;
    QVector<Status> m_simInputValues;
    QVector<Status> m_simOutputValues;
    bool m_simOutputChanged = false;

    // --- Members: Trigger & Label ---

    QKeySequence m_trigger;
    QString m_labelText;

    QString m_currentPixmapPath;

    // --- Members: Capabilities & Display State ---

    bool m_hasColors = false;
    bool m_selected = false;
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

Q_DECLARE_METATYPE(GraphicElement)

/// Stream insertion operator that serializes \a item to \a stream via GraphicElement::save().
QDataStream &operator<<(QDataStream &stream, const GraphicElement *item);

// FIXME: connecting more than one source makes element stop working

