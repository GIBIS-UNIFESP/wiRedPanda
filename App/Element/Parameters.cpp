// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Parameters.h"

#include <utility>

#include "App/Core/Common.h"

Parameters::Parameters(const QList<ParamSpec> &schema)
    : m_schema(&schema)
{
    m_index.fill(-1);
    m_values.reserve(schema.size());
    for (const ParamSpec &spec : schema) {
        Q_ASSERT_X(m_index[static_cast<std::size_t>(spec.id)] < 0, "Parameters", "a parameter twice in a schema");
        m_index[static_cast<std::size_t>(spec.id)] = m_values.size();
        m_values.append(spec.defaultValue);
    }
}

const ParamSpec *Parameters::spec(const ParamId id) const
{
    const qsizetype index = indexOf(id);
    return (index < 0) ? nullptr : &m_schema->at(index);
}

const ParamValue &Parameters::value(const ParamId id) const
{
    const qsizetype index = indexOf(id);
    if (index < 0) {
        Q_ASSERT_X(false, "Parameters::value", "no such parameter in this element type");
        qCWarning(zero) << "No parameter" << static_cast<int>(id) << "in this element type";
        static const ParamValue none;
        return none;
    }
    return m_values.at(index);
}

ParamValue Parameters::set(const ParamId id, ParamValue value)
{
    const qsizetype index = indexOf(id);
    if ((index < 0) || (value.index() != m_schema->at(index).defaultValue.index())) {
        Q_ASSERT_X(false, "Parameters::set", "no such parameter in this element type, or a value of another kind");
        qCWarning(zero) << "Parameter" << static_cast<int>(id) << "not set: none in this element type, or of another kind";
        return value;
    }
    return std::exchange(m_values[index], std::move(value));
}
