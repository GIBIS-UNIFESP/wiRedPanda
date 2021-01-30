#include "dflipflop.h"

DFlipFlop::DFlipFlop(QGraphicsItem *parent)
    : GraphicElement(4, 4, 2, 2, parent)
{
    pixmapSkinName.append(":/memory/D-flipflop.png");
    setPixmap(pixmapSkinName[0]);
    setRotatable(false);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("FlipFlop D");
    input(0)->setName("Data");
    input(1)->setName("Clock");
    input(2)->setName("~Preset");
    input(3)->setName("~Clear");
    output(0)->setName("Q");
    output(1)->setName("~Q");

    output(0)->setDefaultValue(0);
    output(1)->setDefaultValue(1);

    input(2)->setRequired(false);
    input(3)->setRequired(false);
    input(2)->setDefaultValue(1);
    input(3)->setDefaultValue(1);
    lastValue = false;
    lastClk = false;
}

void DFlipFlop::updatePorts()
{
    input(0)->setPos(topPosition(), 13); /* Data */
    input(1)->setPos(topPosition(), 45); /* Clock */
    input(2)->setPos(32, topPosition()); /* Preset */
    input(3)->setPos(32, bottomPosition()); /* Clear */
    output(0)->setPos(bottomPosition(), 15); /* Q */
    output(1)->setPos(bottomPosition(), 45); /* ~Q */
}

void DFlipFlop::setSkin(bool defaultSkin, QString filename)
{
    if (defaultSkin)
        pixmapSkinName[0] = ":/memory/D-flipflop.png";
    else
        pixmapSkinName[0] = filename;
    setPixmap(pixmapSkinName[0]);
}
