// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/ServerInfoHandler.h"

#include <QJsonArray>
#include <QSet>

#include "MCP/Server/Core/MCPValidator.h"

namespace {
/// Maps each MCP method to a category for introspection.
/// Mirrors the routing wired up in MCPProcessor::MCPProcessor.
struct CommandEntry
{
    const char *name;
    const char *category;
};

constexpr CommandEntry kCommandTable[] = {
    // Server / introspection
    {"get_server_info",   "server"},
    {"list_commands",     "introspection"},
    {"describe_command",  "introspection"},
    // File
    {"load_circuit",         "file"},
    {"save_circuit",         "file"},
    {"new_circuit",          "file"},
    {"close_circuit",        "file"},
    {"get_tab_count",        "file"},
    {"export_image",         "file"},
    {"export_arduino",       "file"},
    {"export_systemverilog", "file"},
    // Element
    {"create_element",            "element"},
    {"delete_element",            "element"},
    {"list_elements",             "element"},
    {"move_element",              "element"},
    {"set_element_properties",    "element"},
    {"set_input_value",           "element"},
    {"get_output_value",          "element"},
    {"rotate_element",            "element"},
    {"flip_element",              "element"},
    {"update_element",            "element"},
    {"change_input_size",         "element"},
    {"change_output_size",        "element"},
    {"toggle_truth_table_output", "element"},
    {"morph_element",             "element"},
    // Connection
    {"connect_elements",    "connection"},
    {"disconnect_elements", "connection"},
    {"list_connections",    "connection"},
    {"split_connection",    "connection"},
    // Simulation
    {"simulation_control", "simulation"},
    {"create_waveform",    "simulation"},
    {"export_waveform",    "simulation"},
    // IC
    {"create_ic",          "ic"},
    {"instantiate_ic",     "ic"},
    {"list_ics",           "ic"},
    {"embed_ic",           "ic"},
    {"extract_ic",         "ic"},
    // History
    {"undo",               "history"},
    {"redo",               "history"},
    {"get_undo_stack",     "history"},
    // Theme
    {"get_theme",           "theme"},
    {"set_theme",           "theme"},
    {"get_effective_theme", "theme"},
};

QString categoryFor(const QString &commandName)
{
    for (const auto &entry : kCommandTable) {
        if (commandName == QLatin1String(entry.name)) {
            return QString::fromLatin1(entry.category);
        }
    }
    return {};
}
} // namespace

ServerInfoHandler::ServerInfoHandler(MainWindow *mainWindow, const MCPValidator *validator)
    : BaseHandler(mainWindow, validator)
{
}

QJsonObject ServerInfoHandler::handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId)
{
    if (command == "get_server_info") {
        return handleGetServerInfo(params, requestId);
    } else if (command == "list_commands") {
        return handleListCommands(params, requestId);
    } else if (command == "describe_command") {
        return handleDescribeCommand(params, requestId);
    } else {
        return createErrorResponse(QString("Unknown server info command: %1").arg(command),
                                   requestId, JsonRpcError::MethodNotFound);
    }
}

QJsonObject ServerInfoHandler::handleGetServerInfo(const QJsonObject &, const QJsonValue &requestId)
{
    QJsonObject result;
    result["server_name"] = "wiRedPanda MCP Server";
    result["version"] = APP_VERSION;
    result["protocol_version"] = "1.0";
    result["status"] = "ready";

    QJsonArray capabilities;
    capabilities.append("circuit_design");
    capabilities.append("simulation");
    capabilities.append("file_operations");
    capabilities.append("image_export");
    capabilities.append("waveform_generation");
    capabilities.append("ic_management");
    result["capabilities"] = capabilities;

    return createSuccessResponse(result, requestId);
}

QJsonObject ServerInfoHandler::handleListCommands(const QJsonObject &, const QJsonValue &requestId)
{
    QJsonArray commands;
    QSet<QString> categorySet;

    for (const auto &entry : kCommandTable) {
        QJsonObject cmd;
        cmd["name"] = QString::fromLatin1(entry.name);
        cmd["category"] = QString::fromLatin1(entry.category);
        commands.append(cmd);
        categorySet.insert(QString::fromLatin1(entry.category));
    }

    QStringList sortedCategories(categorySet.begin(), categorySet.end());
    sortedCategories.sort();

    QJsonArray categories;
    for (const QString &c : sortedCategories) {
        categories.append(c);
    }

    QJsonObject result;
    result["commands"] = commands;
    result["categories"] = categories;
    return createSuccessResponse(result, requestId);
}

QJsonObject ServerInfoHandler::handleDescribeCommand(const QJsonObject &params, const QJsonValue &requestId)
{
    if (!validateParameters(params, {"name"})) {
        return createErrorResponse("Missing required parameter: name", requestId, JsonRpcError::InvalidParams);
    }

    QString errorMsg;
    if (!validateNonEmptyString(params.value("name"), "name", errorMsg)) {
        return createErrorResponse(errorMsg, requestId, JsonRpcError::InvalidParams);
    }

    const QString name = params.value("name").toString();
    const QString category = categoryFor(name);

    if (category.isEmpty()) {
        return createErrorResponse(QString("Unknown command: %1").arg(name),
                                   requestId, JsonRpcError::MethodNotFound);
    }

    if (!m_validator) {
        return createErrorResponse("Schema validator not available",
                                   requestId, JsonRpcError::InternalError);
    }

    // m_responseSchemas indexes by "<name>_response" key; m_commandSchemas by bare name.
    const json inputSchema = m_validator->findCommandSchema(name);
    const json responseSchema = m_validator->findResponseSchema(name + "_response");

    QJsonObject result;
    result["name"] = name;
    result["category"] = category;
    result["input_schema"]    = inputSchema.is_null()    ? QJsonObject{} : MCPValidator::nlohmannToQJson(inputSchema);
    result["response_schema"] = responseSchema.is_null() ? QJsonObject{} : MCPValidator::nlohmannToQJson(responseSchema);
    return createSuccessResponse(result, requestId);
}
