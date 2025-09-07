// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../handlers/basehandler.h"
#include <QHash>
#include <QString>
#include <QStringList>
#include <functional>
#include <memory>

class MainWindow;
class MCPValidator;

/*!
 * @class HandlerFactory
 * @brief Factory pattern implementation for dynamic command handler registration
 *
 * Provides centralized registration and creation of command handlers,
 * enabling dynamic command routing and extensibility.
 */
class HandlerFactory
{
public:
    using HandlerCreator = std::function<std::unique_ptr<BaseHandler>(MainWindow*, MCPValidator*)>;

    /*!
     * @brief Get the singleton instance of HandlerFactory
     * @return HandlerFactory& Reference to singleton instance
     */
    static HandlerFactory& instance();

    /*!
     * @brief Register a command handler creator function
     * @param commandName Command name to register
     * @param category Command category (e.g., "file", "element", "simulation")
     * @param creator Function that creates handler instances
     * @return bool True if registration successful, false if command already exists
     */
    bool registerHandler(const QString &commandName,
                         const QString &category,
                         const HandlerCreator &creator);

    /*!
     * @brief Create a command handler for the specified command
     * @param commandName Command name to create handler for
     * @param mainWindow Main window instance
     * @param validator Validator instance
     * @return std::unique_ptr<BaseHandler> Handler instance, null if command not found
     */
    std::unique_ptr<BaseHandler> createHandler(const QString &commandName,
                                               MainWindow *mainWindow,
                                               MCPValidator *validator) const;

    /*!
     * @brief Check if a command is registered
     * @param commandName Command name to check
     * @return bool True if command is registered
     */
    bool isCommandRegistered(const QString &commandName) const;

    /*!
     * @brief Get list of all registered commands
     * @return QStringList List of command names
     */
    QStringList getRegisteredCommands() const;

    /*!
     * @brief Get commands by category
     * @param category Category to filter by
     * @return QStringList List of command names in the category
     */
    QStringList getCommandsByCategory(const QString &category) const;

    /*!
     * @brief Get all available categories
     * @return QStringList List of category names
     */
    QStringList getCategories() const;

    /*!
     * @brief Get command category
     * @param commandName Command name
     * @return QString Category name, empty if command not found
     */
    QString getCommandCategory(const QString &commandName) const;

    /*!
     * @brief Unregister a command handler
     * @param commandName Command name to unregister
     * @return bool True if command was registered and removed
     */
    bool unregisterHandler(const QString &commandName);

    /*!
     * @brief Clear all registered handlers
     */
    void clear();

    /*!
     * @brief Get number of registered commands
     * @return int Number of registered commands
     */
    int getRegisteredCommandCount() const;

    /*!
     * @brief Register all built-in handlers
     * @param mainWindow Main window instance for handler creation
     * @param validator Validator instance for handler creation
     */
    void registerBuiltInHandlers();

private:
    HandlerFactory() = default;
    ~HandlerFactory() = default;
    HandlerFactory(const HandlerFactory&) = delete;
    HandlerFactory& operator=(const HandlerFactory&) = delete;

    struct HandlerInfo {
        QString category;
        HandlerCreator creator;
    };

    QHash<QString, HandlerInfo> m_handlers;
    static HandlerFactory s_instance;
};
