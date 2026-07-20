// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestThemeHandler.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

#include "App/Core/ThemeManager.h"
#include "App/UI/MainWindow.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Handlers/ThemeHandler.h"

void TestThemeHandler::testHandleGetThemeReturnsCurrentTheme()
{
    const Theme prevTheme = ThemeManager::theme();
    ThemeManager::setTheme(Theme::Dark);

    MainWindow window;
    ThemeHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("get_theme", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["theme"].toString(), QStringLiteral("dark"));

    ThemeManager::setTheme(prevTheme);
}

void TestThemeHandler::testHandleSetThemeAppliesLightDarkAndSystem()
{
    const Theme prevTheme = ThemeManager::theme();

    MainWindow window;
    ThemeHandler handler(&window, nullptr);

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
    MainWindow window;
    ThemeHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("set_theme", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestThemeHandler::testHandleSetThemeRejectsInvalidValue()
{
    MainWindow window;
    ThemeHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("set_theme", {{"theme", "purple"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::ValidationError);
}

void TestThemeHandler::testHandleGetEffectiveThemeReturnsResolvedTheme()
{
    const Theme prevTheme = ThemeManager::theme();
    ThemeManager::setTheme(Theme::Light);

    MainWindow window;
    ThemeHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("get_effective_theme", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["result"].toObject()["effective_theme"].toString(), QStringLiteral("light"));

    ThemeManager::setTheme(prevTheme);
}

void TestThemeHandler::testHandleCommandRejectsUnknownCommand()
{
    MainWindow window;
    ThemeHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("totally_made_up", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}
