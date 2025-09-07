// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>

/*!
 * @interface ICommand
 * @brief Standardized interface for all MCP command operations
 *
 * Defines the contract that all command handlers must implement to ensure
 * consistent validation, execution, and error handling across the MCP system.
 */
class ICommand
{
public:
    virtual ~ICommand() = default;

    /*!
     * @brief Execute the command with the given parameters
     * @param params JSON object containing command parameters
     * @return QJsonObject Response with success/error status and results
     */
    virtual QJsonObject execute(const QJsonObject &params) = 0;

    /*!
     * @brief Get the command name/identifier
     * @return QString Command name this handler processes
     */
    virtual QString getCommandName() const = 0;

    /*!
     * @brief Get list of required parameters for this command
     * @return QStringList List of required parameter names
     */
    virtual QStringList getRequiredParameters() const = 0;

    /*!
     * @brief Get command description for documentation/help
     * @return QString Human-readable description of what this command does
     */
    virtual QString getDescription() const = 0;

    /*!
     * @brief Get command category/domain for organization
     * @return QString Category this command belongs to (e.g., "simulation", "elements")
     */
    virtual QString getCategory() const = 0;

    /*!
     * @brief Validate command parameters before execution
     * @param params JSON object containing command parameters
     * @param errorMessage Reference to string that will contain error details if validation fails
     * @return bool True if parameters are valid, false otherwise
     */
    virtual bool validateParameters(const QJsonObject &params, QString &errorMessage) const = 0;

    /*!
     * @brief Check if this command requires an active circuit scene
     * @return bool True if command needs active scene, false otherwise
     */
    virtual bool requiresActiveScene() const = 0;

    /*!
     * @brief Get example usage for this command
     * @return QJsonObject Example command with parameters for documentation
     */
    virtual QJsonObject getExampleUsage() const = 0;
};
