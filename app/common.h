/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QLoggingCategory>
#include <stdexcept>

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

class Pandaception : public std::runtime_error
{
public:
    explicit Pandaception(const QString &message);
};

class Common
{
public:
    inline static bool incrementLabel = false;
};
