// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "truth_table.h"

#include "globalproperties.h"
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
}

void TruthTable::updatePortsProperties()
{
    int index = 1;
    const int step = GlobalProperties::gridSize / 2;

    if (!m_inputPorts.isEmpty()) {
        int y = 32 - (m_inputPorts.size() * step) + step;

        for (auto *port : qAsConst(m_inputPorts)) {

            if (!isRotatable()) {
                port->setRotation(0);
            }

            port->setPos(0, y);

            y += step * 2;

            port->setName("Input (" + QString::number(index)  + ")");
            index++;
        }
    }

    index = 1;

    if (!m_outputPorts.isEmpty()) {
        int y = 32 - (m_outputPorts.size() * step) + step;

        for (auto *port : qAsConst(m_outputPorts)) {

            if (!isRotatable()) {
                port->setRotation(0);
            }

            port->setPos(64, y);

            y += step * 2;

            port->setName("Output (" + QString::number(index)  + ")");
            index++;
        }
    }
}
