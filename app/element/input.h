/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INPUT_H
#define INPUT_H

class Input
{
public:
    virtual bool getOn(int port=0) const = 0;
    virtual void setOn(bool value, int port=0) = 0;
    virtual int outputSize() = 0;
};

#endif /* INPUT_H */
