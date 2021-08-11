/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include <QGraphicsItem>
#include <QKeySequence>

#include "elementtype.h"
#include "itemwithid.h"

enum class ElementGroup : uint_fast8_t { UNKNOWN, OTHER, IC, INPUT, GATE, MEMORY, OUTPUT, MUX, STATICINPUT, REMOTE };

#define MAXIMUMVALIDINPUTSIZE 256

class GraphicElement;
class QNEPort;
class QNEInputPort;
class QNEOutputPort;

typedef QVector<GraphicElement *> ElementVector;

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
    ~GraphicElement() {}

    /* GraphicElement interface. */
    ElementType elementType() const;
    ElementGroup elementGroup() const;

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

    virtual void refresh();

    /* QGraphicsItem interface */
    int type() const override
    {
        return Type;
    }

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /**
     * @brief addPort: adds an input or output port at the end of the port vector.
     */
    QNEPort *addPort(const QString &name, bool isOutput, int flags = 0, int ptr = 0);

    /**
     * @brief addInputPort: adds an input port at the end of the input port vector.
     */
    void addInputPort(const QString &name = QString());

    /**
     * @brief addOutputPort: adds an output port at the end of the output port vector.
     */
    void addOutputPort(const QString &name = QString());

    virtual void setPortName(const QString &name);

    virtual void setSkin(bool defaultSkin, const QString &filename);

    int topPosition() const;

    int bottomPosition() const;

    int maxInputSz() const;

    int maxOutputSz() const;

    /**
     * @brief outputsOnTop: returns true if the output ports are on the top position of the GraphicElement.
     */
    bool outputsOnTop() const;

    QVector<QNEInputPort *> inputs() const;
    void setInputs(const QVector<QNEInputPort *> &inputs);

    QVector<QNEOutputPort *> outputs() const;

    const QNEInputPort *input(int pos = 0) const;
    const QNEOutputPort *output(int pos = 0) const;

    QNEInputPort *input(int pos = 0);
    QNEOutputPort *output(int pos = 0);

    void setOutputs(const QVector<QNEOutputPort *> &outputs);

    int minInputSz() const;

    int minOutputSz() const;

    int inputSize() const;
    void setInputSize(int size);

    int outputSize() const;
    void setOutputSize(const int size);

    /**
     * @brief getFrequency: virtual function overloaded by clock element. Other elements have frequency of 0.
     */
    virtual float getFrequency() const;
    virtual void setFrequency(float freq);

    void setPixmap(const QString &pixmapName);
    void setPixmap(const QString &pixmapName, QRect size);

    bool rotatable() const;

    bool hasLabel() const;

    bool canChangeSkin() const;

    bool hasFrequency() const;

    bool hasColors() const;

    bool hasTrigger() const;

    bool hasAudio() const;

    bool hasCustomConfig( ) const;

    virtual void setColor(const QString &color);
    virtual QString getColor() const;

    virtual void setAudio(const QString &audio);
    virtual QString getAudio() const;

    bool isValid();

    void setLabel(const QString &label);
    QString getLabel() const;

    /**
     * @brief updateTheme: Updates the GraphicElement theme according to the dark/light wiRed Panda theme.
     */
    void updateTheme();
    /**
     * @brief updateThemeLocal: unfinished function with no current use.
     */
    virtual void updateThemeLocal();

    void disable();
    void enable();
    bool disabled();

    QPixmap getPixmap() const;

    QKeySequence getTrigger() const;
    void setTrigger(const QKeySequence &trigger);

    virtual QString genericProperties();

    // Update label in graphical interface
    void updateLabel();

private:
    /**
     * @brief Current pixmap displayed for this GraphicElement.
     */
    QPixmap *m_pixmap;
    QString m_currentPixmapName;
    QColor m_selectionBrush;
    QColor m_selectionPen;
    QGraphicsTextItem *m_label;
    int m_topPosition;
    int m_bottomPosition;
    quint64 m_maxInputSz;
    quint64 m_maxOutputSz;
    quint64 m_minInputSz;
    quint64 m_minOutputSz;
    bool m_outputsOnTop;
    bool m_rotatable;
    bool m_hasLabel;
    bool m_canChangeSkin;
    bool m_hasFrequency;
    bool m_hasColors;
    bool m_hasTrigger;
    bool m_hasAudio;
    bool m_hasCustomConfig;
    bool m_disabled;
    ElementType m_elementType;
    ElementGroup m_elementGroup;
    QString m_labelText;
    QKeySequence m_trigger;

    /**
     * functions to load GraphicElement atributes through a binary data stream
     */
    void loadPos(QDataStream &ds);
    void loadAngle(QDataStream &ds);
    void loadLabel(QDataStream &ds, double version);
    void loadMinMax(QDataStream &ds, double version);
    void loadTrigger(QDataStream &ds, double version);
    void loadInputPorts(QDataStream &ds, QMap<quint64, QNEPort *> &portMap);
    void loadOutputPorts(QDataStream &ds, QMap<quint64, QNEPort *> &portMap);
    void loadInputPort(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, size_t port);
    void loadOutputPort(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, size_t port);
    void loadPixmapSkinNames(QDataStream &ds, double version);
    void loadPixmapSkinName(QDataStream &ds, size_t skin);

    void removePortFromMap(QNEPort *deletedPort, QMap<quint64, QNEPort *> &portMap);
    void removeSurplusInputs(quint64 inputSz, QMap<quint64, QNEPort *> &portMap);
    void removeSurplusOutputs(quint64 outputSz, QMap<quint64, QNEPort *> &portMap);

protected:
    /**
     * @brief Path to all current skins. The default skin is in a research file. Custom skin names are system file paths defined by the user.
     */
    QVector<QString> m_pixmapSkinName;

    void setRotatable(bool rotatable);
    void setHasLabel(bool hasLabel);
    void setHasFrequency(bool hasFrequency);
    void setHasColors(bool hasColors);
    void setCanChangeSkin(bool canChangeSkin);
    void setHasTrigger(bool hasTrigger);
    void setMinInputSz(const int minInputSz);
    void setMinOutputSz(int minOutputSz);
    void setHasAudio(bool hasAudio);
    void setOutputsOnTop(bool outputsOnTop);
    void setMaxOutputSz(int maxOutputSz);
    void setMaxInputSz(int maxInputSz);
    void setTopPosition(int topPosition);
    void setBottomPosition(int bottomPosition);
    void setHasCustomConfig( bool hasCustomConfig );

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    bool m_usingDefaultSkin;

    /**
     * @brief m_inputs: input port vector
     */
    QVector<QNEInputPort *> m_inputs;
    /**
     * @brief m_outputs: output port vector
     */
    QVector<QNEOutputPort *> m_outputs;
};

#endif /* GRAPHICELEMENT_H */
