// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "MCP/Server/Handlers/BaseHandler.h"

class CanvasItem;
class QuickAppController;

/**
 * \class QuickBaseHandler
 * \brief CanvasItem-side base for the Quick MCP handler family. Subclasses BaseHandler
 * directly (not a from-scratch copy) -- see this class's .cpp for exactly which of
 * BaseHandler's methods are safe to inherit unmodified vs. which needed a Quick-native
 * reimplementation, and why.
 *
 * \details Constructed with QuickAppController* instead of MainWindow*, passing \c nullptr up
 * to BaseHandler's own \c m_mainWindow (never touched from here -- every method below either
 * inherits unmodified from BaseHandler or is reimplemented against currentCanvas() instead of
 * BaseHandler::currentScene()). Lives in App/QuickShell/CMakeLists.txt's QUICK_SHELL_SOURCES,
 * not wiredpanda_lib's shared CMakeSources.cmake list, since its .cpp needs
 * App/QuickShell/Canvas/CanvasItem.h -- see project memory
 * project_shared_class_quick_extension_pattern.md for why that split matters here too, even
 * though this is a wholly new class rather than an extension of an existing shared one.
 */
class QuickBaseHandler : public BaseHandler
{
public:
    explicit QuickBaseHandler(QuickAppController *appController, const MCPValidator *validator);

    /// CanvasItem-side counterpart of BaseHandler::currentScene(): the current tab's canvas,
    /// or nullptr if there is no app controller or no current tab.
    CanvasItem *currentCanvas();
    /// \overload
    const CanvasItem *currentCanvas() const;

    /// Reimplements BaseHandler::validateElementId() against currentCanvas()->itemById()
    /// instead of BaseHandler::currentScene()->itemById() -- shadows (hides, not overrides;
    /// BaseHandler's version is non-virtual) the base class version so every Quick handler's
    /// unqualified validateElementId(...) call resolves here.
    bool validateElementId(int elementId, const QString &paramName, QString &errorMsg) const;
    /// Reimplements BaseHandler::validatedElement() the same way, and for the same reason:
    /// BaseHandler's own version calls currentScene() internally, which would always resolve
    /// through the (always-null) inherited m_mainWindow if left un-reimplemented here.
    GraphicElement *validatedElement(const QJsonObject &params, const QString &paramName, QString &errorMsg);

protected:
    QuickAppController *m_appController;
};
