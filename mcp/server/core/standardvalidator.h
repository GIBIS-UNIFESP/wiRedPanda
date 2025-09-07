// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../interfaces/ivalidator.h"

/*!
 * @class StandardValidator
 * @brief Standard implementation of IValidator interface
 *
 * Provides common validation patterns and reusable validation logic
 * for all MCP command handlers.
 */
class StandardValidator : public IValidator
{
public:
    StandardValidator();

    // IValidator interface implementation
    bool validateParameters(const QJsonObject &params,
                            const QList<ValidationRule> &rules,
                            QString &errorMessage) const override;

    bool validateRequiredParameters(const QJsonObject &params,
                                    const QStringList &required,
                                    QString &errorMessage) const override;

    bool validatePositiveInteger(const QJsonValue &value,
                                 const QString &paramName,
                                 QString &errorMessage) const override;

    bool validateNonNegativeInteger(const QJsonValue &value,
                                    const QString &paramName,
                                    QString &errorMessage) const override;

    bool validateNonEmptyString(const QJsonValue &value,
                                const QString &paramName,
                                QString &errorMessage) const override;

    bool validateNumeric(const QJsonValue &value,
                         const QString &paramName,
                         QString &errorMessage) const override;

    bool validateElementId(int elementId,
                           const QString &paramName,
                           QString &errorMessage) const override;

    bool validatePortRange(GraphicElement *element,
                           int portIndex,
                           bool isOutput,
                           const QString &paramName,
                           QString &errorMessage) const override;

    bool validateStringEnum(const QJsonValue &value,
                            const QStringList &allowedValues,
                            const QString &paramName,
                            QString &errorMessage) const override;

    bool validateNumericRange(const QJsonValue &value,
                              double minValue,
                              double maxValue,
                              const QString &paramName,
                              QString &errorMessage) const override;

    bool validateFilePath(const QJsonValue &value,
                          bool checkExists,
                          const QString &paramName,
                          QString &errorMessage) const override;

    ValidationRule createPositiveIntegerRule(const QString &paramName,
                                             bool required = true,
                                             const QString &description = QString()) const override;

    ValidationRule createNonEmptyStringRule(const QString &paramName,
                                            bool required = true,
                                            const QString &description = QString()) const override;

    ValidationRule createNumericRule(const QString &paramName,
                                     bool required = true,
                                     const QString &description = QString()) const override;

    // Static instance for global access
    static const StandardValidator& instance();

private:
    static StandardValidator s_instance;
};
