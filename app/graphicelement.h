/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "elementtype.h"
#include "itemwithid.h"

#include <QGraphicsItem>
#include <QKeySequence>

enum class ElementGroup : uint_fast8_t {
    Gate = 4,
    IC = 2,
    Input = 3,
    Memory = 5,
    Mux = 7,
    Other = 1,
    Output = 6,
    StaticInput = 8,
    Unknown = 0,
};

constexpr auto maximumValidInputSize = 256;

class GraphicElement;
class QNEPort;
class QNEInputPort;
class QNEOutputPort;

using ElementVector = QVector<GraphicElement *>;

/**
 * @brief Virtual class to implement graphical element appearance, input and output ports, and tooltips.
 *
 * The appearance includes editable features such as pose, colors, skins, shortcuts, and labels.
 * It also implements the functions to handle loading and saving the element into files.
 */
class GraphicElement : public QGraphicsObject, public ItemWithId
{
    Q_OBJECT

public:
    enum { Type = QGraphicsItem::UserType + 3 };
    int type() const override { return Type; }

    explicit GraphicElement(ElementType type, ElementGroup group, const int minInputSize, const int maxInputSize, const int minOutputSize, const int maxOutputSize, QGraphicsItem *parent = nullptr);
    explicit GraphicElement(QGraphicsItem *parent = nullptr) : QGraphicsObject(parent) {};
    GraphicElement(const GraphicElement &other) : GraphicElement(other.parentItem()) {};

    /**
     * @brief Saves the graphic element through a binary data stream.
     */
    virtual void save(QDataStream &stream) const;

    /**
     * @brief Loads the graphic element through a binary data stream.
     * @param portMap receives a reference to each input and output port.
     */
    virtual void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version);

    /**
     * @brief updatePorts: Updates the number and the connected elements to the ports whenever needed (e.g. loading the element, changing the number of inputs/outputs).
     */
    virtual void updatePorts();

    /**
     * @brief addPort: adds an input or output port at the end of the port vector.
     */
    QNEPort *addPort(const QString &name, const bool isOutput, const int flags = 0, const int ptr = 0);

    /**
     * @brief addInputPort: adds an input port at the end of the input port vector.
     */
    void addInputPort(const QString &name = {});

    /**
     * @brief addOutputPort: adds an output port at the end of the output port vector.
     */
    void addOutputPort(const QString &name = {});

    /**
     * @brief outputsOnTop: returns true if the output ports are on the top position of the GraphicElement.
     */
    bool outputsOnTop() const;

    /**
     * @brief frequency: virtual function overloaded by clock element. Other elements have frequency of 0.
     */
    virtual float frequency() const;
    virtual void setFrequency(const float freq);

    /**
     * @brief updateTheme: Updates the GraphicElement theme according to the dark/light WiRedPanda theme.
     */
    void updateTheme();

    /**
     * @brief updateThemeLocal: unfinished function with no current use.
     */
    virtual void updateThemeLocal();

    ElementGroup elementGroup() const;
    ElementType elementType() const;
    QKeySequence trigger() const;
    QNEInputPort *input(const int pos = 0);
    QNEOutputPort *output(const int pos = 0);
    QPixmap pixmap() const;
    QRectF boundingRect() const override;
    QString label() const;
    QVector<QNEInputPort *> inputs() const;
    QVector<QNEOutputPort *> outputs() const;
    bool canChangeSkin() const;
    bool disabled() const;
    bool hasAudio() const;
    bool hasColors() const;
    bool hasFrequency() const;
    bool hasLabel() const;
    bool hasTrigger() const;
    bool isValid();
    bool rotatable() const;
    const QNEInputPort *input(const int pos = 0) const;
    const QNEOutputPort *output(const int pos = 0) const;
    int bottomPosition() const;
    int inputSize() const;
    int maxInputSize() const;
    int maxOutputSize() const;
    int minInputSize() const;
    int minOutputSize() const;
    int outputSize() const;
    int topPosition() const;
    virtual QString genericProperties();
    virtual QString audio() const;
    virtual QString color() const;
    virtual void refresh();
    virtual void setAudio(const QString &audio);
    virtual void setColor(const QString &color);
    virtual void setPortName(const QString &name);
    virtual void setSkin(const bool defaultSkin, const QString &fileName);
    void disable();
    void enable();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void setInputSize(const int size);
    void setInputs(const QVector<QNEInputPort *> &inputs);
    void setLabel(const QString &label);
    void setOutputSize(const int size);
    void setOutputs(const QVector<QNEOutputPort *> &outputs);
    void setPixmap(const QString &pixmapName);
    void setPixmap(const QString &pixmapName, QRect size);
    void setTrigger(const QKeySequence &trigger);
    void updateLabel();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void setBottomPosition(const int bottomPosition);
    void setCanChangeSkin(const bool canChangeSkin);
    void setHasAudio(const bool hasAudio);
    void setHasColors(const bool hasColors);
    void setHasFrequency(const bool hasFrequency);
    void setHasLabel(const bool hasLabel);
    void setHasTrigger(const bool hasTrigger);
    void setMaxInputSize(const int maxInputSize);
    void setMaxOutputSize(const int maxOutputSize);
    void setMinInputSize(const int minInputSize);
    void setMinOutputSize(const int minOutputSize);
    void setOutputsOnTop(const bool outputsOnTop);
    void setRotatable(const bool rotatable);
    void setTopPosition(const int topPosition);

    /**
     * @brief Path to all current skins. The default skin is in a resource file. Custom skin names are system file paths defined by the user.
     */
    QStringList m_pixmapSkinName;
    /**
     * @brief m_inputs: input port vector
     */
    QVector<QNEInputPort *> m_inputs;
    /**
     * @brief m_outputs: output port vector
     */
    QVector<QNEOutputPort *> m_outputs;

    bool m_usingDefaultSkin = true;

