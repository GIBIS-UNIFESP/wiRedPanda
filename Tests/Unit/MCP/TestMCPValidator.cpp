// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/MCP/TestMCPValidator.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryFile>
#include <QTest>

#include "MCP/Server/Core/MCPValidator.h"

namespace {

QString realSchemaPath()
{
    return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("schema-mcp.json");
}

json parseJson(const QJsonObject &obj)
{
    return MCPValidator::qjsonToNlohmann(obj);
}

// Writes \a contents to \a file (kept alive by the caller for the validator's lifetime; must
// already be open) and returns its path.
QString writeTempSchema(QTemporaryFile &file, const QByteArray &contents)
{
    file.write(contents);
    file.close();
    return file.fileName();
}

// Lacks a "CommandResponse" definition entirely -- validateResponse()'s base-schema lookup
// must fail cleanly rather than assume the real schema's shape.
const char *kSchemaWithoutCommandResponse = R"({
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "properties": {
        "commands": {"properties": {}},
        "responses": {"properties": {}}
    }
})";

// Exercises findCommandSchema()/findResponseSchema()'s $ref-resolution branch (unused by the
// real schema, which never puts $ref at a command/response's own top level). Every $ref here
// must genuinely resolve: set_root_schema() validates the *entire* document's references
// eagerly at load time (confirmed empirically against the library), so a document containing
// even one unresolvable/external $ref anywhere fails to load at all -- there is no way to get
// a loaded validator whose m_commandSchemas/m_responseSchemas contain an entry with a $ref
// that doesn't resolve.
const char *kSchemaWithRefs = R"({
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "definitions": {
        "RefTarget": {"type": "object"}
    },
    "properties": {
        "commands": {
            "properties": {
                "ref_command": {"$ref": "#/definitions/RefTarget"}
            }
        },
        "responses": {
            "properties": {
                "ref_command_response": {"$ref": "#/definitions/RefTarget"}
            }
        }
    }
})";

// A lone (unpaired) UTF-16 high surrogate: QJsonDocument::toJson() emits it verbatim as
// "\ud800" without validating surrogate pairing, but nlohmann::json::parse() correctly
// rejects it as invalid per strict JSON/Unicode rules -- confirmed empirically, the one
// reliable way found to make qjsonToNlohmann()'s round trip genuinely fail on a QJsonObject
// built entirely through Qt's own public API (Qt's serializer never emits mismatched
// surrogates itself, but it doesn't reject one placed into a QString beforehand either).
QJsonObject objectWithLoneSurrogate()
{
    QString bad;
    bad += QChar(0xD800);
    return QJsonObject{{"jsonrpc", "2.0"}, {"method", "get_server_info"}, {"broken", bad}, {"id", 1}};
}

} // namespace

void TestMCPValidator::testConstructorLoadsRealSchema()
{
    MCPValidator validator(realSchemaPath());
    QVERIFY(validator.isSchemaLoaded());
    QCOMPARE(validator.schemaPath(), realSchemaPath());
}

void TestMCPValidator::testConstructorFailsOnMissingFile()
{
    MCPValidator validator(QStringLiteral("/nonexistent/path/schema-mcp.json"));
    QVERIFY(!validator.isSchemaLoaded());
}

void TestMCPValidator::testConstructorFailsOnMalformedJson()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    const QString path = writeTempSchema(file, "{ this is not valid json");

    MCPValidator validator(path);
    QVERIFY(!validator.isSchemaLoaded());
}

