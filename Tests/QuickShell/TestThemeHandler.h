// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// `ThemeHandler` itself (MCP/Server/Handlers/ThemeHandler.{h,cpp}) is shared, portable code (in
/// CMakeSources.cmake's general SOURCES, not WIDGETS_ONLY_SOURCES) and is reused as-is -- not
/// subclassed -- by QuickMCPProcessor's own
/// `std::make_unique<ThemeHandler>(nullptr, m_validator.get())`: its body only ever reaches
/// ThemeManager's static theme state, never the MainWindow*/nullptr its constructor stores.
/// Keeps the original class name, matching this project's convention for classes shared
/// verbatim with no Quick-specific adaptation (TestSerialization/TestConnections/
/// TestNotifyCatch), rather than a "TestQuick"-prefixed name.
class TestThemeHandler : public QObject
{
    Q_OBJECT

private slots:
    void testHandleGetThemeReturnsCurrentTheme();
    void testHandleSetThemeAppliesLightDarkAndSystem();
    void testHandleSetThemeRejectsMissingParam();
    void testHandleSetThemeRejectsInvalidValue();
    void testHandleGetEffectiveThemeReturnsResolvedTheme();
    void testHandleGetEffectiveThemeResolvesSystemTheme();
    void testHandleCommandRejectsUnknownCommand();
};
