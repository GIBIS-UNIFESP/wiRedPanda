// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/PropertyShortcutHandler.h"

#include <algorithm>

#include "App/Core/Enums.h"
#include "App/Element/Element.h"
#include "App/Element/ElementColour.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"

namespace PropertyShortcutHandlerDetail {

/// Steps \a element's colour one place along the cycle (ElementColour): back when \a dir is negative.
void stepColour(Element &element, const int dir)
{
    const auto &colour = element.param<QString>(ParamId::Colour);
    element.setParam(ParamId::Colour, dir < 0 ? ElementColour::previous(colour) : ElementColour::next(colour));
}

/// Sets \a element's frequency \a step times \a dir past its own, clamped to [\a lowest, \a highest].
void stepFrequency(Element &element, const int dir, const double step, const double lowest, const double highest)
{
    const double frequency = element.param<double>(ParamId::Frequency) + dir * step;
    element.setParam(ParamId::Frequency, std::clamp(frequency, lowest, highest));
}

} // namespace PropertyShortcutHandlerDetail

using PropertyShortcutHandlerDetail::stepColour;
using PropertyShortcutHandlerDetail::stepFrequency;

PropertyShortcutHandler::PropertyShortcutHandler(Scene *scene)
    : m_scene(scene)
{
}

void PropertyShortcutHandler::applyWithUndo(Element *element, const std::function<void()> &mutate)
{
    // Wraps a direct property mutation in an UpdateCommand (F37) so the
    // [ / ] shortcuts are undoable and mark the circuit dirty, exactly like
    // the port-size path below and the property editor.
    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        element->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }
    mutate();
    m_scene->receiveCommand(new UpdateCommand({element}, oldData, m_scene));
}

void PropertyShortcutHandler::adjustMainProperty(int dir)
{
    // dir = -1 for prev, +1 for next
    // Cycles the "primary" configurable property of each selected element:
    // input count for logic gates, output count for rotary inputs,
    // clock frequency (0.5 Hz step), buzzer frequency (100 Hz step), or display color.
    const auto selected = m_scene->selectedElements();
    const bool needsMacro = selected.size() > 1;
    if (needsMacro) {
        m_scene->undoStack()->beginMacro(tr("Cycle element properties"));
    }

    for (auto *element : selected) {
        switch (element->elementType()) {
        // Logic Elements
        case ElementType::And:
        case ElementType::Or:
        case ElementType::Nand:
        case ElementType::Nor:
        case ElementType::Xor:
        case ElementType::Xnor:
        // Output and truthtable
        case ElementType::Led:
        case ElementType::TruthTable: {
            const int newSize = element->inputSize() + dir;
            if (newSize >= element->minInputSize() && newSize <= element->maxInputSize())
                m_scene->receiveCommand(new ChangePortSizeCommand(QList<Element *>{element},
                                                                  newSize, m_scene, true));
            break;
        }
        // Input ports
        case ElementType::InputRotary: {
            const int newSize = element->outputSize() + dir;
            if (newSize >= element->minOutputSize() && newSize <= element->maxOutputSize())
                m_scene->receiveCommand(new ChangePortSizeCommand(QList<Element *>{element},
                                                                  newSize, m_scene, false));
            break;
        }
        case ElementType::Clock:
            if (element->offersParam(ParamId::Frequency))
                applyWithUndo(element, [element, dir] {
                    element->setParam(ParamId::Frequency, element->param<double>(ParamId::Frequency) + dir * 0.5);
                });
            break;

        case ElementType::Buzzer:
            if (element->offersParam(ParamId::Frequency))
                applyWithUndo(element, [element, dir] { stepFrequency(*element, dir, 100.0, 20.0, 20000.0); });
            break;

        case ElementType::Display16:
            // Only previous-color is handled for Display16 (no next-color cycling)
            if (dir < 0 && element->offersParam(ParamId::Colour))
                applyWithUndo(element, [element] { stepColour(*element, -1); });
            break;

        case ElementType::Display14:
        case ElementType::Display7:
            if (element->offersParam(ParamId::Colour))
                applyWithUndo(element, [element, dir] { stepColour(*element, dir); });
            break;

        // Not implemented for these types
        case ElementType::AudioBox:
        case ElementType::DFlipFlop:
        case ElementType::DLatch:
        case ElementType::Demux:
        case ElementType::IC:
        case ElementType::InputButton:
        case ElementType::InputGnd:
        case ElementType::InputSwitch:
        case ElementType::InputVcc:
        case ElementType::JKFlipFlop:
        case ElementType::JKLatch:
        case ElementType::Line:
        case ElementType::Mux:
        case ElementType::Node:
        case ElementType::Not:
        case ElementType::SRFlipFlop:
        case ElementType::SRLatch:
        case ElementType::TFlipFlop:
        case ElementType::Text:
        case ElementType::Unknown:
            break;
        }

        // Toggling selection off and on forces the property inspector to refresh
        element->setSelected(false);
        element->setSelected(true);
    }

    if (needsMacro) {
        m_scene->undoStack()->endMacro();
    }
}

