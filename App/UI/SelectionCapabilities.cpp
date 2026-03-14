// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/SelectionCapabilities.h"

#include "App/Core/Enums.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"

SelectionCapabilities computeCapabilities(const QList<GraphicElement *> &elements)
{
    SelectionCapabilities c;

    if (elements.isEmpty()) {
        return c;
    }

    auto *firstElement = elements.constFirst();
    auto *firstInput   = qobject_cast<GraphicElementInput *>(firstElement);

    // Start all flags true; AND-reduce over the selection below.
    c.hasAudioBox = c.hasAudio = c.hasColors = c.hasDelay = c.hasElements = true;
    c.hasFrequency = c.hasLabel = c.hasOnlyInputs = c.hasWirelessMode = c.hasTrigger = c.hasTruthTable = true;
    c.canChangeSkin = c.canMorph = true;
    c.hasSameAudio = c.hasSameColors = c.hasSameDelay = c.hasSameFrequency = true;
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
        c.canChangeSkin &= elm->canChangeSkin();
        c.hasColors     &= elm->hasColors();
        c.hasAudio      &= elm->hasAudio();
        c.hasAudioBox   &= elm->hasAudioBox();
        c.hasFrequency  &= elm->hasFrequency();
        c.hasDelay      &= elm->hasDelay();
        c.minimumInputs  = std::max(c.minimumInputs,  elm->minInputSize());
        c.maximumInputs  = std::min(c.maximumInputs,  elm->maxInputSize());
        c.minimumOutputs = std::max(c.minimumOutputs, elm->minOutputSize());
        c.maximumOutputs = std::min(c.maximumOutputs, elm->maxOutputSize());
        c.hasWirelessMode &= elm->hasWirelessMode();
        c.hasTrigger      &= elm->hasTrigger();

        c.hasSameLabel      &= (elm->label()    == firstElement->label());
        c.hasSameColors     &= (elm->color()    == firstElement->color());
        c.hasSameFrequency  &= qFuzzyCompare(elm->frequency(), firstElement->frequency());
        c.hasSameDelay      &= qFuzzyCompare(elm->delay(),     firstElement->delay());
        c.hasSameInputSize  &= (elm->inputSize()  == firstElement->inputSize());
        c.hasSameOutputSize &= (elm->outputSize() == firstElement->outputSize());
        c.maxCurrentOutputSize = std::min(c.maxCurrentOutputSize, elm->outputSize());

        if (auto *elmInput = qobject_cast<GraphicElementInput *>(elm);
                elmInput && (group == ElementGroup::Input) && (firstElement->elementGroup() == ElementGroup::Input)) {
            c.hasSameOutputValue &= (elmInput->outputValue() == firstInput->outputValue());
            c.sameCheckState     &= (elmInput->isLocked()    == firstInput->isLocked());
        }

        c.hasSameTrigger &= (elm->trigger()      == firstElement->trigger());
        c.hasSameType    &= (elm->elementType()  == firstElement->elementType());
        c.hasSameAudio   &= (elm->audio()        == firstElement->audio());

        // Static inputs (Vcc/Gnd) and regular inputs share morph compatibility
        // because they occupy the same pin role in a circuit.
        bool sameElementGroup  = (group == firstGroup);
        sameElementGroup |= (group == ElementGroup::Input       && firstGroup == ElementGroup::StaticInput);
        sameElementGroup |= (group == ElementGroup::StaticInput && firstGroup == ElementGroup::Input);
        c.hasOnlyInputs &= (group == ElementGroup::Input);
        c.canMorph      &= sameElementGroup;
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
