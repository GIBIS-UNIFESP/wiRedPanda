// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "enums.h"
#include "itemwithid.h"
#include "logicelement.h"

#include <QGraphicsItem>
#include <QKeySequence>
#include <QPixmapCache>
#include <QVersionNumber>
#include <memory>

class GraphicElement;
class QNEInputPort;
class QNEOutputPort;
class QNEPort;

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
    enum { Type = QGraphicsItem::UserType + 3 };
    int type() const override { return Type; }

    explicit GraphicElement(ElementType type, ElementGroup group, const QString &pixmapPath, const QString &titleText, const QString &translatedName,
                            const int minInputSize, const int maxInputSize, const int minOutputSize, const int maxOutputSize, QGraphicsItem *parent = nullptr);
    explicit GraphicElement(QGraphicsItem *parent = nullptr) : QGraphicsObject(parent) {}
    GraphicElement(const GraphicElement &other) : GraphicElement(other.parentItem()) {}

    //! Saves the graphic element through a binary data stream.
    virtual void save(QDataStream &stream) const;

    /**
     * @brief Loads the graphic element through a binary data stream.
     * @param portMap receives a reference to each input and output port.
     */
    virtual void load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version);

    //! Updates the number and the connected elements to the ports whenever needed (e.g. loading the element, changing the number of inputs/outputs).
    virtual void updatePortsProperties();

    //! virtual function overloaded by clock element. Other elements have frequency of 0.
    virtual float frequency() const;

    //! virtual function overloaded by clock element. Other elements have frequency of 0.
    virtual float delay() const;

    //! Updates the GraphicElement theme according to the dark/light WiRedPanda theme.
    virtual void updateTheme();

    ElementGroup elementGroup() const;
    ElementType elementType() const;
    LogicElement *logic() const;
    QKeySequence trigger() const;
    QNEInputPort *inputPort(const int index = 0);
    QNEOutputPort *outputPort(const int index = 0);
    QPointF pixmapCenter() const;
    QRectF boundingRect() const override;
    QString label() const;
    QString previousColor() const;
    QString nextColor() const;
    QString previousAudio() const;
    QString nextAudio() const;
    bool canChangeSkin() const;
    bool hasAudio() const;
    bool hasColors() const;
    bool hasFrequency() const;
    bool hasDelay() const;
    bool hasLabel() const;
    bool hasTrigger() const;
    bool hasTruthTable() const;
    bool hasAudioBox() const;
    bool hasNodeConnection() const;
    bool isRotatable() const;
    bool isValid();
    const QVector<QNEInputPort *> &inputs() const;
    const QVector<QNEOutputPort *> &outputs() const;
    int inputSize() const;
    int maxInputSize() const;
    int maxOutputSize() const;
    int minInputSize() const;
    int minOutputSize() const;
    int outputSize() const;
    int priority() const;
    qreal rotation() const;
    virtual QString audio() const;
    virtual QString color() const;
    virtual QString genericProperties();
    virtual void refresh();
    virtual void setAudio(const QString &audio);
    virtual void setColor(const QString &color);
    virtual void setFrequency(const float freq);
    virtual void setDelay(const float delay);
    virtual void setSkin(const bool defaultSkin, const QString &fileName);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void retranslate();
    void rotatePorts(const qreal angle);
    void setInputSize(const int size);
    void setInputs(const QVector<QNEInputPort *> &inputs);
    void setLabel(const QString &label);
    void setLogic(LogicElement *newLogic);
    void setOutputSize(const int size);
    void setOutputs(const QVector<QNEOutputPort *> &outputs);
    void setPixmap(const QString &pixmapPath);
    void setPixmap(const int index);
    void setPortName(const QString &name);
    void setPriority(const int value);
    void setRotation(const qreal angle);
    void setTrigger(const QKeySequence &trigger);
    void setIsWireless(const bool isWireless);
    void updateLabel();
    void setMapId(const int Id);
    int mapId() const;

