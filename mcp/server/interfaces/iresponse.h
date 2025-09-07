// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QJsonObject>
#include <QString>
#include <QVariant>

/*!
 * @enum ResponseStatus
 * @brief Standardized response status codes
 */
enum class ResponseStatus
{
    Success,                    // Command executed successfully
    InvalidParameters,          // Invalid or missing parameters
    ValidationError,           // Parameter validation failed
    SceneNotAvailable,         // Required scene not available
    ElementNotFound,           // Referenced element doesn't exist
    ConnectionFailed,          // Connection operation failed
    FileError,                 // File I/O error
    SimulationError,           // Simulation-related error
    InternalError,             // Internal processing error
    NotImplemented,            // Feature not yet implemented
    PermissionDenied,          // Operation not permitted
    ResourceUnavailable        // Required resource unavailable
};

/*!
 * @interface IResponse
 * @brief Standardized interface for all MCP command responses
 *
 * Provides consistent response format and error handling across all
 * command handlers in the MCP system.
 */
class IResponse
{
public:
    virtual ~IResponse() = default;

    /*!
     * @brief Get the response status
     * @return ResponseStatus Status code indicating success or failure type
     */
    virtual ResponseStatus getStatus() const = 0;

    /*!
     * @brief Get success flag for JSON response
     * @return bool True if command was successful, false otherwise
     */
    virtual bool isSuccess() const = 0;

    /*!
     * @brief Get error message if command failed
     * @return QString Error message, empty if successful
     */
    virtual QString getErrorMessage() const = 0;

    /*!
     * @brief Get result data from successful command
     * @return QJsonObject Result data, empty if failed or no data
     */
    virtual QJsonObject getResultData() const = 0;

    /*!
     * @brief Get complete JSON response object
     * @return QJsonObject Complete response ready for transmission
     */
    virtual QJsonObject toJsonObject() const = 0;

    /*!
     * @brief Set result data for successful response
     * @param data JSON object containing result data
     */
    virtual void setResultData(const QJsonObject &data) = 0;

    /*!
     * @brief Set error information for failed response
     * @param status Response status code
     * @param message Error message
     */
    virtual void setError(ResponseStatus status, const QString &message) = 0;

    /*!
     * @brief Add metadata to response (e.g., execution time, version)
     * @param key Metadata key
     * @param value Metadata value
     */
    virtual void addMetadata(const QString &key, const QVariant &value) = 0;

    /*!
     * @brief Get metadata value by key
     * @param key Metadata key
     * @return QVariant Metadata value, invalid QVariant if not found
     */
    virtual QVariant getMetadata(const QString &key) const = 0;

    /*!
     * @brief Get response status as human-readable string
     * @return QString Status description
     */
    virtual QString getStatusString() const = 0;
};
