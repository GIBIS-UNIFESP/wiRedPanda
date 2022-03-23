/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICNODE_H
#define LOGICNODE_H

#include "logicelement.h"

class LogicNode : public LogicElement
{
public:
    explicit LogicNode();

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &) override;
};

#endif // LOGICNODE_H