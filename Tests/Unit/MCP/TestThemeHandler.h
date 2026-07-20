// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestThemeHandler : public QObject
{
    Q_OBJECT

private slots:
    void testHandleGetThemeReturnsCurrentTheme();
    void testHandleSetThemeAppliesLightDarkAndSystem();
    void testHandleSetThemeRejectsMissingParam();
    void testHandleSetThemeRejectsInvalidValue();
    void testHandleGetEffectiveThemeReturnsResolvedTheme();
    void testHandleCommandRejectsUnknownCommand();
};
