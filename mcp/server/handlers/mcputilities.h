// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "basehandler.h"

/*!
 * @class MCPUtilities
 * @brief Concrete utility handler for common MCP validation functions
 *
 * Provides access to BaseHandler utility methods for classes that cannot
 * inherit from BaseHandler (like MCPProcessor which inherits from QObject).
 */
class MCPUtilities : public BaseHandler
{
public:
    explicit MCPUtilities(MainWindow *mainWindow, MCPValidator *validator);

    // Required implementation of pure virtual method (unused for utilities)
    QJsonObject handleCommand(const QString &command, const QJsonObject &params, const QJsonValue &requestId) override;
};
