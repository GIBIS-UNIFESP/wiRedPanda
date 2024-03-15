// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "truth_table.h"

#include "elementfactory.h"
#include "globalproperties.h"
#include "qneconnection.h"
#include "qneport.h"

namespace
{
    int id = qRegisterMetaType<TruthTable>();
}

TruthTable::TruthTable(QGraphicsItem *parent)
    : GraphicElement(ElementType::TruthTable, ElementGroup::IC, ":/basic/truthtable-rotated.svg", tr("TruthTable"), tr("TruthTable"), 2, 8, 1, 8, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);
    setHasTruthTable(true);
    setCanChangeSkin(true);
    setHasLabel(true);

    TruthTable::updatePortsProperties();
    TruthTable::generatePixmap();
}

void TruthTable::updatePortsProperties()
{
    int index = 0;
    const int step = GlobalProperties::gridSize / 2;

    if (!m_inputPorts.isEmpty()) {
        int y = 32 - (m_inputPorts.size() * step) + step;

        for (auto *port : qAsConst(m_inputPorts)) {

            if (!isRotatable()) {
                port->setRotation(0);
            }

            port->setPos(0, y);

            y += step * 2;

            port->setName(QChar::fromLatin1('A' + index));
            index++;
        }
    }

    index = 0;

    if (!m_outputPorts.isEmpty()) {
        int y = 32 - (m_outputPorts.size() * step) + step;

        for (auto *port : qAsConst(m_outputPorts)) {

            if (!isRotatable()) {
                port->setRotation(0);
            }

            port->setPos(64, y);

            y += step * 2;

            port->setName("S" + QString::number(index));
            index++;
        }
    }
}

void TruthTable::generatePixmap()
{
    // make pixmap
    const QSize size = portsBoundingRect().united(QRectF(0, 0, 64, 64)).size().toSize();
    qDebug() << "Tamanho de portBounding: " << portsBoundingRect();
    qDebug() << "Tamanho de size: " << size;

    QPixmap tempPixmap(size);
    qDebug() << "Tamanho de tempPixmap: " << tempPixmap;
    tempPixmap.fill(Qt::transparent);

    QPainter tmpPainter(&tempPixmap);

    tmpPainter.setBrush(QColor(126, 126, 126));
    tmpPainter.setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));

    // draw package
    QPoint topLeft = tempPixmap.rect().topLeft();
    topLeft.setX(topLeft.x() + 7);
    QSize finalSize = tempPixmap.rect().size();
    finalSize.setWidth(finalSize.width() - 14);
    QRectF finalRect = QRectF(topLeft, finalSize);
    tmpPainter.drawRoundedRect(finalRect, 3, 3);

    QPixmap panda(":/basic/truthtable-rotated.svg");
    QPointF pandaOrigin = finalRect.center();
    pandaOrigin.setX(pandaOrigin.x() - panda.width() / 2);
    pandaOrigin.setY(pandaOrigin.y() - panda.height() / 2);
    tmpPainter.drawPixmap(pandaOrigin, panda);

    // draw shadow
    tmpPainter.setBrush(QColor(78, 78, 78));
    tmpPainter.setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));

    QRectF shadowRect(finalRect.bottomLeft(), finalRect.bottomRight());
    shadowRect.adjust(0, -3, 0, 0);
    tmpPainter.drawRoundedRect(shadowRect, 3, 3);

    // draw semicircle
    /*QRectF topCenter = QRectF(finalRect.topLeft() + QPointF(18, -12), QSize(24, 24));
    tmpPainter.drawChord(topCenter, 0, -180 * 16);*/

    m_pixmap = std::make_unique<QPixmap>(tempPixmap);

}

void TruthTable::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    TruthTable::updatePortsProperties();

    if (isSelected()) {
        painter->save();
        painter->setBrush(m_selectionBrush);
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        painter->drawRoundedRect(portsBoundingRect().united(QRectF(0, 0, 64, 64)), 5, 5);
        painter->restore();
    }

    generatePixmap();
    painter->drawPixmap(boundingRect().topLeft(), pixmap());
    qDebug() << "Tamanho painter->drawRoundedRect: " << painter;

}

/*void TruthTable::update()
{
    TruthTable::updatePortsProperties();

    const qreal bottom = portsBoundingRect().united(QRectF(0, 0, 64, 64)).bottom();
    m_label->setPos(30, bottom + 5);

    generatePixmap();
}*/
