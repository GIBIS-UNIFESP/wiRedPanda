// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "truth_table.h"

#include "globalproperties.h"

namespace
{
    int id = qRegisterMetaType<TruthTable>();
}

TruthTable::TruthTable(QGraphicsItem *parent)
    : GraphicElement(ElementType::TruthTable, ElementGroup::IC, ":/basic/truthtable.svg", tr("TruthTable"), tr("TruthTable"), 2, 8, 1, 1, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }
    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);
    setHasTruthTable(true);
    setCanChangeSkin(true);

}
