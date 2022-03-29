// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"

Q_LOGGING_CATEGORY(zero,  "0")
Q_LOGGING_CATEGORY(one,   "1")
Q_LOGGING_CATEGORY(two,   "2")
Q_LOGGING_CATEGORY(three, "3")
Q_LOGGING_CATEGORY(four,  "4")
Q_LOGGING_CATEGORY(five,  "5")

void Comment::setVerbosity(const int verbosity)
{
    QString rules;

    switch(verbosity){
    default:                         [[fallthrough]];
    case 0:  rules += "0 = false\n"; [[fallthrough]];
    case 1:  rules += "1 = false\n"; [[fallthrough]];
    case 2:  rules += "2 = false\n"; [[fallthrough]];
    case 3:  rules += "3 = false\n"; [[fallthrough]];
    case 4:  rules += "4 = false\n"; [[fallthrough]];
    case 5:  rules += "5 = false\n";
    }

    QLoggingCategory::setFilterRules(rules);

    qSetMessagePattern("%{if-debug}%{line}: %{function} => %{endif}%{message}");
}
