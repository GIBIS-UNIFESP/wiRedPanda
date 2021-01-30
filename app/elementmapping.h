#ifndef ELEMENTMAPPING_H
#define ELEMENTMAPPING_H

#include "graphicelement.h"
#include "input.h"
#include "logicelement.h"
#include "logicelement/logicinput.h"

#include <QGraphicsScene>
#include <QHash>
#include <QMap>
#include <QObject>
#include <QTimer>

class Clock;
class IC;

class ElementMapping;
class ICMapping;
typedef QMap<GraphicElement *, LogicElement *> ElementMap;
typedef QMap<Input *, LogicElement *> InputMap;

class ElementMapping
{
public:
    ElementMapping(const QVector<GraphicElement *> &elms, QString file = QString());
    virtual ~ElementMapping();

    void clear();

    static QVector<GraphicElement *> sortGraphicElements(QVector<GraphicElement *> elms);

    virtual void initialize();

    void sort();

    void update();

    ICMapping *getICMapping(IC *ic) const;
    LogicElement *getLogicElement(GraphicElement *elm) const;

    bool canRun() const;
    bool canInitialize() const;

protected:
    // Attributes
    QString currentFile;
    bool initialized;
    ElementMap map;
    InputMap inputMap;
    QVector<Clock *> clocks;
    QVector<GraphicElement *> elements;
    QMap<IC *, ICMapping *> icMappings;
    QVector<LogicElement *> logicElms;

    LogicInput globalGND;
    LogicInput globalVCC;

    QVector<LogicElement *> deletableElements;

    // Methods
    LogicElement *buildLogicElement(GraphicElement *elm);

    void setDefaultValue(GraphicElement *elm, QNEPort *in);
    void applyConnection(GraphicElement *elm, QNEPort *in);
    void generateMap();
    void connectElements();
    void validateElements();
    void sortLogicElements();
    static int calculatePriority(GraphicElement *elm, QHash<GraphicElement *, bool> &beingvisited, QHash<GraphicElement *, int> &priority);
    void insertElement(GraphicElement *elm);
    void insertIC(IC *ic);
};

#endif // ELEMENTMAPPING_H