void PropertyShortcutHandler::prevMainProperty()
{
    adjustMainProperty(-1);
}

void PropertyShortcutHandler::nextMainProperty()
{
    adjustMainProperty(1);
}

void PropertyShortcutHandler::adjustSecondaryProperty(int dir)
{
    // dir = -1 for prev, +1 for next
    const auto selected = m_scene->selectedElements();
    const bool needsMacro = selected.size() > 1;
    if (needsMacro) {
        m_scene->undoStack()->beginMacro(tr("Cycle element properties"));
    }

    for (auto *element : selected) {
        switch (element->elementType()) {
        case ElementType::TruthTable: {
            const int newSize = element->outputSize() + dir;
            if (newSize >= element->minOutputSize() && newSize <= element->maxOutputSize())
                m_scene->receiveCommand(new ChangePortSizeCommand(QList<Element *>{element},
                                                                  newSize, m_scene, false));
            break;
        }
        case ElementType::Led:
            if (element->offersParam(ParamId::Colour))
                applyWithUndo(element, [element, dir] { stepColour(*element, dir); });
            break;

        case ElementType::And:
        case ElementType::AudioBox:
        case ElementType::Buzzer:
        case ElementType::Clock:
        case ElementType::DFlipFlop:
        case ElementType::DLatch:
        case ElementType::Demux:
        case ElementType::Display14:
        case ElementType::Display16:
        case ElementType::Display7:
        case ElementType::IC:
        case ElementType::InputButton:
        case ElementType::InputGnd:
        case ElementType::InputRotary:
        case ElementType::InputSwitch:
        case ElementType::InputVcc:
        case ElementType::JKFlipFlop:
        case ElementType::JKLatch:
        case ElementType::Line:
        case ElementType::Mux:
        case ElementType::Nand:
        case ElementType::Node:
        case ElementType::Nor:
        case ElementType::Not:
        case ElementType::Or:
        case ElementType::SRFlipFlop:
        case ElementType::SRLatch:
        case ElementType::TFlipFlop:
        case ElementType::Text:
        case ElementType::Unknown:
        case ElementType::Xnor:
        case ElementType::Xor:
            break;
        }

        element->setSelected(false);
        element->setSelected(true);
    }

    if (needsMacro) {
        m_scene->undoStack()->endMacro();
    }
}

void PropertyShortcutHandler::prevSecondaryProperty()
{
    adjustSecondaryProperty(-1);
}

void PropertyShortcutHandler::nextSecondaryProperty()
{
    adjustSecondaryProperty(1);
}

void PropertyShortcutHandler::nextElement()
{
    const auto selected = m_scene->selectedElements();
    const bool needsMacro = selected.size() > 1;
    if (needsMacro) {
        m_scene->undoStack()->beginMacro(tr("Morph elements"));
    }

    for (auto *element : selected) {
        const QPointF elmPosition = element->scenePos();
        auto nextType = Enums::nextElmType(element->elementType());

        // ElementType::Unknown signals there is no "next" in the cycle for this type
        if (nextType == ElementType::Unknown) { continue; }

        m_scene->receiveCommand(new MorphCommand(QList<Element *>{element},
                                                 Enums::nextElmType(element->elementType()), m_scene));

        // MorphCommand replaces the element in-place; re-select via position because
        // the old pointer is now invalid after the command's redo()
        auto *item = m_scene->itemAt(elmPosition);
        if (item) {
            item->setSelected(true);
        }
    }

    if (needsMacro) {
        m_scene->undoStack()->endMacro();
    }
}

void PropertyShortcutHandler::prevElement()
{
    const auto selected = m_scene->selectedElements();
    const bool needsMacro = selected.size() > 1;
    if (needsMacro) {
        m_scene->undoStack()->beginMacro(tr("Morph elements"));
    }

    for (auto *element : selected) {
        const QPointF elmPosition = element->scenePos();
        auto prevType = Enums::prevElmType(element->elementType());

        if (prevType == ElementType::Unknown) { continue; }

        m_scene->receiveCommand(new MorphCommand(QList<Element *>{element},
                                                 Enums::prevElmType(element->elementType()), m_scene));

        auto *item = m_scene->itemAt(elmPosition);
        if (item) {
            item->setSelected(true);
        }
    }

    if (needsMacro) {
        m_scene->undoStack()->endMacro();
    }
}
