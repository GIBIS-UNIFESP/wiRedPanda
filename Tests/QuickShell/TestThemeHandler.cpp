// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestThemeHandler.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "App/Core/ThemeManager.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Handlers/ThemeHandler.h"

namespace {

/// Local equivalent of Tests/Common/TestUtils.h's ScopedThemeOverride -- that header is
/// Widgets-coupled overall (pulls in QApplication/Scene.h), but the struct itself is a trivial,
/// portable RAII guard.
struct ScopedThemeOverride {
    Theme original = ThemeManager::theme();
    ~ScopedThemeOverride() { ThemeManager::setTheme(original); }
};

} // namespace

void TestThemeHandler::testHandleGetThemeReturnsCurrentTheme()
{
    const Theme prevTheme = ThemeManager::theme();
    ThemeManager::setTheme(Theme::Dark);

    ThemeHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("get_theme", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["theme"].toString(), QStringLiteral("dark"));

    ThemeManager::setTheme(prevTheme);
}

void TestThemeHandler::testHandleSetThemeAppliesLightDarkAndSystem()
{
    const Theme prevTheme = ThemeManager::theme();

    ThemeHandler handler(nullptr, nullptr);

    const QJsonObject lightResponse = handler.handleCommand("set_theme", {{"theme", "light"}}, 1);
    QVERIFY2(lightResponse.contains("result"), qPrintable(QJsonDocument(lightResponse).toJson()));
    QCOMPARE(ThemeManager::theme(), Theme::Light);
    QCOMPARE(lightResponse["result"].toObject()["theme"].toString(), QStringLiteral("light"));

    const QJsonObject darkResponse = handler.handleCommand("set_theme", {{"theme", "dark"}}, 1);
    QCOMPARE(ThemeManager::theme(), Theme::Dark);
    QCOMPARE(darkResponse["result"].toObject()["theme"].toString(), QStringLiteral("dark"));

    // Case-insensitive, and covers the System branch.
    const QJsonObject systemResponse = handler.handleCommand("set_theme", {{"theme", "SYSTEM"}}, 1);
    QCOMPARE(ThemeManager::theme(), Theme::System);
    QCOMPARE(systemResponse["result"].toObject()["theme"].toString(), QStringLiteral("system"));

    ThemeManager::setTheme(prevTheme);
}

void TestThemeHandler::testHandleSetThemeRejectsMissingParam()
{
    ThemeHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("set_theme", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestThemeHandler::testHandleSetThemeRejectsInvalidValue()
{
    ThemeHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("set_theme", {{"theme", "purple"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestThemeHandler::testHandleGetEffectiveThemeReturnsResolvedTheme()
{
    const Theme prevTheme = ThemeManager::theme();
    ThemeManager::setTheme(Theme::Light);

    ThemeHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("get_effective_theme", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["effective_theme"].toString(), QStringLiteral("light"));

    ThemeManager::setTheme(prevTheme);
}

void TestThemeHandler::testHandleGetEffectiveThemeResolvesSystemTheme()
{
    // ThemeManager::effectiveTheme() is exactly
    // (theme()==System) ? resolveSystemTheme() : theme() -- the only interesting branch (the
    // trivial passthrough is already covered above). resolveSystemTheme() always resolves to
    // a concrete Light or Dark, never "system" itself, regardless of the real OS setting.
    ScopedThemeOverride themeGuard;
    ThemeManager::setTheme(Theme::System);

    ThemeHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("get_effective_theme", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QString effectiveTheme = response["result"].toObject()["effective_theme"].toString();
    QVERIFY2(effectiveTheme == QStringLiteral("light") || effectiveTheme == QStringLiteral("dark"),
              qPrintable(effectiveTheme));
}

void TestThemeHandler::testHandleCommandRejectsUnknownCommand()
{
    ThemeHandler handler(nullptr, nullptr);

    const QJsonObject response = handler.handleCommand("totally_made_up", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}