void TestMCPValidator::testConstructorFailsOnUnresolvableSchemaRef()
{
    // Syntactically valid JSON, but the schema itself is semantically broken: the root $ref
    // can never resolve (no "definitions" section at all). json::parse() succeeds, so this
    // exercises set_root_schema()'s own std::exception path -- distinct from the
    // json::parse_error path above.
    QTemporaryFile file;
    QVERIFY(file.open());
    const QString path = writeTempSchema(file, R"({"$ref": "#/definitions/DoesNotExist"})");

    MCPValidator validator(path);
    QVERIFY(!validator.isSchemaLoaded());
}

void TestMCPValidator::testValidateCommandFailsWhenSchemaNotLoaded()
{
    MCPValidator validator(QStringLiteral("/nonexistent/path/schema-mcp.json"));
    const QJsonObject command{{"jsonrpc", "2.0"}, {"method", "get_server_info"}, {"id", 1}};

    const ValidationResult result = validator.validateCommand(command);
    QVERIFY(!result.isValid);
    QCOMPARE(result.errorMessage, QStringLiteral("Schema not loaded"));
}

void TestMCPValidator::testValidateCommandRejectsMissingMethodField()
{
    MCPValidator validator(realSchemaPath());
    const json command = parseJson(QJsonObject{{"jsonrpc", "2.0"}, {"id", 1}});

    const ValidationResult result = validator.validateCommand(command);
    QVERIFY(!result.isValid);
    QCOMPARE(result.errorMessage, QStringLiteral("Missing or invalid 'method' field"));
}

void TestMCPValidator::testValidateCommandRejectsUnknownMethod()
{
    MCPValidator validator(realSchemaPath());
    const QJsonObject command{{"jsonrpc", "2.0"}, {"method", "totally_made_up_method"}, {"id", 1}};

    const ValidationResult result = validator.validateCommand(command);
    QVERIFY(!result.isValid);
    QVERIFY2(result.errorMessage.contains("No schema found for command type"), qPrintable(result.errorMessage));
    QCOMPARE(result.commandType, QStringLiteral("totally_made_up_method"));
}

void TestMCPValidator::testValidateCommandAcceptsValidCommand()
{
    MCPValidator validator(realSchemaPath());
    const QJsonObject command{{"jsonrpc", "2.0"}, {"method", "get_server_info"}, {"params", QJsonObject()}, {"id", 1}};

    const ValidationResult result = validator.validateCommand(command);
    QVERIFY2(result.isValid, qPrintable(result.errorMessage));
    QCOMPARE(result.commandType, QStringLiteral("get_server_info"));
}

void TestMCPValidator::testValidateCommandRejectsSchemaViolation()
{
    MCPValidator validator(realSchemaPath());
    // create_element requires params.type/x/y; omit x/y entirely.
    const QJsonObject params{{"type", "And"}};
    const QJsonObject command{{"jsonrpc", "2.0"}, {"method", "create_element"}, {"params", params}, {"id", 1}};

    const ValidationResult result = validator.validateCommand(command);
    QVERIFY(!result.isValid);
    QCOMPARE(result.commandType, QStringLiteral("create_element"));
    QVERIFY2(!result.errorMessage.isEmpty(), "a schema violation must carry a real, non-empty error message");
    // The real validator's message format is "At <path> of <value> - <reason>" --
    // extractErrorPath() must pull the real failing path out of it (here "/params",
    // where "x"/"y" are missing), not just leave errorPath empty.
    QVERIFY2(!result.errorPath.isEmpty(), qPrintable(result.errorMessage));
}

void TestMCPValidator::testValidateCommandQJsonOverloadDelegates()
{
    MCPValidator validator(realSchemaPath());
    const QJsonObject command{{"jsonrpc", "2.0"}, {"method", "get_server_info"}, {"params", QJsonObject()}, {"id", 1}};

    const ValidationResult qjsonResult = validator.validateCommand(command);
    const ValidationResult jsonResult = validator.validateCommand(parseJson(command));

    QCOMPARE(qjsonResult.isValid, jsonResult.isValid);
    QCOMPARE(qjsonResult.commandType, jsonResult.commandType);
}

void TestMCPValidator::testValidateCommandQJsonOverloadCatchesMalformedInput()
{
    MCPValidator validator(realSchemaPath());

    const ValidationResult result = validator.validateCommand(objectWithLoneSurrogate());
    QVERIFY(!result.isValid);
    QVERIFY2(result.errorMessage.startsWith("Malformed command:"), qPrintable(result.errorMessage));
}

void TestMCPValidator::testValidateCommandCachesCompiledValidatorAcrossCalls()
{
    // First call compiles and caches a json_validator for "command:get_server_info"; the
    // second call must hit that cache entry instead of recompiling -- both must still
    // validate correctly either way.
    MCPValidator validator(realSchemaPath());
    const QJsonObject command{{"jsonrpc", "2.0"}, {"method", "get_server_info"}, {"params", QJsonObject()}, {"id", 1}};

    const ValidationResult first = validator.validateCommand(command);
    const ValidationResult second = validator.validateCommand(command);
    QVERIFY(first.isValid);
    QVERIFY(second.isValid);
}

void TestMCPValidator::testValidateResponseFailsWhenSchemaNotLoaded()
{
    MCPValidator validator(QStringLiteral("/nonexistent/path/schema-mcp.json"));
    const QJsonObject response{{"jsonrpc", "2.0"}, {"result", QJsonObject()}, {"id", 1}};

    const ValidationResult result = validator.validateResponse(response);
    QVERIFY(!result.isValid);
    QCOMPARE(result.errorMessage, QStringLiteral("Schema not loaded"));
}

void TestMCPValidator::testValidateResponseRejectsBaseSchemaViolation()
{
    MCPValidator validator(realSchemaPath());
    // CommandResponse requires exactly one of result/error, and both jsonrpc+id.
    const QJsonObject response{{"jsonrpc", "2.0"}, {"id", 1}};

    const ValidationResult result = validator.validateResponse(response);
    QVERIFY(!result.isValid);
    QVERIFY2(result.errorMessage.contains("Base response validation failed"), qPrintable(result.errorMessage));
}

void TestMCPValidator::testValidateResponseAcceptsBaseOnlyWhenNoExpectedCommand()
{
    MCPValidator validator(realSchemaPath());
    const QJsonObject response{{"jsonrpc", "2.0"}, {"result", QJsonObject()}, {"id", 1}};

    const ValidationResult result = validator.validateResponse(response);
    QVERIFY2(result.isValid, qPrintable(result.errorMessage));
    QCOMPARE(result.schemaUsed, QStringLiteral("base_response"));
}

void TestMCPValidator::testValidateResponseUsesSpecificSchemaWhenAvailable()
{
    MCPValidator validator(realSchemaPath());

    // create_element_response requires result.element_id (integer >= 1).
    const QJsonObject validResult{{"element_id", 42}};
    const QJsonObject validResponse{{"jsonrpc", "2.0"}, {"result", validResult}, {"id", 1}};
    const ValidationResult validOutcome = validator.validateResponse(validResponse, "create_element");
    QVERIFY2(validOutcome.isValid, qPrintable(validOutcome.errorMessage));
    QCOMPARE(validOutcome.schemaUsed, QStringLiteral("create_element_response"));

    const QJsonObject invalidResult{{"width", 10}}; // missing required element_id
    const QJsonObject invalidResponse{{"jsonrpc", "2.0"}, {"result", invalidResult}, {"id", 1}};
    const ValidationResult invalidOutcome = validator.validateResponse(invalidResponse, "create_element");
    QVERIFY(!invalidOutcome.isValid);
    QVERIFY2(invalidOutcome.errorMessage.contains("Specific response validation failed"), qPrintable(invalidOutcome.errorMessage));
}

void TestMCPValidator::testValidateResponseQJsonOverloadCatchesMalformedInput()
{
    MCPValidator validator(realSchemaPath());

    const ValidationResult result = validator.validateResponse(objectWithLoneSurrogate());
    QVERIFY(!result.isValid);
    QVERIFY2(result.errorMessage.startsWith("Malformed response:"), qPrintable(result.errorMessage));
}

void TestMCPValidator::testValidateResponseRejectsMissingCommandResponseDefinition()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    const QString path = writeTempSchema(file, kSchemaWithoutCommandResponse);
    MCPValidator validator(path);
    QVERIFY(validator.isSchemaLoaded());

    const QJsonObject response{{"jsonrpc", "2.0"}, {"result", QJsonObject()}, {"id", 1}};
    const ValidationResult result = validator.validateResponse(response);

    QVERIFY(!result.isValid);
    QCOMPARE(result.errorMessage, QStringLiteral("CommandResponse schema not found in definitions"));
}

