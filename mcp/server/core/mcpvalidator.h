// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QJsonObject>
#include <QString>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

using json = nlohmann::json;
using nlohmann::json_schema::json_validator;

/*!
 * @struct ValidationResult
 * @brief Result of JSON schema validation
 */
struct ValidationResult
{
    bool isValid = false;
    QString errorMessage;
    QString schemaUsed;
    QString commandType;
    QString errorPath;

    ValidationResult() = default;
    ValidationResult(bool valid, const QString &error = QString(), const QString &schema = QString(),
                    const QString &command = QString(), const QString &path = QString())
        : isValid(valid), errorMessage(error), schemaUsed(schema), commandType(command), errorPath(path) {}
};

/*!
 * @class MCPValidator
 * @brief JSON Schema validator for MCP commands and responses using native json-schema-validator
 *
 * Simplified validator using pboettch/json-schema-validator for native JSON Schema Draft 7 support.
 */
class MCPValidator
{
public:
    explicit MCPValidator(const QString &schemaPath);
    ~MCPValidator();

    // Core validation methods
    ValidationResult validateCommand(const QJsonObject &command);
    ValidationResult validateCommand(const json &command);
    ValidationResult validateResponse(const QJsonObject &response, const QString &expectedCommand = QString());
    ValidationResult validateResponse(const json &response, const QString &expectedCommand = QString());

    // Schema management
    bool loadSchema(const QString &schemaPath);
    bool isSchemaLoaded() const;
    QString getSchemaPath() const;

    // Utility methods
    static json qjsonToNlohmann(const QJsonObject &qjson);
    static QJsonObject nlohmannToQJson(const json &nlohmannJson);

private:
    // Internal validation methods
    ValidationResult validateAgainstSchema(const json &data, const json &schema, const QString &commandType);
    json findCommandSchema(const QString &commandName);
    json findResponseSchema(const QString &commandName);
    QString extractErrorPath(const QString &errorMsg);

    // Member variables
    json m_schema;
    json m_commandSchemas;
    json m_responseSchemas;
    std::unique_ptr<json_validator> m_validator;
    QString m_schemaPath;
    bool m_schemaLoaded = false;
};
