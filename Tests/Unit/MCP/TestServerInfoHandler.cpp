// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestServerInfoHandler.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QTest>

#include "App/UI/MainWindow.h"
#include "MCP/Server/Core/JsonRpcError.h"
#include "MCP/Server/Core/MCPValidator.h"
#include "MCP/Server/Handlers/ServerInfoHandler.h"

namespace {
QString serverInfoRealSchemaPath()
{
    return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("schema-mcp.json");
}
} // namespace

void TestServerInfoHandler::testHandleGetServerInfoReturnsRealInfo()
{
    MainWindow window;
    ServerInfoHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("get_server_info", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();

    QCOMPARE(result["server_name"].toString(), QStringLiteral("wiRedPanda MCP Server"));
    QCOMPARE(result["status"].toString(), QStringLiteral("ready"));
    QVERIFY(!result["version"].toString().isEmpty());
    const QJsonArray capabilities = result["capabilities"].toArray();
    QVERIFY(capabilities.contains(QJsonValue("circuit_design")));
    QVERIFY(capabilities.contains(QJsonValue("simulation")));
}

void TestServerInfoHandler::testHandleListCommandsReturnsFullTableAndSortedCategories()
{
    MainWindow window;
    ServerInfoHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("list_commands", {}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();

    const QJsonArray commands = result["commands"].toArray();
    QCOMPARE(commands.size(), 43); // matches MCPProcessor's real dispatch-map count

    bool foundCreateElement = false;
    for (const QJsonValue &v : commands) {
        if (v.toObject()["name"].toString() == "create_element") {
            foundCreateElement = true;
            QCOMPARE(v.toObject()["category"].toString(), QStringLiteral("element"));
        }
    }
    QVERIFY(foundCreateElement);

    const QJsonArray categories = result["categories"].toArray();
    QStringList categoryStrings;
    for (const QJsonValue &v : categories) {
        categoryStrings.append(v.toString());
    }
    QStringList sorted = categoryStrings;
    sorted.sort();
    QCOMPARE(categoryStrings, sorted);
    QCOMPARE(categoryStrings.size(), QSet<QString>(categoryStrings.begin(), categoryStrings.end()).size());
}

void TestServerInfoHandler::testHandleDescribeCommandRejectsMissingName()
{
    MainWindow window;
    ServerInfoHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("describe_command", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestServerInfoHandler::testHandleDescribeCommandRejectsEmptyName()
{
    MainWindow window;
    ServerInfoHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("describe_command", {{"name", ""}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::InvalidParams);
}

void TestServerInfoHandler::testHandleDescribeCommandRejectsUnknownName()
{
    MainWindow window;
    ServerInfoHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("describe_command", {{"name", "totally_made_up"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}

void TestServerInfoHandler::testHandleDescribeCommandRejectsNoValidator()
{
    MainWindow window;
    ServerInfoHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("describe_command", {{"name", "get_server_info"}}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject error = response["error"].toObject();
    QCOMPARE(error["code"].toInt(), JsonRpcError::InternalError);
    QCOMPARE(error["message"].toString(), QStringLiteral("Schema validator not available"));
}

void TestServerInfoHandler::testHandleDescribeCommandReturnsRealSchemas()
{
    MCPValidator validator(serverInfoRealSchemaPath());
    QVERIFY(validator.isSchemaLoaded());

    MainWindow window;
    ServerInfoHandler handler(&window, &validator);

    const QJsonObject response = handler.handleCommand("describe_command", {{"name", "create_element"}}, 1);
    QVERIFY2(response.contains("result"), qPrintable(QJsonDocument(response).toJson()));
    const QJsonObject result = response["result"].toObject();

    QCOMPARE(result["name"].toString(), QStringLiteral("create_element"));
    QCOMPARE(result["category"].toString(), QStringLiteral("element"));
    QVERIFY2(!result["input_schema"].toObject().isEmpty(), qPrintable(QJsonDocument(response).toJson()));
    QVERIFY2(!result["response_schema"].toObject().isEmpty(), qPrintable(QJsonDocument(response).toJson()));
}

void TestServerInfoHandler::testHandleCommandRejectsUnknownCommand()
{
    MainWindow window;
    ServerInfoHandler handler(&window, nullptr);

    const QJsonObject response = handler.handleCommand("totally_made_up", {}, 1);
    QVERIFY2(response.contains("error"), qPrintable(QJsonDocument(response).toJson()));
    QCOMPARE(response["error"].toObject()["code"].toInt(), JsonRpcError::MethodNotFound);
}
