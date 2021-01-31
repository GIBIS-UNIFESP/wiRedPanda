/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include <QGraphicsItem>
#include <QKeySequence>

#include "common.h"
#include "elementtype.h"
#include "itemwithid.h"
#include "nodes/qneport.h"

enum class ElementGroup : uint_fast8_t { UNKNOWN, OTHER, IC, INPUT, GATE, MEMORY, OUTPUT, MUX, STATICINPUT };

#define MAXIMUMVALIDINPUTSIZE 256

class GraphicElement;

typedef QVector<GraphicElement *> ElementVector;

class GraphicElement : public QGraphicsObject, public ItemWithId
{
    Q_OBJECT
public:
    enum : uint32_t { Type = QGraphicsItem::UserType + 3 };

    GraphicElement(ElementType type, ElementGroup group, int minInputSz, int maxInputSz, int minOutputSz, int maxOutputSz, QGraphicsItem *parent = nullptr);

protected:
    QVector<QString> pixmapSkinName;

    /* GraphicElement interface. */
public:
    ElementType elementType() const;

    ElementGroup elementGroup() const;

    virtual void save(QDataStream &ds) const;

    virtual void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version);

    virtual void updatePorts();

    virtual void refresh();

    /* QGraphicsItem interface */
public:
    int type() const override
    {
        return Type;
    }

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QNEPort *addPort(const QString &name, bool isOutput, int flags = 0, int ptr = 0);

    void addInputPort(const QString &name = QString());

    void addOutputPort(const QString &name = QString());

    virtual void setPortName(const QString &name);

    virtual void setSkin(bool defaultSkin, const QString &filename);

    int topPosition() const;

    int bottomPosition() const;

    int maxInputSz() const;

    int maxOutputSz() const;

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

    virtual void setColor(const QString &color);
    virtual QString getColor() const;

    virtual void setAudio(const QString &audio);
    virtual QString getAudio() const;

    bool isValid();

    void setLabel(const QString &label);
    QString getLabel() const;

    void updateTheme();
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
    void updateSkinsPath(const QString &newSkinPath);

protected:
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

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    bool usingDefaultSkin;

private:
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
    bool m_disabled;
    ElementType m_elementType;
    ElementGroup m_elementGroup;
    QString m_labelText;
    QKeySequence m_trigger;

    void loadPos(QDataStream &ds);

    void loadAngle(QDataStream &ds);

    void loadLabel(QDataStream &ds, double version);

    void loadMinMax(QDataStream &ds, double version);

    void loadTrigger(QDataStream &ds, double version);

    void loadInputPorts(QDataStream &ds, QMap<quint64, QNEPort *> &portMap);

    void removePortFromMap(QNEPort *deletedPort, QMap<quint64, QNEPort *> &portMap);

    void loadOutputPorts(QDataStream &ds, QMap<quint64, QNEPort *> &portMap);

    void removeSurplusInputs(quint64 inputSz, QMap<quint64, QNEPort *> &portMap);

    void removeSurplusOutputs(quint64 outputSz, QMap<quint64, QNEPort *> &portMap);

    void loadInputPort(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, size_t port);

    void loadOutputPort(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, size_t port);

    void loadPixmapSkinNames(QDataStream &ds, double version);

    void loadPixmapSkinName(QDataStream &ds, size_t skin);

protected:
    QVector<QNEInputPort *> m_inputs;
    QVector<QNEOutputPort *> m_outputs;
};

#endif /* GRAPHICELEMENT_H */
