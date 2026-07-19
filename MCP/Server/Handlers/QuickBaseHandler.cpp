// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MCP/Server/Handlers/QuickBaseHandler.h"

#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickAppController.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"

// BaseHandler's methods split cleanly into two groups (confirmed by reading every line of
// BaseHandler.cpp, not assumed): createErrorResponse()/createSuccessResponse()/tryCommand()/
// validateParameters()/validateNonEmptyString()/validateNonNegativeInteger()/validateNumeric()/
// validatePositiveInteger()/validatePortRange()/inputPortByLabel()/outputPortByLabel()/
// availablePorts()/availableInputPorts()/availableOutputPorts() operate only on
// GraphicElement*/Port*/QJsonObject/QJsonValue -- zero Scene/MainWindow coupling anywhere in
// their bodies, so they're inherited here completely unmodified. Only validateElementId() and
// validatedElement() touch currentScene() (directly, or via a call BaseHandler's own body binds
// statically to its own currentScene()/validateElementId() -- non-virtual functions resolve at
// their *defining* class's scope, not the runtime object's most-derived type, so shadowing
// validateElementId() alone would NOT fix validatedElement(), which needed its own
// reimplementation too) -- those two are reimplemented below against currentCanvas().

QuickBaseHandler::QuickBaseHandler(QuickAppController *appController, const MCPValidator *validator)
    : BaseHandler(nullptr, validator)
    , m_appController(appController)
{
}

CanvasItem *QuickBaseHandler::currentCanvas()
{
    if (!m_appController) {
        return nullptr;
    }
    QuickWorkSpace *tab = m_appController->currentTab();
    if (!tab) {
        return nullptr;
    }
    return tab->canvas();
}

const CanvasItem *QuickBaseHandler::currentCanvas() const
{
    if (!m_appController) {
        return nullptr;
    }
    const QuickWorkSpace *tab = m_appController->currentTab();
    if (!tab) {
        return nullptr;
    }
    return tab->canvas();
}

bool QuickBaseHandler::validateElementId(int elementId, const QString &paramName, QString &errorMsg) const
{
    if (elementId <= 0) {
        errorMsg = QString("Parameter '%1' must be a positive integer (got %2)").arg(paramName).arg(elementId);
        return false;
    }

    const CanvasItem *canvas = currentCanvas();
    if (!canvas) {
        errorMsg = "No active circuit scene available";
        return false;
    }

    auto *item = canvas->itemById(elementId);
    if (!item) {
        errorMsg = QString("Element not found: %1").arg(elementId);
        return false;
    }

    return true;
}

GraphicElement *QuickBaseHandler::validatedElement(const QJsonObject &params, const QString &paramName, QString &errorMsg)
{
    if (!validatePositiveInteger(params.value(paramName), paramName, errorMsg)) {
        return nullptr;
    }
    const int elementId = params.value(paramName).toInt();
    if (!validateElementId(elementId, paramName, errorMsg)) {
        return nullptr;
    }
    auto *item = currentCanvas()->itemById(elementId);
    auto *element = dynamic_cast<GraphicElement *>(item);
    if (!element) {
        errorMsg = QString("Item %1 is not a graphic element").arg(elementId);
        return nullptr;
    }
    return element;
}
