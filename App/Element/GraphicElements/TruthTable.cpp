// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/TruthTable.h"

#include <QGraphicsSceneMouseEvent>

#include "App/Core/Constants.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<TruthTable> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::TruthTable,
        .group = ElementGroup::IC,
        .minInputSize = 2,
        .maxInputSize = 8,
        .minOutputSize = 1,
        .maxOutputSize = 8,
        .hasLabel = true,
    };
    static_assert(validate(constraints));

    /// Bits in the table: one per (output column, row), a row for each input pattern of the most inputs.
    static constexpr qsizetype tableBits = (qsizetype{1} << constraints.maxInputSize) * constraints.maxOutputSize;

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/truthtable-rotated.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("TruthTable", "TRUTH TABLE");
        descriptor.translatedName = QT_TRANSLATE_NOOP("TruthTable", "Truth Table");
        descriptor.trContext = "TruthTable";
        descriptor.defaultAppearances = QStringList({":/Components/Logic/truthtable-rotated.svg"});
        // Laid out as [out0_row0..out0_rowN, out1_row0..out1_rowN, ...], all false by default. updateLogic() indexes
        // the table up to its last bit and the toggle command flips bits in it, so it holds exactly tableBits whatever
        // a (possibly corrupt or crafted) file supplied: padded with zeros or truncated
        descriptor.parameters = {{
            .id = ParamId::Table,
            .defaultValue = QBitArray(tableBits),
            .key = "key",
            .accept = [](const Element &, const ParamValue &value) -> std::optional<ParamValue> {
                QBitArray bits = std::get<QBitArray>(value);
                bits.resize(tableBits);
                return bits;
            },
            // A portable file keeps the table even when it is all false
            .keepsDefault = true,
        }};
        // A truth table's map came with its bits, in 4.2 (VersionInfo::hasTruthTableData()); an older file holds
        // nothing
        descriptor.mapSince = QVersionNumber(4, 2);
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new TruthTable(); });
        return true;
    }();
};

namespace {

/// Rows in a full table: the row index is the inputs read MSB-first, so an element wired to the
/// maximum number of inputs addresses 2^maxInputSize of them.
constexpr qsizetype kTruthTableRows = qsizetype{1} << ElementInfo<TruthTable>::constraints.maxInputSize;

/// Bits in the table: one per (output column, row). updateLogic() indexes kTruthTableRows * output
/// + row, so the largest index it can form is exactly this minus one -- the array has ZERO
/// slack, and derived constants are what keep that true if the port limits ever move.
constexpr qsizetype kTruthTableKeyBits = ElementInfo<TruthTable>::tableBits;

static_assert(kTruthTableKeyBits == 2048,
              "The .panda format stores this QBitArray verbatim and the table's spec resizes every loaded "
              "key to it, so the size is part of the file format. Raising maxInputSize or "
              "maxOutputSize silently changes what is written and what older files decode to -- "
              "handle it as a format change, not a constant edit.");

} // namespace

TruthTable::TruthTable(QGraphicsItem *parent)
    : Element(ElementType::TruthTable, parent)
{
    updatePortsProperties();
}

void TruthTable::layoutPorts()
{
    // The ports take the base layout: columns centred on the 64 px body, at a 16 px pitch (every other grid line)
    Element::layoutPorts();

    // The footprint follows the ports, and the label goes below it (TruthTablePresenter::label())
    pictureReplaced();
}

std::optional<Element::PortAttributes> TruthTable::inputAttributes(const int index) const
{
    // Inputs are labeled A, B, C, ... (alphabetically, matching truth table convention)
    return PortAttributes{.name = QString(QChar::fromLatin1(static_cast<char>('A' + index)))};
}

std::optional<Element::PortAttributes> TruthTable::outputAttributes(const int index) const
{
    // Outputs are labeled S0, S1, ... to distinguish them from input names
    return PortAttributes{.name = "S" + QString::number(index)};
}

void TruthTable::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    emit requestOpenTruthTableEditor();
}

const QBitArray &TruthTable::key() const
{
    return param<QBitArray>(ParamId::Table);
}

void TruthTable::setkey(const QBitArray &key)
{
    setParam(ParamId::Table, key);
}

void TruthTable::setKeyBit(const int index, const bool value)
{
    Q_ASSERT((index >= 0) && (index < key().size()));
    QBitArray table = key();
    table.setBit(index, value);
    setkey(table);
}

void TruthTable::updateLogic()
{
    if (!simUpdateInputsAllowUnknown()) {
        return;
    }

    // If any input is Unknown/Error, the row cannot be determined
    for (const auto s : simInputs()) {
        if (s == Status::Unknown || s == Status::Error) {
            for (int i = 0; i < outputSize(); ++i) {
                setOutputValue(i, s);
            }
            return;
        }
    }

    // The row index is the inputs read MSB-first (input 0 is the most
    // significant bit) — computed once with integer ops instead of building
    // and re-parsing a binary QString per output per tick.
    quint32 pos = 0;
    for (const auto s : simInputs()) {
        pos = (pos << 1) | ((s == Status::Active) ? 1U : 0U);
    }

    const QBitArray &table = key();
    for (int i = 0; i < outputSize(); ++i) {
        const bool result = table.at(kTruthTableRows * i + static_cast<qsizetype>(pos));
        setOutputValue(i, result);
    }
}
