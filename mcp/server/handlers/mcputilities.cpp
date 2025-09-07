// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mcputilities.h"

MCPUtilities::MCPUtilities(MainWindow *mainWindow, MCPValidator *validator)
    : BaseHandler(mainWindow, validator)
{
}

QJsonObject MCPUtilities::handleCommand(const QString &, const QJsonObject &, const QJsonValue &requestId)
{
    // This method is not used - MCPUtilities is only for accessing validation methods
    return createErrorResponse("MCPUtilities::handleCommand should not be called", requestId);
}
