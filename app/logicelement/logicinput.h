/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICINPUT_H
#define LOGICINPUT_H

#include "logicelement.h"

class LogicInput : public LogicElement
{
public:
    explicit LogicInput(bool defaultValue = false, int n_outputs=1);

    /* LogicElement interface */
protected:
    void _updateLogic(const std::vector<bool> &) override;
};

#endif // LOGICINPUT_H
