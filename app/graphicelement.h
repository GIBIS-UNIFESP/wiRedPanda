#ifndef GRAPHICELEMENT_H
#define GRAPHICELEMENT_H

#include <cstdint>

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QKeySequence>

#include "common.h"
#include "itemwithid.h"
#include "nodes/qneport.h"

// enum class ElementType : uint_fast8_t {
//  UNKNOWN, BUTTON, SWITCH, LED, NOT, AND, OR, NAND, NOR, CLOCK, XOR, XNOR, VCC, GND, DISPLAY,
//  DLATCH, JKLATCH, DFLIPFLOP, JKFLIPFLOP, SRFLIPFLOP, TFLIPFLOP, TLATCH, BOX, NODE, MUX, DEMUX,
//  BUZZER, DISPLAY14, LEDGRID
//};

enum class ElementType : uint_fast8_t {
    UNKNOWN,
    BUTTON,
    SWITCH,
    LED,
    NOT,
    AND,
    OR,
    NAND,
    NOR,
    CLOCK,
    XOR,
    XNOR,
    VCC,
    GND,
    DISPLAY,
    DLATCH,
    JKLATCH,
    DFLIPFLOP,
    JKFLIPFLOP,
    SRFLIPFLOP,
    TFLIPFLOP,
    UNUSED,
    IC,
    NODE,
    MUX,
    DEMUX,
    BUZZER,
    DISPLAY14
};

enum class ElementGroup : uint_fast8_t { UNKNOWN, OTHER, IC, INPUT, GATE, MEMORY, OUTPUT, MUX, STATICINPUT };

#define MAXIMUMVALIDINPUTSIZE 256

class GraphicElement;

typedef QVector<GraphicElement *> ElementVector;
typedef QVector<QNEPort *> QNEPortVector;

class GraphicElement : public QGraphicsObject, public ItemWithId
{
    Q_OBJECT
public:
    enum : uint32_t { Type = QGraphicsItem::UserType + 3 };

    explicit GraphicElement(int minInputSz, int maxInputSz, int minOutputSz, int maxOutputSz, QGraphicsItem *parent = nullptr);

private:
    QPixmap *pixmap;
    QString currentPixmapName;
    QColor m_selectionBrush;
    QColor m_selectionPen;

protected:
    QVector<QString> pixmapSkinName;

    /* GraphicElement interface. */
public:
    virtual ElementType elementType() = 0;
    bool usingDefaultSkin;

    virtual ElementGroup elementGroup() = 0;

    virtual void save(QDataStream &ds) const;

    virtual void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version);

    virtual void updatePorts();

    virtual void refresh();

    /* QGraphicsItem interface */
public:
    int type() const
    {
        return (Type);
    }

    virtual QRectF boundingRect() const;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QNEPort *addPort(const QString &name, bool isOutput, int flags = 0, int ptr = 0);

    void addInputPort(const QString &name = QString());

    void addOutputPort(const QString &name = QString());

    virtual void setPortName(QString name);

    virtual void setSkin(bool defaultSkin, QString filename);

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

    virtual void setColor(QString getColor);
    virtual QString getColor() const;

    virtual void setAudio(QString audio);
    virtual QString getAudio() const;
    /*
     *  bool beingVisited( ) const;
     *  void setBeingVisited( bool beingVisited );
     */

    /*
     *  bool visited( ) const;
     *  void setVisited( bool visited );
     */

    bool isValid();

    void setLabel(QString label);
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
    void updateSkinsPath(QString newSkinPath);

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

    /*  virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e); */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    QGraphicsTextItem *label;
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
