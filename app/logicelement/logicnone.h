/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef LOGICNONE_H
#define LOGICNONE_H

#include "logicelement.h"

class LogicNone : public LogicElement
{
  /* LogicElement interface */
public:
  explicit LogicNone();
protected:
    void _updateLogic(const std::vector<bool> &inputs) override;
};

#endif // LOGICNONE_H
