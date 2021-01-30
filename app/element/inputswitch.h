#ifndef INPUTSWITCH_H
#define INPUTSWITCH_H

#include "graphicelement.h"
#include "input.h"

class InputSwitch : public GraphicElement, public Input
{
public:
    explicit InputSwitch(QGraphicsItem *parent = nullptr);
    virtual ~InputSwitch() override = default;
    static int current_id_number; // Number used to create distinct labels for each instance of this element.

    bool on;
    /* QGraphicsItem interface */
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /* GraphicElement interface */
public:
    virtual ElementType elementType() override
    {
        return (ElementType::SWITCH);
    }
    virtual ElementGroup elementGroup() override
    {
        return (ElementGroup::INPUT);
    }

    /* GraphicElement interface */
public:
    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    virtual bool getOn() const override;
    virtual void setOn(bool value) override;
    void setSkin(bool defaultSkin, QString filename) override;
};

#endif /* INPUTSWITCH_H */
