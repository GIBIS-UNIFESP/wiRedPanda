// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include <QBitArray>
#include <QGraphicsItem>
#include <QKeySequence>
#include <QVersionNumber>

#include "App/Core/Enums.h"
#include "App/Core/ItemWithId.h"
#include "App/Element/LogicElements/LogicElement.h"

class GraphicElement;
class QNEInputPort;
class QNEOutputPort;
class QNEPort;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;

/**
 * @brief Virtual class to implement graphical element appearance, input and output ports, and tooltips.
 *
 * The appearance includes editable features such as pose, colors, skins, shortcuts, and labels.
 * It also implements the functions to handle loading and saving the element into files.
 */
class GraphicElement : public QGraphicsObject, public ItemWithId
{
    Q_OBJECT
    Q_PROPERTY(QString pixmapPath MEMBER m_pixmapPath CONSTANT)
    Q_PROPERTY(QString titleText MEMBER m_titleText CONSTANT)
    Q_PROPERTY(QString translatedName MEMBER m_translatedName CONSTANT)

public:
    // --- Type Info ---

    enum { Type = QGraphicsItem::UserType + 3 };
    int type() const override { return Type; }

    // --- Lifecycle ---

    explicit GraphicElement(ElementType type, ElementGroup group, const QString &pixmapPath, const QString &titleText, const QString &translatedName,
                            const int minInputSize, const int maxInputSize, const int minOutputSize, const int maxOutputSize, QGraphicsItem *parent = nullptr);
    explicit GraphicElement(QGraphicsItem *parent = nullptr) : QGraphicsObject(parent) {}
    GraphicElement(const GraphicElement &other) : GraphicElement(other.parentItem()) {}

    // --- Serialization ---

    //! Saves the graphic element through a binary data stream.
    virtual void save(QDataStream &stream) const;

    /**
     * @brief Loads the graphic element through a binary data stream.
     * @param portMap receives a reference to each input and output port.
     */
    virtual void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version);

    // --- Port Management ---

    //! Updates the number and the connected elements to the ports whenever needed (e.g. loading the element, changing the number of inputs/outputs).
    virtual void updatePortsProperties();

    // --- Element Type & Identity ---

    ElementGroup elementGroup() const;
    ElementType elementType() const;
    LogicElement *logic() const;
    int priority() const;

    // --- Port Access ---

    QNEInputPort *inputPort(const int index = 0);
    QNEOutputPort *outputPort(const int index = 0);
    const QVector<QNEInputPort *> &inputs() const;
    const QVector<QNEOutputPort *> &outputs() const;
    int inputSize() const;
    int outputSize() const;

    // --- Port Size Constraints ---

    int maxInputSize() const;
    int minInputSize() const;
    int maxOutputSize() const;
    int minOutputSize() const;

    // --- Labeling ---

    QString label() const;
    void setLabel(const QString &label);
    void updateLabel();
    bool hasLabel() const;

    // --- Trigger Control ---

    QKeySequence trigger() const;
    void setTrigger(const QKeySequence &trigger);
    bool hasTrigger() const;

    // --- Audio Properties ---

    virtual QString audio() const;
    virtual void setAudio(const QString &audio);
    bool hasAudio() const;
    bool hasAudioBox() const;
    QString nextAudio() const;
    QString previousAudio() const;

    // --- Color Properties ---

    virtual QString color() const;
    virtual void setColor(const QString &color);
    bool hasColors() const;
    QString nextColor() const;
    QString previousColor() const;

    // --- Frequency & Delay ---

    //! virtual function overloaded by clock element. Other elements have frequency of 0.
    virtual float frequency() const;

    //! virtual function overloaded by clock element. Other elements have frequency of 0.
    virtual float delay() const;
    bool hasFrequency() const;
    bool hasDelay() const;
    virtual void setFrequency(const float freq);
    virtual void setDelay(const float delay);

    // --- Skin Management ---

    bool canChangeSkin() const;
    virtual void setSkin(const bool defaultSkin, const QString &fileName);
    void setPixmap(const QString &pixmapPath);
    void setPixmap(const int index);

    // --- Truth Table ---

    bool hasTruthTable() const;

    // --- Rotation ---

    qreal rotation() const;
    void setRotation(const qreal angle);
    bool isRotatable() const;
    void rotatePorts(const qreal angle);

    // --- Geometric Properties ---

    QPointF pixmapCenter() const;
    QRectF boundingRect() const override;

    // --- State Queries ---

    bool isValid();

    // --- Polymorphic Logic Element Interface ---

    //! Polymorphic interface for logic element access (eliminates manual IC type checking)
    virtual LogicElement *getInputLogic(int portIndex) const;
    virtual LogicElement *getOutputLogic(int portIndex) const;

    // --- Polymorphic Port Index Interface ---

    //! Polymorphic interface for getting the correct input/output indices for ports
    virtual int getInputIndexForPort(int portIndex) const;
    virtual int getOutputIndexForPort(int portIndex) const;

    // --- Polymorphic Element Mapping Interface ---

    //! Polymorphic interface for element mapping (replaces manual generateMap() calls)
    virtual QVector<std::shared_ptr<LogicElement>> getLogicElementsForMapping();

    // --- Polymorphic Port Naming Interface ---

    //! Polymorphic interface for port naming (replaces elementType() == IC checks)
    virtual bool canSetPortNames() const;
    virtual void setInputPortName(int port, const QString &name);
    virtual void setOutputPortName(int port, const QString &name);

    // --- Virtual Methods ---

    virtual QString genericProperties();
    virtual void refresh();

    // --- Qt Graphics & Display ---

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void retranslate();

    // --- Setters (Port & Logic Configuration) ---

    void setInputSize(const int size);
    void setOutputSize(const int size);
    void setInputs(const QVector<QNEInputPort *> &inputs);
    void setOutputs(const QVector<QNEOutputPort *> &outputs);
    void setLogic(LogicElement *newLogic);
    void setPortName(const QString &name);
    void setPriority(const int value);

    // --- Theme ---

    //! Updates the GraphicElement theme according to the dark/light wiRedPanda theme.
    virtual void updateTheme();

