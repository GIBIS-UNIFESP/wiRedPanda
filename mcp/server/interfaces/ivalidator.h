// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QStringList>
#include <functional>

class GraphicElement;

/*!
 * @struct ValidationRule
 * @brief Represents a single parameter validation rule
 */
struct ValidationRule
{
    QString parameterName;                                     // Name of parameter to validate
    bool required = true;                                      // Whether parameter is required
    std::function<bool(const QJsonValue&, QString&)> validator; // Validation function
    QString description;                                       // Human-readable description

    ValidationRule() = default;
    ValidationRule(const QString &name, bool req,
                   std::function<bool(const QJsonValue&, QString&)> val,
                   const QString &desc = QString())
        : parameterName(name), required(req), validator(val), description(desc) {}
};

/*!
 * @interface IValidator
 * @brief Standardized interface for parameter validation
 *
 * Provides common validation patterns and reusable validation logic
 * for all MCP command handlers.
 */
class IValidator
{
public:
    virtual ~IValidator() = default;

    /*!
     * @brief Validate parameters against a set of rules
     * @param params JSON object containing parameters to validate
     * @param rules List of validation rules to apply
     * @param errorMessage Reference to string that will contain error details if validation fails
     * @return bool True if all validations pass, false otherwise
     */
    virtual bool validateParameters(const QJsonObject &params,
                                    const QList<ValidationRule> &rules,
                                    QString &errorMessage) const = 0;

    /*!
     * @brief Validate that required parameters are present
     * @param params JSON object containing parameters
     * @param required List of required parameter names
     * @param errorMessage Reference to string for error details
     * @return bool True if all required parameters are present
     */
    virtual bool validateRequiredParameters(const QJsonObject &params,
                                            const QStringList &required,
                                            QString &errorMessage) const = 0;

    // Common validation methods for standard parameter types

    /*!
     * @brief Validate positive integer parameter
     * @param value JSON value to validate
     * @param paramName Parameter name for error messages
     * @param errorMessage Reference to string for error details
     * @return bool True if value is a positive integer
     */
    virtual bool validatePositiveInteger(const QJsonValue &value,
                                         const QString &paramName,
                                         QString &errorMessage) const = 0;

    /*!
     * @brief Validate non-negative integer parameter
     * @param value JSON value to validate
     * @param paramName Parameter name for error messages
     * @param errorMessage Reference to string for error details
     * @return bool True if value is a non-negative integer
     */
    virtual bool validateNonNegativeInteger(const QJsonValue &value,
                                            const QString &paramName,
                                            QString &errorMessage) const = 0;

    /*!
     * @brief Validate non-empty string parameter
     * @param value JSON value to validate
     * @param paramName Parameter name for error messages
     * @param errorMessage Reference to string for error details
     * @return bool True if value is a non-empty string
     */
    virtual bool validateNonEmptyString(const QJsonValue &value,
                                        const QString &paramName,
                                        QString &errorMessage) const = 0;

    /*!
     * @brief Validate numeric parameter (int or double)
     * @param value JSON value to validate
     * @param paramName Parameter name for error messages
     * @param errorMessage Reference to string for error details
     * @return bool True if value is numeric and finite
     */
    virtual bool validateNumeric(const QJsonValue &value,
                                 const QString &paramName,
                                 QString &errorMessage) const = 0;

    /*!
     * @brief Validate element ID and check if element exists
     * @param elementId Element ID to validate
     * @param paramName Parameter name for error messages
     * @param errorMessage Reference to string for error details
     * @return bool True if element ID is valid and element exists
     */
    virtual bool validateElementId(int elementId,
                                   const QString &paramName,
                                   QString &errorMessage) const = 0;

    /*!
     * @brief Validate port range for an element
     * @param element Element to check port range for
     * @param portIndex Port index to validate
     * @param isOutput True for output port, false for input port
     * @param paramName Parameter name for error messages
     * @param errorMessage Reference to string for error details
     * @return bool True if port index is valid for the element
     */
    virtual bool validatePortRange(GraphicElement *element,
                                   int portIndex,
                                   bool isOutput,
                                   const QString &paramName,
                                   QString &errorMessage) const = 0;

    /*!
     * @brief Validate string value against allowed values
     * @param value JSON value to validate
     * @param allowedValues List of allowed string values
     * @param paramName Parameter name for error messages
     * @param errorMessage Reference to string for error details
     * @return bool True if value is in allowed values list
     */
    virtual bool validateStringEnum(const QJsonValue &value,
                                    const QStringList &allowedValues,
                                    const QString &paramName,
                                    QString &errorMessage) const = 0;

    /*!
     * @brief Validate numeric range
     * @param value JSON value to validate
     * @param minValue Minimum allowed value (inclusive)
     * @param maxValue Maximum allowed value (inclusive)
     * @param paramName Parameter name for error messages
     * @param errorMessage Reference to string for error details
     * @return bool True if value is within range
     */
    virtual bool validateNumericRange(const QJsonValue &value,
                                      double minValue,
                                      double maxValue,
                                      const QString &paramName,
                                      QString &errorMessage) const = 0;

    /*!
     * @brief Validate file path and check if file exists
     * @param value JSON value containing file path
     * @param checkExists Whether to verify file actually exists
     * @param paramName Parameter name for error messages
     * @param errorMessage Reference to string for error details
     * @return bool True if file path is valid
     */
    virtual bool validateFilePath(const QJsonValue &value,
                                  bool checkExists,
                                  const QString &paramName,
                                  QString &errorMessage) const = 0;

    /*!
     * @brief Create a ValidationRule for positive integer
     * @param paramName Parameter name
     * @param required Whether parameter is required
     * @param description Optional description
     * @return ValidationRule Configured validation rule
     */
    virtual ValidationRule createPositiveIntegerRule(const QString &paramName,
                                                     bool required = true,
                                                     const QString &description = QString()) const = 0;

    /*!
     * @brief Create a ValidationRule for non-empty string
     * @param paramName Parameter name
     * @param required Whether parameter is required
     * @param description Optional description
     * @return ValidationRule Configured validation rule
     */
    virtual ValidationRule createNonEmptyStringRule(const QString &paramName,
                                                    bool required = true,
                                                    const QString &description = QString()) const = 0;

    /*!
     * @brief Create a ValidationRule for numeric values
     * @param paramName Parameter name
     * @param required Whether parameter is required
     * @param description Optional description
     * @return ValidationRule Configured validation rule
     */
    virtual ValidationRule createNumericRule(const QString &paramName,
                                             bool required = true,
                                             const QString &description = QString()) const = 0;
};
