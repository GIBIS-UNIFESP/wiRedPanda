// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/SelectionCapabilities.h"

#include <variant>

#include "App/Core/Enums.h"
#include "App/Element/Element.h"

namespace SelectionCapabilitiesDetail {

/// Returns \c true if \a element and \a other agree on parameter \a id: both lack it, or both have it at one value,
/// numbers compared within rounding (qFuzzyCompare()).
bool sameParam(const Element &element, const Element &other, const ParamId id)
{
    if (element.hasParam(id) != other.hasParam(id)) {
        return false;
    }
    if (!element.hasParam(id)) {
        return true;
    }
    const auto &value = element.parameters().value(id);
    const auto &otherValue = other.parameters().value(id);
    const auto *number = std::get_if<double>(&value);
    const auto *otherNumber = std::get_if<double>(&otherValue);
    if (number && otherNumber) {
        return qFuzzyCompare(*number, *otherNumber);
    }
    return value == otherValue;
}

} // namespace SelectionCapabilitiesDetail

using SelectionCapabilitiesDetail::sameParam;

SelectionCapabilities computeCapabilities(const QList<Element *> &elements)
{
    SelectionCapabilities c;

    if (elements.isEmpty()) {
        return c;
    }

    auto *firstElement = elements.constFirst();

    // Start all flags true; AND-reduce over the selection below.
    c.hasAudioBox = c.hasColors = c.hasDelay = c.hasElements = true;
    c.hasFrequency = c.hasLabel = c.hasOnlyInputs = c.hasLatchedValue = c.hasWirelessMode = c.hasTrigger = c.hasTruthTable = c.hasVolume = true;
    c.canChangeAppearance = c.canMorph = true;
    c.isFileBacked = c.isEmbedded = (firstElement->elementType() == ElementType::IC);
    c.hasSameColors = c.hasSameDelay = c.hasSameFrequency = c.hasSameVolume = true;
    c.hasSameInputSize = c.hasSameLabel = c.hasSameOutputSize = true;
    c.hasSameOutputValue = c.hasSameTrigger = c.hasSameType = true;
    c.sameCheckState = true;
    // Start at a very high "infinity" so std::min finds the real maximum.
    c.maximumInputs = c.maximumOutputs = c.maxCurrentOutputSize = 100000000;
    c.elementType = firstElement->elementType();

    for (auto *elm : elements) {
        const auto group      = elm->elementGroup();
        const auto firstGroup = firstElement->elementGroup();

        c.hasTruthTable &= elm->hasTruthTable();
        c.hasLabel      &= elm->hasLabel();
        c.canChangeAppearance &= elm->canChangeAppearance();
        c.hasColors     &= elm->offersParam(ParamId::Colour);
        c.hasAudioBox   &= elm->offersParam(ParamId::AudioFile);
        c.hasFrequency  &= elm->offersParam(ParamId::Frequency);
        c.hasDelay      &= elm->offersParam(ParamId::Delay);
        c.minimumInputs  = (std::max)(c.minimumInputs,  elm->minInputSize());
        c.maximumInputs  = (std::min)(c.maximumInputs,  elm->maxInputSize());
        c.minimumOutputs = (std::max)(c.minimumOutputs, elm->minOutputSize());
        c.maximumOutputs = (std::min)(c.maximumOutputs, elm->maxOutputSize());
        c.hasVolume       &= elm->offersParam(ParamId::Volume);
        c.hasWirelessMode &= elm->offersParam(ParamId::Wireless);
        c.hasTrigger      &= elm->hasTrigger();

        c.hasSameLabel      &= (elm->label()    == firstElement->label());
        c.hasSameColors     &= sameParam(*elm, *firstElement, ParamId::Colour);
        c.hasSameFrequency  &= sameParam(*elm, *firstElement, ParamId::Frequency);
        c.hasSameDelay      &= sameParam(*elm, *firstElement, ParamId::Delay);
        c.hasSameInputSize  &= (elm->inputSize()  == firstElement->inputSize());
        c.hasSameOutputSize &= (elm->outputSize() == firstElement->outputSize());
        c.maxCurrentOutputSize = (std::min)(c.maxCurrentOutputSize, elm->outputSize());

        if ((group == ElementGroup::Input) && (firstElement->elementGroup() == ElementGroup::Input)) {
            c.hasSameOutputValue &= (inputValue(*elm) == inputValue(*firstElement));
            c.sameCheckState     &= (elm->isLocked()  == firstElement->isLocked());
        }

        c.hasSameTrigger &= (elm->trigger()      == firstElement->trigger());
        c.hasSameType    &= (elm->elementType()  == firstElement->elementType());
        c.hasSameVolume  &= sameParam(*elm, *firstElement, ParamId::Volume);

        // Embedded IC flags: AND-reduced so both are true only when ALL selected
        // elements are ICs that are embedded or file-backed respectively.
        if (elm->elementType() != ElementType::IC) {
            c.isFileBacked = false;
            c.isEmbedded = false;
        } else {
            c.isFileBacked &= !elm->isEmbedded();
            c.isEmbedded   &=  elm->isEmbedded();
        }

        // Static inputs (Vcc/Gnd) and regular inputs share morph compatibility
        // because they occupy the same pin role in a circuit.
        bool sameElementGroup  = (group == firstGroup);
        sameElementGroup |= (group == ElementGroup::Input       && firstGroup == ElementGroup::StaticInput);
        sameElementGroup |= (group == ElementGroup::StaticInput && firstGroup == ElementGroup::Input);
        c.hasOnlyInputs    &= (group == ElementGroup::Input);
        c.hasLatchedValue  &= (group == ElementGroup::Input) && (elm->elementType() != ElementType::InputButton);
        c.canMorph         &= sameElementGroup;
    }

    if (!c.hasSameType) {
        c.elementType = ElementType::Unknown;
    }

    // A size combobox is only useful when there is a valid range of options.
    c.canChangeInputSize  = (c.minimumInputs  < c.maximumInputs);
    // TruthTable output count is controlled by its own dialog, not this combobox.
    c.canChangeOutputSize = (c.minimumOutputs < c.maximumOutputs && !c.hasTruthTable);
    // For Demux, input size is derived from output size — hide input combobox.
    if (c.hasSameType && c.elementType == ElementType::Demux) {
        c.canChangeInputSize = false;
    }

    return c;
}

int inputValue(const Element &element)
{
    return element.hasParam(ParamId::Position) ? element.param<int>(ParamId::Position) : static_cast<int>(element.isOn());
}