void TestMCPValidator::testFindCommandSchemaReturnsNullForUnknownCommand()
{
    MCPValidator validator(realSchemaPath());
    QVERIFY(validator.findCommandSchema("totally_made_up_command").is_null());
}

void TestMCPValidator::testFindResponseSchemaReturnsNullForUnknownCommand()
{
    MCPValidator validator(realSchemaPath());
    QVERIFY(validator.findResponseSchema("totally_made_up_command_response").is_null());
}

void TestMCPValidator::testFindCommandSchemaResolvesRef()
{
    // The real schema never puts $ref at a command's own top level (every command schema is
    // fully inlined) -- this exercises the $ref-resolution branch with a purpose-built fixture.
    QTemporaryFile file;
    QVERIFY(file.open());
    const QString path = writeTempSchema(file, kSchemaWithRefs);
    MCPValidator validator(path);
    QVERIFY(validator.isSchemaLoaded());

    const json resolved = validator.findCommandSchema("ref_command");
    QVERIFY(!resolved.is_null());
    QVERIFY(!resolved.contains("$ref"));
    QCOMPARE(QString::fromStdString(resolved.value("type", std::string())), QStringLiteral("object"));
}

void TestMCPValidator::testFindResponseSchemaResolvesRef()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    const QString path = writeTempSchema(file, kSchemaWithRefs);
    MCPValidator validator(path);
    QVERIFY(validator.isSchemaLoaded());

    const json resolved = validator.findResponseSchema("ref_command_response");
    QVERIFY(!resolved.is_null());
    QVERIFY(!resolved.contains("$ref"));
    QCOMPARE(QString::fromStdString(resolved.value("type", std::string())), QStringLiteral("object"));
}

void TestMCPValidator::testQjsonToNlohmannRoundTrip()
{
    const QJsonObject original{{"method", "get_server_info"}, {"id", 7}};
    const json converted = MCPValidator::qjsonToNlohmann(original);

    QCOMPARE(QString::fromStdString(converted["method"].get<std::string>()), QStringLiteral("get_server_info"));
    QCOMPARE(converted["id"].get<int>(), 7);
}

void TestMCPValidator::testNlohmannToQJsonRoundTrip()
{
    json original;
    original["method"] = "get_server_info";
    original["id"] = 7;

    const QJsonObject converted = MCPValidator::nlohmannToQJson(original);
    QCOMPARE(converted.value("method").toString(), QStringLiteral("get_server_info"));
    QCOMPARE(converted.value("id").toInt(), 7);
}

void TestMCPValidator::testNlohmannToQJsonReturnsEmptyOnUnparsableJson()
{
    // A bare top-level scalar dumps to valid JSON text ("42"), but
    // QJsonDocument::fromJson() rejects any non-object/non-array top level ("illegal
    // value") -- confirmed empirically, not assumed from Qt's docs.
    const json bareScalar = 42;
    const QJsonObject converted = MCPValidator::nlohmannToQJson(bareScalar);
    QVERIFY(converted.isEmpty());
}