protected:
    QPixmap pixmap() const;
    QRectF portsBoundingRect() const;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    bool sceneEvent(QEvent *event) override;
    void setCanChangeSkin(const bool canChangeSkin);
    void setHasAudio(const bool hasAudio);
    void setHasColors(const bool hasColors);
    void setHasFrequency(const bool hasFrequency);
    void setHasDelay(const bool hasDelay);
    void setHasLabel(const bool hasLabel);
    void setHasTrigger(const bool hasTrigger);
    void setHasTruthTable(const bool hasTruthTable);
    void setHasAudioBox(const bool hasAudioBox);
    void setMaxInputSize(const int maxInputSize);
    void setMaxOutputSize(const int maxOutputSize);
    void setMinInputSize(const int minInputSize);
    void setMinOutputSize(const int minOutputSize);
    void setRotatable(const bool rotatable);
    void setHasNodeConnection(const bool hasNodeConnection);

    //! Path to all default skins. The default skin is in a resource file.
    QStringList m_defaultSkins;

    //! Path to all custom skins. Custom skin names are system file paths defined by the user.
    QStringList m_alternativeSkins;

    //! input port vector
    QVector<QNEInputPort *> m_inputPorts;

    //! output port vector
    QVector<QNEOutputPort *> m_outputPorts;

    //! Current pixmap displayed for this GraphicElement.
    std::unique_ptr<QPixmap> m_pixmap = std::make_unique<QPixmap>();

    QColor m_selectionBrush;
    QColor m_selectionPen;
    QGraphicsTextItem *m_label = new QGraphicsTextItem(this);
    QString m_pixmapPath;
    QString m_titleText;
    QString m_translatedName;
    bool m_usingDefaultSkin = true;

private:
    static void removePortFromMap(QNEPort *deletedPort, QMap<quint64, QNEPort *> &portMap);

    //! adds an input port at the end of the input port vector.
    void addInputPort(const QString &name = {});

    //! adds an output port at the end of the output port vector.
    void addOutputPort(const QString &name = {});

    //! adds an input or output port at the end of the port vector.
    void addPort(const QString &name, const bool isOutput, const int ptr = 0);

    //! functions to load GraphicElement atributes through a binary data stream
    void loadPos(QDataStream &stream);

    void highlight(const bool isSelected);
    void loadInputPort(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const int port);
    void loadInputPorts(QDataStream &stream, QMap<quint64, QNEPort *> &portMap);
    void loadLabel(QDataStream &stream, const QVersionNumber version);
    void loadNewFormat(QDataStream &stream, QMap<quint64, QNEPort *> &portMap);
    void loadOldFormat(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version);
    void loadOutputPort(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const int port);
    void loadOutputPorts(QDataStream &stream, QMap<quint64, QNEPort *> &portMap);
    void loadPixmapSkinName(QDataStream &stream, const int skin);
    void loadPixmapSkinNames(QDataStream &stream, const QVersionNumber version);
    void loadPortsSize(QDataStream &stream, const QVersionNumber version);
    void loadPriority(QDataStream &stream, const QVersionNumber version);
    void loadRotation(QDataStream &stream, const QVersionNumber version);
    void loadTrigger(QDataStream &stream, const QVersionNumber version);
    void removeSurplusInputs(const quint64 inputSize_, QMap<quint64, QNEPort *> &portMap);
    void removeSurplusOutputs(const quint64 outputSize_, QMap<quint64, QNEPort *> &portMap);

    ElementGroup m_elementGroup = ElementGroup::Unknown;
    ElementType m_elementType = ElementType::Unknown;
    LogicElement *m_logic = nullptr;
    QKeySequence m_trigger;
    QString m_currentPixmapPath;
    QString m_labelText;
    bool m_canChangeSkin = false;
    bool m_hasAudio = false;
    bool m_hasColors = false;
    bool m_hasFrequency = false;
    bool m_hasLabel = false;
    bool m_hasTrigger = false;
    bool m_hasTruthTable = false;
    bool m_hasAudioBox = false;
    bool m_rotatable = true;
    bool m_hasNodeConnection = false;
    bool m_selected = false;
    bool m_hasDelay = false;
    bool m_isWireless = false;
    int m_mapId = -1;
    qreal m_angle = 0;
    quint64 m_maxInputSize = 0;
    quint64 m_maxOutputSize = 0;
    quint64 m_minInputSize = 0;
    quint64 m_minOutputSize = 0;
    quint64 m_priority = 0;
};

Q_DECLARE_METATYPE(GraphicElement)

QDataStream &operator<<(QDataStream &stream, const GraphicElement *item);

// FIXME: connecting more than one source makes element stop working
