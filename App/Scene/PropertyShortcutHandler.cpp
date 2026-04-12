// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/PropertyShortcutHandler.h"

#include <algorithm>

#include "App/Core/Enums.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"

PropertyShortcutHandler::PropertyShortcutHandler(Scene *scene)
    : m_scene(scene)
{
}

void PropertyShortcutHandler::adjustMainProperty(int dir)
{
    // dir = -1 for prev, +1 for next
    // Cycles the "primary" configurable property of each selected element:
    // input count for logic gates, output count for rotary inputs,
    // clock frequency (0.5 Hz step), buzzer frequency (100 Hz step), or display color.
    for (auto *element : m_scene->selectedElements()) {
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
                m_scene->receiveCommand(new ChangePortSizeCommand(QList<GraphicElement *>{element},
                                                                  newSize, m_scene, true));
            break;
        }
        // Input ports
        case ElementType::InputRotary: {
            const int newSize = element->outputSize() + dir;
            if (newSize >= element->minOutputSize() && newSize <= element->maxOutputSize())
                m_scene->receiveCommand(new ChangePortSizeCommand(QList<GraphicElement *>{element},
                                                                  newSize, m_scene, false));
            break;
        }
        case ElementType::Clock:
            if (element->hasFrequency())
                element->setFrequency(element->frequency() + dir * 0.5);
            break;

        case ElementType::Buzzer:
            if (element->hasFrequency())
                element->setFrequency(std::clamp(element->frequency() + dir * 100.0, 20.0, 20000.0));
            break;

        case ElementType::Display16:
            // Only previous-color is handled for Display16 (no next-color cycling)
            if (dir < 0 && element->hasColors())
                element->setColor(element->previousColor());
            break;

        case ElementType::Display14:
        case ElementType::Display7:
            if (element->hasColors())
                element->setColor(dir < 0 ? element->previousColor() : element->nextColor());
            break;

        default: // Not implemented
            break;
        }

        // Toggling selection off and on forces the property inspector to refresh
        element->setSelected(false);
        element->setSelected(true);
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
    for (auto *element : m_scene->selectedElements()) {
        switch (element->elementType()) {
        case ElementType::TruthTable: {
            const int newSize = element->outputSize() + dir;
            if (newSize >= element->minOutputSize() && newSize <= element->maxOutputSize())
                m_scene->receiveCommand(new ChangePortSizeCommand(QList<GraphicElement *>{element},
                                                                  newSize, m_scene, false));
            break;
        }
        case ElementType::Led:
            if (element->hasColors())
                element->setColor(dir < 0 ? element->previousColor() : element->nextColor());
            break;

        default:
            break;
        }

        element->setSelected(false);
        element->setSelected(true);
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
    for (auto *element : m_scene->selectedElements()) {
        const QPointF elmPosition = element->scenePos();
        auto nextType = Enums::nextElmType(element->elementType());

        // ElementType::Unknown signals there is no "next" in the cycle for this type
        if (nextType == ElementType::Unknown) { continue; }

        m_scene->receiveCommand(new MorphCommand(QList<GraphicElement *>{element},
                                                 Enums::nextElmType(element->elementType()), m_scene));

        // MorphCommand replaces the element in-place; re-select via position because
        // the old pointer is now invalid after the command's redo()
        auto *item = m_scene->itemAt(elmPosition);
        if (item) {
            item->setSelected(true);
        }
    }
}

void PropertyShortcutHandler::prevElement()
{
    for (auto *element : m_scene->selectedElements()) {
        const QPointF elmPosition = element->scenePos();
        auto prevType = Enums::prevElmType(element->elementType());

        if (prevType == ElementType::Unknown) { continue; }

        m_scene->receiveCommand(new MorphCommand(QList<GraphicElement *>{element},
                                                 Enums::prevElmType(element->elementType()), m_scene));

        auto *item = m_scene->itemAt(elmPosition);
        if (item) {
            item->setSelected(true);
        }
    }
}

