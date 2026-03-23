// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/PropertyShortcutHandler.h"

#include "App/Core/Enums.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"

PropertyShortcutHandler::PropertyShortcutHandler(Scene *scene)
    : m_scene(scene)
{
}

void PropertyShortcutHandler::prevMainProperty()
{
    // Keyboard shortcut to decrement the "primary" configurable property of each
    // selected element: input count for logic gates, output count for rotary inputs,
    // clock frequency (step 0.5 Hz), buzzer audio, or display color
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
        case ElementType::TruthTable:
            if (element->inputSize() > element->minInputSize())
                m_scene->receiveCommand(new ChangeInputSizeCommand(QList<GraphicElement *>{element},
                                                                   element->inputSize() - 1, m_scene));
            break;

        // Input ports
        case ElementType::InputRotary:
            if (element->outputSize() > element->minOutputSize())
                m_scene->receiveCommand(new ChangeOutputSizeCommand(QList<GraphicElement *>{element},
                                                                    element->outputSize() - 1, m_scene));
            break;

        case ElementType::Clock:
            if (element->hasFrequency())
                element->setFrequency(element->frequency() - 0.5f);
            break;

        case ElementType::Buzzer:
            if (element->hasAudio())
                element->setAudio(element->previousAudio());
            break;

        case ElementType::Display16:
        case ElementType::Display14:
        case ElementType::Display7:
            if (element->hasColors())
                element->setColor(element->previousColor());
            break;

        default: // Not implemented
            break;
        }

        // Toggling selection off and on forces the property inspector to refresh
        element->setSelected(false);
        element->setSelected(true);
    }
}

void PropertyShortcutHandler::nextMainProperty()
{
    // Mirror of prevMainProperty() — increments the same per-type primary property
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
        case ElementType::TruthTable:
            if (element->inputSize() < element->maxInputSize())
                m_scene->receiveCommand(new ChangeInputSizeCommand(QList<GraphicElement *>{element},
                                                                   element->inputSize() + 1, m_scene));
            break;

        // Input ports
        case ElementType::InputRotary:
            if (element->outputSize() < element->maxOutputSize())
                m_scene->receiveCommand(new ChangeOutputSizeCommand(QList<GraphicElement *>{element},
                                                                    element->outputSize() + 1, m_scene));
            break;

        case ElementType::Clock:
            if (element->hasFrequency())
                element->setFrequency(element->frequency() + 0.5f);
            break;

        case ElementType::Buzzer:
            if (element->hasAudio())
                element->setAudio(element->nextAudio());
            break;

        case ElementType::Display14:
        case ElementType::Display7:
            if (element->hasColors())
                element->setColor(element->nextColor());
            break;

        default: // Not implemented
            break;
        }

        element->setSelected(false);
        element->setSelected(true);
    }
}

void PropertyShortcutHandler::prevSecondaryProperty()
{
    for (auto *element : m_scene->selectedElements()) {
        switch (element->elementType()) {
        case ElementType::TruthTable:
            if (element->outputSize() > element->minOutputSize())
                m_scene->receiveCommand(new ChangeOutputSizeCommand(QList<GraphicElement *>{element},
                                                                    element->outputSize() - 1, m_scene));
            break;

        case ElementType::Led:
            if (element->hasColors())
                element->setColor(element->previousColor());
            break;

        default:
            break;
        }

        element->setSelected(false);
        element->setSelected(true);
    }
}

void PropertyShortcutHandler::nextSecondaryProperty()
{
    for (auto *element : m_scene->selectedElements()) {
        switch (element->elementType()) {
        case ElementType::TruthTable:
            if (element->outputSize() < element->maxOutputSize())
                m_scene->receiveCommand(new ChangeOutputSizeCommand(QList<GraphicElement *>{element},
                                                                    element->outputSize() + 1, m_scene));
            break;

        case ElementType::Led:
            if (element->hasColors())
                element->setColor(element->nextColor());
            break;

        default:
            break;
        }

        element->setSelected(false);
        element->setSelected(true);
    }
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

