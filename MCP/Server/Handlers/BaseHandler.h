// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QJsonObject>
#include <QString>

class MainWindow;
class MCPValidator;
class Scene;
class GraphicElement;

/**
 * \class BaseHandler
 * \brief Abstract base class for MCP command handlers
 *
 * Provides common functionality and interface for all MCP command handlers.
 * Each handler is responsible for a specific domain of MCP commands.
 */
class BaseHandler
{
public:
    explicit BaseHandler(MainWindow *mainWindow, MCPValidator *validator);
    virtual ~BaseHandler() = default;

    // Pure virtual method for handling commands - each handler implements this
    virtual QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) = 0;

    // Public utility methods available to all classes
    QJsonObject createErrorResponse(const QString &error, const QJsonValue &requestId = QJsonValue());
    QJsonObject createSuccessResponse(const QJsonObject &result = {}, const QJsonValue &requestId = QJsonValue());
    Scene *getCurrentScene();
    bool validateAndSendResponse(const QJsonObject &response, const QString &command = QString());
    bool validateElementId(int elementId, const QString &paramName, QString &errorMsg);
    bool validateNonEmptyString(const QJsonValue &value, const QString &paramName, QString &errorMsg);
    bool validateNonNegativeInteger(const QJsonValue &value, const QString &paramName, QString &errorMsg);
    bool validateNumeric(const QJsonValue &value, const QString &paramName, QString &errorMsg);
    bool validateParameters(const QJsonObject &params, const QStringList &required);
    bool validatePortRange(GraphicElement *element, int portIndex, bool isOutput, const QString &paramName, QString &errorMsg);
    bool validatePositiveInteger(const QJsonValue &value, const QString &paramName, QString &errorMsg);
    void sendResponse(const QJsonObject &response);

    /**
     * \brief Wraps \a fn in a try/catch, returning an error response on exception.
     * \details Calls \a fn() inside a try block. On `std::exception` or unknown exception,
     * returns `createErrorResponse("Failed to \<action\>: ...")`. Use this to eliminate
     * repetitive try/catch boilerplate in handler methods.
     */
    template<typename Fn>
    QJsonObject tryCommand(Fn &&fn, const QString &action, const QJsonValue &requestId = QJsonValue())
    {
        try {
            return std::forward<Fn>(fn)();
        } catch (const std::exception &e) {
            return createErrorResponse(QString("Failed to %1: %2").arg(action, e.what()), requestId);
        } catch (...) {
            return createErrorResponse(QString("Failed to %1: Unknown exception").arg(action), requestId);
        }
    }

    /**
     * \brief Validates \a paramName in \a params, looks up the element, and returns it.
     * \details Combines validatePositiveInteger() + validateElementId() + itemById() +
     * dynamic_cast in one call. On failure sets \a errorMsg and returns nullptr.
     * Typical use: `auto *e = getValidatedElement(params, "element_id", errorMsg); if (!e) return createErrorResponse(errorMsg, requestId);`
     */
    GraphicElement *getValidatedElement(const QJsonObject &params, const QString &paramName, QString &errorMsg);

    // Port lookup by label (similar to CircuitBuilder in testutils.cpp)
    bool getInputPortByLabel(GraphicElement *element, const QString &label, int &portIndex, QString &errorMsg);
    bool getOutputPortByLabel(GraphicElement *element, const QString &label, int &portIndex, QString &errorMsg);
    QString getAvailableInputPorts(GraphicElement *element);
    QString getAvailableOutputPorts(GraphicElement *element);

protected:
    // Shared resources
    MainWindow *m_mainWindow;
    MCPValidator *m_validator;
};