protected:
    // --- Graphics & Rendering ---

    QPixmap pixmap() const;
    QRectF portsBoundingRect() const;

    // --- Qt Event Handling ---

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    bool sceneEvent(QEvent *event) override;

    // --- Capability Setters ---

    void setCanChangeSkin(const bool canChangeSkin);
    void setHasAudio(const bool hasAudio);
    void setHasAudioBox(const bool hasAudioBox);
    void setHasColors(const bool hasColors);
    void setHasDelay(const bool hasDelay);
    void setHasFrequency(const bool hasFrequency);
    void setHasLabel(const bool hasLabel);
    void setHasTrigger(const bool hasTrigger);
    void setHasTruthTable(const bool hasTruthTable);
    void setRotatable(const bool rotatable);

    // --- Port Size Constraint Setters ---

    void setMaxInputSize(const int maxInputSize);
    void setMinInputSize(const int minInputSize);
    void setMaxOutputSize(const int maxOutputSize);
    void setMinOutputSize(const int minOutputSize);

    //! Path to all default skins. The default skin is in a resource file.
    QStringList m_defaultSkins;

    //! Path to all custom skins. Custom skin names are system file paths defined by the user.
    QStringList m_alternativeSkins;

    //! input port vector
    QVector<QNEInputPort *> m_inputPorts;

    //! output port vector
    QVector<QNEOutputPort *> m_outputPorts;

    //! Current pixmap displayed for this GraphicElement.
    QPixmap m_pixmap;

    QColor m_selectionBrush;
    QColor m_selectionPen;
    QGraphicsTextItem *m_label = new QGraphicsTextItem(this);

    // --- Members: Metadata ---

    QString m_pixmapPath;
    QString m_titleText;
    QString m_translatedName;
    bool m_usingDefaultSkin = true;

private:
    // --- Port Management Helpers ---

    //! adds an input port at the end of the input port vector.
    void addInputPort(const QString &name = {});

    //! adds an output port at the end of the output port vector.
    void addOutputPort(const QString &name = {});

    //! adds an input or output port at the end of the port vector.
    void addPort(const QString &name, const bool isOutput);

    static void removePortFromMap(QNEPort *deletedPort, QMap<quint64, QNEPort *> &portMap);
    void removeSurplusInputs(const quint64 inputSize_, QMap<quint64, QNEPort *> &portMap);
    void removeSurplusOutputs(const quint64 outputSize_, QMap<quint64, QNEPort *> &portMap);

    //! functions to load GraphicElement atributes through a binary data stream
    // --- Serialization Helpers ---

    void loadNewFormat(QDataStream &stream, QMap<quint64, QNEPort *> &portMap);
    void loadOldFormat(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version);

    // --- Position Loading ---

    void loadPos(QDataStream &stream);

    // --- Port Loading ---

    void loadInputPort(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const int port);
    void loadInputPorts(QDataStream &stream, QMap<quint64, QNEPort *> &portMap);
    void loadOutputPort(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const int port);
    void loadOutputPorts(QDataStream &stream, QMap<quint64, QNEPort *> &portMap);
    void loadPortsSize(QDataStream &stream, const QVersionNumber version);

    // --- Property Loading ---

    void loadLabel(QDataStream &stream, const QVersionNumber version);
    void loadPixmapSkinName(QDataStream &stream, const int skin);
    void loadPixmapSkinNames(QDataStream &stream, const QVersionNumber version);
    void loadPriority(QDataStream &stream, const QVersionNumber version);
    void loadRotation(QDataStream &stream, const QVersionNumber version);
    void loadTrigger(QDataStream &stream, const QVersionNumber version);

    // --- Display & Interaction ---

    void highlight(const bool isSelected);

    // --- Members: Element Type & Identity ---

    ElementGroup m_elementGroup = ElementGroup::Unknown;
    ElementType m_elementType = ElementType::Unknown;
    LogicElement *m_logic = nullptr;
    quint64 m_priority = 0;

    // --- Members: Trigger & Label ---

    QKeySequence m_trigger;
    QString m_labelText;

    // --- Members: Pixmap & Skins ---

    QString m_currentPixmapPath;

    // --- Members: Capabilities (Feature Flags) ---

    bool m_canChangeSkin = false;
    bool m_hasAudio = false;
    bool m_hasAudioBox = false;
    bool m_hasColors = false;
    bool m_hasDelay = false;
    bool m_hasFrequency = false;
    bool m_hasLabel = false;
    bool m_hasTrigger = false;
    bool m_hasTruthTable = false;

    // --- Members: Transform & Display State ---

    bool m_rotatable = true;
    bool m_selected = false;
    qreal m_angle = 0;

    // --- Members: Port Size Constraints ---

    quint64 m_minInputSize = 0;
    quint64 m_maxInputSize = 0;
    quint64 m_minOutputSize = 0;
    quint64 m_maxOutputSize = 0;
};

Q_DECLARE_METATYPE(GraphicElement)

QDataStream &operator<<(QDataStream &stream, const GraphicElement *item);

// FIXME: connecting more than one source makes element stop working
