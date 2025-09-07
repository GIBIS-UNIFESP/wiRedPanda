// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "standardvalidator.h"
#include "elementfactory.h"
#include "graphicelement.h"

#include <QFile>
#include <QFileInfo>
#include <cmath>

StandardValidator StandardValidator::s_instance;

StandardValidator::StandardValidator()
{
}

bool StandardValidator::validateParameters(const QJsonObject &params,
                                           const QList<ValidationRule> &rules,
                                           QString &errorMessage) const
{
    for (const auto &rule : rules) {
        if (rule.required && !params.contains(rule.parameterName)) {
            errorMessage = QString("Missing required parameter: %1").arg(rule.parameterName);
            return false;
        }

        if (params.contains(rule.parameterName)) {
            const QJsonValue value = params.value(rule.parameterName);
            if (rule.validator && !rule.validator(value, errorMessage)) {
                return false;
            }
        }
    }
    return true;
}

bool StandardValidator::validateRequiredParameters(const QJsonObject &params,
                                                   const QStringList &required,
                                                   QString &errorMessage) const
{
    for (const QString &param : required) {
        if (!params.contains(param)) {
            errorMessage = QString("Missing required parameter: %1").arg(param);
            return false;
        }
    }
    return true;
}

bool StandardValidator::validatePositiveInteger(const QJsonValue &value,
                                                const QString &paramName,
                                                QString &errorMessage) const
{
    if (!value.isDouble()) {
        errorMessage = QString("Parameter '%1' must be an integer").arg(paramName);
        return false;
    }

    int intVal = value.toInt();
    if (intVal <= 0) {
        errorMessage = QString("Parameter '%1' must be a positive integer (got %2)").arg(paramName).arg(intVal);
        return false;
    }

    return true;
}

bool StandardValidator::validateNonNegativeInteger(const QJsonValue &value,
                                                   const QString &paramName,
                                                   QString &errorMessage) const
{
    if (!value.isDouble()) {
        errorMessage = QString("Parameter '%1' must be an integer").arg(paramName);
        return false;
    }

    int intVal = value.toInt();
    if (intVal < 0) {
        errorMessage = QString("Parameter '%1' must be non-negative (got %2)").arg(paramName).arg(intVal);
        return false;
    }

    return true;
}

bool StandardValidator::validateNonEmptyString(const QJsonValue &value,
                                               const QString &paramName,
                                               QString &errorMessage) const
{
    if (!value.isString()) {
        errorMessage = QString("Parameter '%1' must be a string").arg(paramName);
        return false;
    }

    QString str = value.toString();
    if (str.isEmpty()) {
        errorMessage = QString("Parameter '%1' cannot be empty").arg(paramName);
        return false;
    }

    return true;
}

bool StandardValidator::validateNumeric(const QJsonValue &value,
                                        const QString &paramName,
                                        QString &errorMessage) const
{
    if (!value.isDouble()) {
        errorMessage = QString("Parameter '%1' must be a numeric value").arg(paramName);
        return false;
    }

    double numVal = value.toDouble();
    if (std::isnan(numVal) || std::isinf(numVal)) {
        errorMessage = QString("Parameter '%1' must be a finite numeric value").arg(paramName);
        return false;
    }

    return true;
}

bool StandardValidator::validateElementId(int elementId,
                                          const QString &paramName,
                                          QString &errorMessage) const
{
    if (elementId <= 0) {
        errorMessage = QString("Parameter '%1' must be a positive integer (got %2)").arg(paramName).arg(elementId);
        return false;
    }

    auto *item = ElementFactory::itemById(elementId);
    if (!item) {
        errorMessage = QString("Element not found: %1").arg(elementId);
        return false;
    }

    return true;
}

bool StandardValidator::validatePortRange(GraphicElement *element,
                                          int portIndex,
                                          bool isOutput,
                                          const QString &paramName,
                                          QString &errorMessage) const
{
    if (!element) {
        errorMessage = QString("Invalid element for port validation");
        return false;
    }

    int maxPorts = 0;
    if (isOutput) {
        maxPorts = element->outputs().size();
    } else {
        maxPorts = element->inputs().size();
    }

    if (portIndex >= maxPorts) {
        QString portType = isOutput ? "output" : "input";
        errorMessage = QString("Parameter '%1' port index %2 is out of range (element has %3 %4 ports)")
                       .arg(paramName)
                       .arg(portIndex)
                       .arg(maxPorts)
                       .arg(portType);
        return false;
    }

    return true;
}

bool StandardValidator::validateStringEnum(const QJsonValue &value,
                                           const QStringList &allowedValues,
                                           const QString &paramName,
                                           QString &errorMessage) const
{
    if (!value.isString()) {
        errorMessage = QString("Parameter '%1' must be a string").arg(paramName);
        return false;
    }

    QString str = value.toString();
    if (!allowedValues.contains(str)) {
        errorMessage = QString("Parameter '%1' must be one of: %2 (got '%3')")
                       .arg(paramName)
                       .arg(allowedValues.join(", "))
                       .arg(str);
        return false;
    }

    return true;
}

bool StandardValidator::validateNumericRange(const QJsonValue &value,
                                             double minValue,
                                             double maxValue,
                                             const QString &paramName,
                                             QString &errorMessage) const
{
    if (!validateNumeric(value, paramName, errorMessage)) {
        return false;
    }

    double numVal = value.toDouble();
    if (numVal < minValue || numVal > maxValue) {
        errorMessage = QString("Parameter '%1' must be between %2 and %3 (got %4)")
                       .arg(paramName)
                       .arg(minValue)
                       .arg(maxValue)
                       .arg(numVal);
        return false;
    }

    return true;
}

bool StandardValidator::validateFilePath(const QJsonValue &value,
                                         bool checkExists,
                                         const QString &paramName,
                                         QString &errorMessage) const
{
    if (!validateNonEmptyString(value, paramName, errorMessage)) {
        return false;
    }

    QString filePath = value.toString();

    if (checkExists) {
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            errorMessage = QString("File not found: %1").arg(filePath);
            return false;
        }

        if (!fileInfo.isReadable()) {
            errorMessage = QString("File is not readable: %1").arg(filePath);
            return false;
        }
    }

    return true;
}

ValidationRule StandardValidator::createPositiveIntegerRule(const QString &paramName,
                                                            bool required,
                                                            const QString &description) const
{
    auto validator = [this](const QJsonValue &value, QString &errorMsg) {
        return validatePositiveInteger(value, "", errorMsg);
    };

    return ValidationRule(paramName, required, validator,
                         description.isEmpty() ? QString("Positive integer value") : description);
}

ValidationRule StandardValidator::createNonEmptyStringRule(const QString &paramName,
                                                           bool required,
                                                           const QString &description) const
{
    auto validator = [this](const QJsonValue &value, QString &errorMsg) {
        return validateNonEmptyString(value, "", errorMsg);
    };

    return ValidationRule(paramName, required, validator,
                         description.isEmpty() ? QString("Non-empty string value") : description);
}

ValidationRule StandardValidator::createNumericRule(const QString &paramName,
                                                    bool required,
                                                    const QString &description) const
{
    auto validator = [this](const QJsonValue &value, QString &errorMsg) {
        return validateNumeric(value, "", errorMsg);
    };

    return ValidationRule(paramName, required, validator,
                         description.isEmpty() ? QString("Numeric value") : description);
}

const StandardValidator& StandardValidator::instance()
{
    return s_instance;
}