private:
    /**
     * functions to load GraphicElement atributes through a binary data stream
     */
    void loadPos(QDataStream &stream);

    void loadAngle(QDataStream &stream);
    void loadInputPort(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const int port);
    void loadInputPorts(QDataStream &stream, QMap<quint64, QNEPort *> &portMap);
    void loadLabel(QDataStream &stream, const double version);
    void loadMinMax(QDataStream &stream, const double version);
    void loadOutputPort(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const int port);
    void loadOutputPorts(QDataStream &stream, QMap<quint64, QNEPort *> &portMap);
    void loadPixmapSkinName(QDataStream &stream, const int skin);
    void loadPixmapSkinNames(QDataStream &stream, const double version);
    void loadTrigger(QDataStream &stream, const double version);
    void removePortFromMap(QNEPort *deletedPort, QMap<quint64, QNEPort *> &portMap);
    void removeSurplusInputs(quint64 inputSize, QMap<quint64, QNEPort *> &portMap);
    void removeSurplusOutputs(quint64 outputSize, QMap<quint64, QNEPort *> &portMap);

    /**
     * @brief Current pixmap displayed for this GraphicElement.
     */
    QPixmap *m_pixmap = nullptr;

    ElementGroup m_elementGroup = ElementGroup::Unknown;
    ElementType m_elementType = ElementType::Unknown;
    QColor m_selectionBrush;
    QColor m_selectionPen;
    QGraphicsTextItem *m_label = new QGraphicsTextItem(this);
    QKeySequence m_trigger;
    QString m_currentPixmapName;
    QString m_labelText;
    bool m_canChangeSkin = false;
    bool m_disabled = false;
    bool m_hasAudio = false;
    bool m_hasColors = false;
    bool m_hasFrequency = false;
    bool m_hasLabel = false;
    bool m_hasTrigger = false;
    bool m_outputsOnTop = true;
    bool m_rotatable = true;
    int m_bottomPosition = 64;
    int m_topPosition = 0;
    quint64 m_maxInputSize = 0;
    quint64 m_maxOutputSize = 0;
    quint64 m_minInputSize = 0;
    quint64 m_minOutputSize = 0;
};

Q_DECLARE_METATYPE(GraphicElement)

QDataStream &operator<<(QDataStream &stream, const GraphicElement *item);
