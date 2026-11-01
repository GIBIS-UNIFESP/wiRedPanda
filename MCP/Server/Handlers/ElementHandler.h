// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QList>

#include "App/Scene/Commands.h"
#include "MCP/Server/Handlers/BaseHandler.h"

/**
 * \class ElementHandler
 * \brief Handler for element operation commands (create, delete, modify, transform, etc.)
 *
 * Handles all element-related MCP commands including creating elements,
 * deleting elements, listing elements, moving elements, setting properties,
 * rotating, flipping, morphing elements, and changing port sizes.
 */
class ElementHandler : public BaseHandler
{
public:
    explicit ElementHandler(MainWindow *mainWindow, const MCPValidator *validator);

    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;

    /// A property MCP reads and writes through the element, under its key in schema-mcp.json.
    struct KeyedProperty {
        const char *key;
        ElementProperty property;
    };

    /// The properties list_elements reports and set_element_properties sets, each where it applies to the element
    /// (SetPropertyCommand::appliesTo()): the label to every element, a parameter where the element offers it.
    static const QList<KeyedProperty> &keyedProperties();

private:
    QJsonObject handleCreateElement(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleDeleteElement(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleListElements(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleMoveElement(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleSetElementProperties(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleSetInputValue(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleGetOutputValue(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleRotateElement(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleFlipElement(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleUpdateElement(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleChangeInputSize(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleChangeOutputSize(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleToggleTruthTableOutput(const QJsonObject &params, const QJsonValue &requestId);
    QJsonObject handleMorphElement(const QJsonObject &params, const QJsonValue &requestId);
};
