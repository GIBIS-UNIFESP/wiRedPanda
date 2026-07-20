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

    /// Quick-native counterpart of the Scene-based validateElementId() BaseHandler used to
    /// have (removed -- see this class's .cpp), built against currentCanvas()->itemById()
    /// instead of a Scene's.
    bool validateElementId(int elementId, const QString &paramName, QString &errorMsg) const;
    /// Quick-native counterpart of the Scene-based validatedElement() BaseHandler used to have,
    /// for the same reason.
    GraphicElement *validatedElement(const QJsonObject &params, const QString &paramName, QString &errorMsg);

protected:
    QuickAppController *m_appController;
};
