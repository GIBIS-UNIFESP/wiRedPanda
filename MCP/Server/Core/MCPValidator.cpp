// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Core/MCPValidator.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>

MCPValidator::MCPValidator(const QString &schemaPath)
    : m_schemaPath(schemaPath)
{
    // Load schema but don't fail constructor if it fails
    loadSchema(schemaPath);
}

MCPValidator::~MCPValidator() = default;

bool MCPValidator::loadSchema(const QString &schemaPath)
{
    m_schemaPath = schemaPath;
    m_schemaLoaded = false;

    QFile schemaFile(schemaPath);
    if (!schemaFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray schemaData = schemaFile.readAll();
    schemaFile.close();

    try {
        // Parse schema JSON
        m_schema = json::parse(schemaData.toStdString());

        // Create native validator
        m_validator = std::make_unique<json_validator>();
        m_validator->set_root_schema(m_schema);

        // Extract command and response schemas for easier access
        if (m_schema.contains("properties")) {
            auto properties = m_schema["properties"];
            if (properties.contains("commands") && properties["commands"].contains("properties")) {
                m_commandSchemas = properties["commands"]["properties"];
            }
            if (properties.contains("responses") && properties["responses"].contains("properties")) {
                m_responseSchemas = properties["responses"]["properties"];
            }
        }

        m_schemaLoaded = true;
        return true;

    } catch (const json::parse_error &) {
        return false;
    } catch (const json::exception &) {
        // Empirically, not found reachable: json::parse() failures are always
        // json::parse_error (caught above); every schema-shape defect tried against the real
        // json-schema-validator library (invalid "type" values, unresolvable/external $ref)
        // either compiles silently or throws a plain std::exception (caught below), never a
        // json::exception subtype. The subsequent .contains()/[] extraction calls on
        // m_schema also can't throw: nlohmann's contains() returns false (not an exception)
        // for a non-object value, short-circuiting every nested [] access below it.
        return false; // LCOV_EXCL_LINE
    } catch (const std::exception &) {
        return false;
    }
}

bool MCPValidator::isSchemaLoaded() const
{
    return m_schemaLoaded;
}

QString MCPValidator::schemaPath() const
{
    return m_schemaPath;
}

ValidationResult MCPValidator::validateCommand(const QJsonObject &command)
{
    try {
        return validateCommand(qjsonToNlohmann(command));
    } catch (const std::exception &e) {
        return ValidationResult(false, QString("Malformed command: %1").arg(e.what()));
    }
}

ValidationResult MCPValidator::validateCommand(const json &command)
{
    if (!m_schemaLoaded) {
        return ValidationResult(false, "Schema not loaded");
    }

    // Extract method type (JSON-RPC 2.0 format)
    QString commandType = "unknown";
    if (command.contains("method") && command["method"].is_string()) {
        commandType = QString::fromStdString(command["method"].get<std::string>());
    } else {
        return ValidationResult(false, "Missing or invalid 'method' field", "", commandType);
    }

    // Find the appropriate command schema
    json commandSchema = findCommandSchema(commandType);
    if (commandSchema.is_null()) {
        return ValidationResult(false,
            QString("No schema found for command type: %1").arg(commandType),
            "", commandType);
    }

    // Validate against the command schema using native validator
    return validateAgainstSchema(command, commandSchema, commandType, "command:" + commandType.toStdString());
}

ValidationResult MCPValidator::validateResponse(const QJsonObject &response, const QString &expectedCommand)
{
    try {
        return validateResponse(qjsonToNlohmann(response), expectedCommand);
    } catch (const std::exception &e) {
        return ValidationResult(false, QString("Malformed response: %1").arg(e.what()));
    }
}

ValidationResult MCPValidator::validateResponse(const json &response, const QString &expectedCommand)
{
    if (!m_schemaLoaded) {
        return ValidationResult(false, "Schema not loaded");
    }

    // First validate against base CommandResponse schema
    json baseResponseSchema;
    if (m_schema.contains("definitions") && m_schema["definitions"].contains("CommandResponse")) {
        baseResponseSchema = m_schema["definitions"]["CommandResponse"];
    } else {
        return ValidationResult(false, "CommandResponse schema not found in definitions");
    }

    ValidationResult baseResult = validateAgainstSchema(response, baseResponseSchema, expectedCommand, "response:base");
    if (!baseResult.isValid) {
        return ValidationResult(false,
            QString("Base response validation failed: %1").arg(baseResult.errorMessage),
            "base_response", expectedCommand);
    }

    // If we have a specific expected command, validate against specific response schema.
    // Response schemas are keyed by "<command>_response" in the schema (see
    // ServerInfoHandler::describeCommand()'s identical lookup) -- passing the bare command
    // name here always missed, silently skipping specific response validation entirely.
    if (!expectedCommand.isEmpty()) {
        json responseSchema = findResponseSchema(expectedCommand + "_response");
        if (!responseSchema.is_null()) {
            ValidationResult specificResult = validateAgainstSchema(response, responseSchema, expectedCommand, "response:" + expectedCommand.toStdString());
            if (!specificResult.isValid) {
                return ValidationResult(false,
                    QString("Specific response validation failed: %1").arg(specificResult.errorMessage),
                    expectedCommand + "_response", expectedCommand);
            }
            return ValidationResult(true, "", expectedCommand + "_response", expectedCommand);
        }
    }

    return ValidationResult(true, "", "base_response", expectedCommand);
}

ValidationResult MCPValidator::validateAgainstSchema(const json &data, const json &schema, const QString &commandType, const std::string &cacheKey)
{
    try {
        // Use the main validator which has access to all definitions for $ref resolution
        if (m_validator) {
            json_validator *validator = nullptr;

            // Schemas are immutable after load — compile each one once (F44);
            // a fresh json_validator per call was the dominant per-request cost.
            const auto it = m_validatorCache.find(cacheKey);
            if (it != m_validatorCache.end()) {
                validator = it->second.get();
            } else {
                // For schema references that may contain $ref, we need to create a complete schema
                json fullSchema = schema;

                // If this is a sub-schema without definitions but contains $ref (directly or
                // nested under allOf/anyOf/oneOf, as every "<command>_response" schema does via
                // its "$ref": "#/definitions/CommandResponse" combinator), we need to include
                // the definitions section. Attaching it unconditionally (rather than gating on
                // "properties", which misses allOf/anyOf/oneOf-shaped schemas entirely) is
                // harmless for schemas that don't need it -- an unreferenced "definitions" key
                // has no effect on validation.
                if (m_schema.contains("definitions")) {
                    fullSchema["definitions"] = m_schema["definitions"];
                    fullSchema["$schema"] = m_schema["$schema"];
                }

                auto compiled = std::make_unique<json_validator>();
                compiled->set_root_schema(fullSchema);
                validator = m_validatorCache.emplace(cacheKey, std::move(compiled)).first->second.get();
            }

            // This throws on validation failure with detailed error messages
            validator->validate(data);
        }

        return ValidationResult(true, "", "", commandType);

    } catch (const std::exception &e) {
        // Native error messages are much better than our manual ones
        QString errorMsg = QString::fromStdString(e.what());
        QString errorPath = extractErrorPath(errorMsg);

        return ValidationResult(false, errorMsg, "", commandType, errorPath);
    }
}

json MCPValidator::findCommandSchema(const QString &commandName) const
{
    if (!m_commandSchemas.is_null() && m_commandSchemas.contains(commandName.toStdString())) {
        json schema = m_commandSchemas[commandName.toStdString()];

        // Handle $ref resolution
        if (schema.contains("$ref") && schema["$ref"].is_string()) {
            QString refPath = QString::fromStdString(schema["$ref"].get<std::string>());
            if (refPath.startsWith("#/definitions/")) {
                QString defName = refPath.mid(14); // Remove "#/definitions/"
                if (m_schema.contains("definitions") && m_schema["definitions"].contains(defName.toStdString())) {
                    return m_schema["definitions"][defName.toStdString()];
                }
            }
        }

        return schema;
    }

    return json();
}

json MCPValidator::findResponseSchema(const QString &commandName) const
{
    if (!m_responseSchemas.is_null() && m_responseSchemas.contains(commandName.toStdString())) {
        json schema = m_responseSchemas[commandName.toStdString()];

        // Handle $ref resolution
        if (schema.contains("$ref") && schema["$ref"].is_string()) {
            QString refPath = QString::fromStdString(schema["$ref"].get<std::string>());
            if (refPath.startsWith("#/definitions/")) {
                QString defName = refPath.mid(14); // Remove "#/definitions/"
                if (m_schema.contains("definitions") && m_schema["definitions"].contains(defName.toStdString())) {
                    return m_schema["definitions"][defName.toStdString()];
                }
            }
        }

        return schema;
    }

    return json();
}

QString MCPValidator::extractErrorPath(const QString &errorMsg)
{
    // Extract JSON path from error message using regex. The native validator's real message
    // format is "At <path> of <value> - <reason>" (e.g. "At /x of \"y\" - unexpected instance
    // type", or "At  of {...} - ..." with an empty path at the root) -- confirmed empirically
    // against the library, not assumed from the older "at '/path'" comment below, which no
    // real message this validator produces ever matches (case mismatch and no quotes).
    static const QRegularExpression pathRegex(R"(^At\s(\S*)\s+of\s)");
    QRegularExpressionMatch match = pathRegex.match(errorMsg);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    // Empirically, not found reachable: this is only ever called with a real
    // json_validator::validate() failure message (every violation kind tried -- required,
    // type, enum, additionalProperties, oneOf -- consistently used the "At ... of ... - ..."
    // format above). A schema-*compilation* failure (a different message shape, e.g. an
    // unresolvable $ref) can't reach here either: any such $ref would already have made
    // set_root_schema() reject the whole document at load time, so m_commandSchemas/
    // m_responseSchemas could never contain an entry whose own compilation could fail this way.
    return QString(); // LCOV_EXCL_LINE
}

json MCPValidator::qjsonToNlohmann(const QJsonObject &qjson)
{
    QJsonDocument doc(qjson);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    try {
        return json::parse(jsonData.toStdString());
    } catch (const json::parse_error &e) {
        throw std::runtime_error(std::string("qjsonToNlohmann: JSON parse error: ") + e.what());
    }
}

QJsonObject MCPValidator::nlohmannToQJson(const json &nlohmannJson)
{
    std::string jsonString = nlohmannJson.dump();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(jsonString), &parseError);
    if (parseError.error != QJsonParseError::NoError || doc.isNull()) {
        qWarning() << "nlohmannToQJson: failed to parse JSON:" << parseError.errorString();
        return {};
    }
    return doc.object();
}
