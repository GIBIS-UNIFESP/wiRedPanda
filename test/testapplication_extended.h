// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestApplicationExtended : public QObject
{
    Q_OBJECT

private slots:
    void testApplicationStartup();
    void testApplicationShutdown();
    void testApplicationSettings();
    void testApplicationCommands();
    void testApplicationErrorHandling();
};