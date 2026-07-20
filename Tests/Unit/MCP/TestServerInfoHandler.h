// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestServerInfoHandler : public QObject
{
    Q_OBJECT

private slots:
    void testHandleGetServerInfoReturnsRealInfo();
    void testHandleListCommandsReturnsFullTableAndSortedCategories();

    void testHandleDescribeCommandRejectsMissingName();
    void testHandleDescribeCommandRejectsEmptyName();
    void testHandleDescribeCommandRejectsUnknownName();
    void testHandleDescribeCommandRejectsNoValidator();
    void testHandleDescribeCommandReturnsRealSchemas();

    void testHandleCommandRejectsUnknownCommand();
};
