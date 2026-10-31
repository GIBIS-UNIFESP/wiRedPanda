// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Parameters: an element's typed values, held against its type's schema (ParamSpec).
 */

#pragma once

#include <array>
#include <functional>
#include <optional>
#include <variant>

#include <QBitArray>
#include <QList>
#include <QString>
#include <QVersionNumber>

class Element;

/// The parameters an element type can have, named the same in every type that has them.
enum class ParamId {
    On,        ///< An input's level: a switch's, or a button's or a clock's, which no file keeps.
    Locked,    ///< Whether an input ignores the user's pointer and keys.
    Position,  ///< The output a rotary switch selects.
    Frequency, ///< A clock's or a buzzer's frequency, in Hz.
    Delay,     ///< A clock's phase delay, as a fraction of its period.
    Volume,    ///< An audio output's volume.
    Colour,    ///< An LED's or a display's colour name.
    AudioFile, ///< The sound file an audio box plays.
    Wireless,  ///< A node's WirelessMode, as an int.
    Table,     ///< A truth table's outputs, a column of rows for each output.
};

/// The number of parameter ids.
inline constexpr int kParamIdCount = static_cast<int>(ParamId::Table) + 1;

/// A parameter's value: one kind per parameter, the kind of its default.
using ParamValue = std::variant<bool, int, double, QString, QBitArray>;

/**
 * \struct ParamSpec
 * \brief One parameter of an element type: its default, the key a file keeps it under, the values it takes, when it
 * is offered for editing and how a file holds it.
 */
struct ParamSpec {
    /// Returns the value \a value, of the parameter's kind, is taken as by \a element, or nothing to leave the
    /// parameter as it is.
    using Accept = std::function<std::optional<ParamValue>(const Element &element, const ParamValue &value)>;

    /// Returns whether \a element offers the parameter for editing.
    using Offered = std::function<bool(const Element &element)>;

    ParamId id = ParamId::On;
    ParamValue defaultValue; ///< The value a new element has; its kind is the parameter's.

    /// The key a file keeps it under; none for a transient parameter, whose change is no change of the circuit.
    const char *key = nullptr;

    Accept accept{};   ///< None: every value of its kind is taken as it is.
    Offered offered{}; ///< None: always offered.

    /// The version from which a file's value means something; an older file's is dropped (the clock's delay before
    /// 4.3). None: every version's.
    QVersionNumber since{};

    /// Whether a portable file keeps the value when it is the default (the truth table's); it omits it otherwise.
    bool keepsDefault = false;

    /// Whether the value is a file's path (the audio box's sound), written as ExternalFilePath::forWriting() has it and
    /// read through forReading(); a portable file omits a resource's, which every element starts with.
    bool fileReference = false;
};

/**
 * \class Parameters
 * \brief The values of an element's parameters, one for each of its type's ParamSpecs, each of its spec's kind.
 * \details It holds the values as they are; Element::setParam() checks them against the specs first. Asking for a
 * parameter the type does not have, or for one as another kind, is a broken invariant: an assertion, and in a release
 * build the kind's empty value.
 */
class Parameters
{
public:
    /// Holds \a schema's defaults; \a schema (a descriptor's) outlives it.
    explicit Parameters(const QList<ParamSpec> &schema);

    /// Returns the spec of \a id, or null when the type has no such parameter.
    const ParamSpec *spec(ParamId id) const;

    /// Returns whether the type has parameter \a id.
    bool has(const ParamId id) const { return spec(id) != nullptr; }

    /// Returns the value of \a id.
    const ParamValue &value(ParamId id) const;

    /// Returns the value of \a id as a \a T, its kind.
    template<typename T>
    const T &get(const ParamId id) const
    {
        if (const T *value = std::get_if<T>(&this->value(id))) {
            return *value;
        }
        Q_ASSERT_X(false, "Parameters::get", "the parameter is of another kind");
        static const T none{};
        return none;
    }

    /// Replaces the value of \a id with \a value, of its kind, and returns the one it had.
    ParamValue set(ParamId id, ParamValue value);

private:
    /// Returns where \a id is in the schema, or -1.
    qsizetype indexOf(const ParamId id) const { return m_index[static_cast<std::size_t>(id)]; }

    const QList<ParamSpec> *m_schema;
    QList<ParamValue> m_values; ///< The value of each spec, in the schema's order.

    /// Where each id is in the schema, or -1: an input's level is read on every simulation tick.
    std::array<qsizetype, kParamIdCount> m_index;
};
