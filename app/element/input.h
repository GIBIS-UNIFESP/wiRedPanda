/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INPUT_H
#define INPUT_H

class Input
{
public:
    virtual bool getOn() const = 0;
    virtual void setOn(bool value) = 0;
};

#endif /* INPUT_H */
