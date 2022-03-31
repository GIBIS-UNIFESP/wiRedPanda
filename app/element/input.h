/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

class Input
{
public:
    virtual bool getOn(int port = 0) const = 0;
    virtual void setOn(bool value, int port = 0) = 0;
    virtual int outputSize() const;
    virtual int outputValue() const;
    bool isLocked() const;
    void setLocked(bool value);

protected:
    bool locked;
};

