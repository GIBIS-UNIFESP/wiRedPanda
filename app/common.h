/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <iostream>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(zero)
Q_DECLARE_LOGGING_CATEGORY(one)
Q_DECLARE_LOGGING_CATEGORY(two)
Q_DECLARE_LOGGING_CATEGORY(three)
Q_DECLARE_LOGGING_CATEGORY(four)
Q_DECLARE_LOGGING_CATEGORY(five)

class Comment
{
public:
    static void setVerbosity(const int verbosity);
};

#define ERRORMSG(exp) std::string(__FILE__) + ": " + std::to_string(__LINE__) + ": " + std::string(__FUNCTION__) + ": Error: " + exp
