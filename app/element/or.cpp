#include "or.h"

Or::Or(QGraphicsItem *parent)
    : GraphicElement(2, 8, 1, 1, parent)
{
    pixmapSkinName.append(":/basic/or.png");
    setOutputsOnTop(true);
    setPixmap(pixmapSkinName[0]);
    setCanChangeSkin(true);
    updatePorts();
    setPortName("OR");
}

void Or::setSkin(bool defaultSkin, QString filename)
{
    if (defaultSkin)
        pixmapSkinName[0] = ":/basic/or.png";
    else
        pixmapSkinName[0] = filename;
    setPixmap(pixmapSkinName[0]);
}
