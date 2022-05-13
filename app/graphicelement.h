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
    enum : uint32_t { Type = QGraphicsItem::UserType + 3 };

    GraphicElement(ElementType type, ElementGroup group, int minInputSz, int maxInputSz, int minOutputSz, int maxOutputSz, QGraphicsItem *parent = nullptr);
    GraphicElement(QGraphicsItem *parent = nullptr) : QGraphicsObject(parent) {};
    GraphicElement(const GraphicElement &other) : GraphicElement(other.parentItem()) {};

    /**
     * @brief Saves the graphic element through a binary data stream.
     */
    virtual void save(QDataStream &ds) const;

    /**
     * @brief Loads the graphic element through a binary data stream.
     * @param portMap receives a reference to each input and output port.
     */
    virtual void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version);

    /**
     * @brief updatePorts: Updates the number and the connected elements to the ports whenever needed (e.g. loading the element, changing the number of inputs/outputs).
     */
    virtual void updatePorts();

    int type() const override
    {
        return Type;
    }

    /**
     * @brief addPort: adds an input or output port at the end of the port vector.
     */
    QNEPort *addPort(const QString &name, bool isOutput, int flags = 0, int ptr = 0);

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
     * @brief getFrequency: virtual function overloaded by clock element. Other elements have frequency of 0.
     */
    virtual float getFrequency() const;
    virtual void setFrequency(float /*freq*/);

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
    QKeySequence getTrigger() const;
    QNEInputPort *input(int pos = 0);
    QNEOutputPort *output(int pos = 0);
    QPixmap getPixmap() const;
    QRectF boundingRect() const override;
    QString getLabel() const;
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
    const QNEInputPort *input(int pos = 0) const;
    const QNEOutputPort *output(int pos = 0) const;
    int bottomPosition() const;
    int inputSize() const;
    int maxInputSz() const;
    int maxOutputSz() const;
    int minInputSz() const;
    int minOutputSz() const;
    int outputSize() const;
    int topPosition() const;
    virtual QString genericProperties();
    virtual QString getAudio() const;
    virtual QString getColor() const;
    virtual void refresh();
    virtual void setAudio(const QString &audio);
    virtual void setColor(const QString &color);
    virtual void setPortName(const QString &name);
    virtual void setSkin(bool defaultSkin, const QString &filename);
    void disable();
    void enable();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void setInputSize(int size);
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
    void setBottomPosition(int bottomPosition);
    void setCanChangeSkin(bool canChangeSkin);
    void setHasAudio(bool hasAudio);
    void setHasColors(bool hasColors);
    void setHasFrequency(bool hasFrequency);
    void setHasLabel(bool hasLabel);
    void setHasTrigger(bool hasTrigger);
    void setMaxInputSz(int maxInputSz);
    void setMaxOutputSz(int maxOutputSz);
    void setMinInputSz(const int minInputSz);
    void setMinOutputSz(int minOutputSz);
    void setOutputsOnTop(bool outputsOnTop);
    void setRotatable(bool rotatable);
    void setTopPosition(int topPosition);

    /**
     * @brief Path to all current skins. The default skin is in a research file. Custom skin names are system file paths defined by the user.
     */
    QVector<QString> m_pixmapSkinName;
    /**
     * @brief m_inputs: input port vector
     */
    QVector<QNEInputPort *> m_inputs;
    /**
     * @brief m_outputs: output port vector
     */
    QVector<QNEOutputPort *> m_outputs;

    bool m_usingDefaultSkin;

private:
    /**
     * functions to load GraphicElement atributes through a binary data stream
     */
    void loadPos(QDataStream &ds);

    void loadAngle(QDataStream &ds);
    void loadInputPort(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, size_t port);
    void loadInputPorts(QDataStream &ds, QMap<quint64, QNEPort *> &portMap);
    void loadLabel(QDataStream &ds, double version);
    void loadMinMax(QDataStream &ds, double version);
    void loadOutputPort(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, size_t port);
    void loadOutputPorts(QDataStream &ds, QMap<quint64, QNEPort *> &portMap);
    void loadPixmapSkinName(QDataStream &ds, size_t skin);
    void loadPixmapSkinNames(QDataStream &ds, double version);
    void loadTrigger(QDataStream &ds, double version);
    void removePortFromMap(QNEPort *deletedPort, QMap<quint64, QNEPort *> &portMap);
    void removeSurplusInputs(quint64 inputSz, QMap<quint64, QNEPort *> &portMap);
    void removeSurplusOutputs(quint64 outputSz, QMap<quint64, QNEPort *> &portMap);

    /**
     * @brief Current pixmap displayed for this GraphicElement.
     */
    QPixmap *m_pixmap;

    ElementGroup m_elementGroup;
    ElementType m_elementType;
    QColor m_selectionBrush;
    QColor m_selectionPen;
    QGraphicsTextItem *m_label;
    QKeySequence m_trigger;
    QString m_currentPixmapName;
    QString m_labelText;
    bool m_canChangeSkin;
    bool m_disabled;
    bool m_hasAudio;
    bool m_hasColors;
    bool m_hasFrequency;
    bool m_hasLabel;
    bool m_hasTrigger;
    bool m_outputsOnTop;
    bool m_rotatable;
    int m_bottomPosition;
    int m_topPosition;
    quint64 m_maxInputSz;
    quint64 m_maxOutputSz;
    quint64 m_minInputSz;
    quint64 m_minOutputSz;
};

Q_DECLARE_METATYPE(GraphicElement)

QDataStream &operator<<(QDataStream &ds, const GraphicElement *item);
