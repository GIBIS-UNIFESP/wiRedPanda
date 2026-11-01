// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::TruthTable> {
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
    static_assert(tableBits == 2048,
                  "The .panda format stores this QBitArray verbatim and the table's spec resizes every loaded "
                  "key to it, so the size is part of the file format. Raising maxInputSize or "
                  "maxOutputSize silently changes what is written and what older files decode to -- "
                  "handle it as a format change, not a constant edit.");

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::TruthTable;
        descriptor.kernelParameters = {ParamId::Table};
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/truthtable-rotated.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("TruthTable", "TRUTH TABLE");
        descriptor.translatedName = QT_TRANSLATE_NOOP("TruthTable", "Truth Table");
        descriptor.trContext = "TruthTable";
        descriptor.appearanceSlots = {{"body", ":/Components/Logic/truthtable-rotated.svg"}};
        // Laid out as [out0_row0..out0_rowN, out1_row0..out1_rowN, ...], all false by default. Its kernel indexes the
        // table up to its last bit and the toggle command flips bits in it, so it holds exactly tableBits whatever
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
        descriptor.inputAttributes = [](const Element &, const int index) {
            // Inputs are labeled A, B, C, ... (alphabetically, matching truth table convention)
            return PortAttributes{.name = QString(QChar::fromLatin1(static_cast<char>('A' + index)))};
        };
        descriptor.outputAttributes = [](const Element &, const int index) {
            // Outputs are labeled S0, S1, ... to distinguish them from input names
            return PortAttributes{.name = "S" + QString::number(index)};
        };
        // The ports take the base layout, columns centred on the 64 px body at a 16 px pitch; the footprint follows
        // them, and the label goes below it (TruthTablePresenter::label())
        descriptor.bodyFollowsPorts = true;
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
