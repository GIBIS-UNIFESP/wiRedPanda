// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ElementContextMenu.h"

#include <QAction>
#include <QComboBox>
#include <QMenu>
#include <QPixmap>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/UI/SelectionCapabilities.h"

// Helper: add a morph target unless the element already IS that type.
static QAction *addElementAction(QMenu *menu, GraphicElement *selectedElm, ElementType type, const bool hasSameType)
{
    if (hasSameType && (selectedElm->elementType() == type)) {
        return nullptr;
    }

    auto *action = menu->addAction(QIcon(ElementFactory::pixmap(type)), ElementFactory::translatedName(type));
    action->setData(static_cast<int>(type));
    return action;
}

void ElementContextMenu::exec(QPoint screenPos,
                              QGraphicsItem *itemAtMouse,
                              const SelectionCapabilities &caps,
                              const QList<GraphicElement *> &elements,
                              QComboBox *colorCombo,
                              Scene *scene,
                              const std::function<void(QUndoCommand *)> &sendCommand,
                              const std::function<void()> &onRename,
                              const std::function<void()> &onTriggerChange,
                              const std::function<void()> &onSkinChange,
                              const std::function<void()> &onSkinRevert,
                              const std::function<void()> &onFrequencyFocus,
                              const std::function<void()> &onEditSubcircuit,
                              const std::function<void()> &onEmbedSubcircuit,
                              const std::function<void()> &onExtractToFile)
{
    QMenu menu;
    const QString changeSkinText(QObject::tr("Change skin to ..."));
    const QString colorMenuText(QObject::tr("Change color to..."));
    const QString frequencyText(QObject::tr("Change frequency"));
    const QString morphMenuText(QObject::tr("Morph to..."));
    const QString renameText(QObject::tr("Rename"));
    const QString revertSkinText(QObject::tr("Set skin to default"));
    const QString rotateLeftText(QObject::tr("Rotate left"));
    const QString rotateRightText(QObject::tr("Rotate right"));
    const QString triggerText(QObject::tr("Change trigger"));

    if (caps.hasLabel) {
        menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/rename.svg")), renameText)->setData(renameText);
    }

    if (caps.hasTrigger) {
        menu.addAction(QIcon(ElementFactory::pixmap(ElementType::InputButton)), triggerText)->setData(triggerText);
    }

    if (caps.canChangeSkin) {
        menu.addAction(changeSkinText);
        menu.addAction(revertSkinText);
    }

    menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/rotateL.svg")), rotateLeftText)->setData(rotateLeftText);
    menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/rotateR.svg")), rotateRightText)->setData(rotateRightText);

    if (caps.hasFrequency) {
        menu.addAction(QIcon(ElementFactory::pixmap(ElementType::Clock)), frequencyText)->setData(frequencyText);
    }

    QMenu *submenuColors = nullptr;

    if (caps.hasColors) {
        submenuColors = menu.addMenu(colorMenuText);
        for (int i = 0; i < colorCombo->count(); ++i) {
            if (colorCombo->currentIndex() != i) {
                submenuColors->addAction(colorCombo->itemIcon(i), colorCombo->itemText(i));
            }
        }
    }

    QMenu *submenuMorph = nullptr;

    if (auto *selectedElm = qgraphicsitem_cast<GraphicElement *>(itemAtMouse); selectedElm && caps.canMorph) {
        submenuMorph = menu.addMenu(morphMenuText);

        switch (selectedElm->elementGroup()) {
        case ElementGroup::Gate: {
            if (selectedElm->inputSize() == 1) {
                addElementAction(submenuMorph, selectedElm, ElementType::Node, caps.hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Not, caps.hasSameType);
            } else {
                addElementAction(submenuMorph, selectedElm, ElementType::And, caps.hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Nand, caps.hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Nor, caps.hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Or, caps.hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Xnor, caps.hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Xor, caps.hasSameType);
            }
            break;
        }

        case ElementGroup::StaticInput: [[fallthrough]];
        case ElementGroup::Input: {
            addElementAction(submenuMorph, selectedElm, ElementType::Clock, caps.hasSameType);
            addElementAction(submenuMorph, selectedElm, ElementType::InputButton, caps.hasSameType);
            addElementAction(submenuMorph, selectedElm, ElementType::InputGnd, caps.hasSameType);
            addElementAction(submenuMorph, selectedElm, ElementType::InputRotary, caps.hasSameType);
            addElementAction(submenuMorph, selectedElm, ElementType::InputSwitch, caps.hasSameType);
            addElementAction(submenuMorph, selectedElm, ElementType::InputVcc, caps.hasSameType);
            break;
        }

        case ElementGroup::Memory: {
            // 2 inputs → DLatch; 4 inputs → DFF/TFF; 5 inputs → JKFF/SRFF.
            if (selectedElm->inputSize() == 2) {
                addElementAction(submenuMorph, selectedElm, ElementType::DLatch, caps.hasSameType);
                break;
            }
            if (selectedElm->inputSize() == 4) {
                addElementAction(submenuMorph, selectedElm, ElementType::DFlipFlop, caps.hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::TFlipFlop, caps.hasSameType);
                break;
            }
            if (selectedElm->inputSize() == 5) {
                addElementAction(submenuMorph, selectedElm, ElementType::JKFlipFlop, caps.hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::SRFlipFlop, caps.hasSameType);
            }
            break;
        }

        case ElementGroup::Output: {
            if ((selectedElm->elementType() == ElementType::Display7)
                    || (selectedElm->elementType() == ElementType::Display14)
                    || (selectedElm->elementType() == ElementType::Display16)) {
                addElementAction(submenuMorph, selectedElm, ElementType::Display7, caps.hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Display14, caps.hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Display16, caps.hasSameType);
            } else {
                addElementAction(submenuMorph, selectedElm, ElementType::Buzzer, caps.hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Led, caps.hasSameType);
            }
            break;
        }

        case ElementGroup::IC:      [[fallthrough]];
        case ElementGroup::Mux:     [[fallthrough]];
        case ElementGroup::Other:
        case ElementGroup::Unknown: break;

        default: break;
        }

        if (submenuMorph->actions().empty()) {
            menu.removeAction(submenuMorph->menuAction());
        }
    }

    // --- IC sub-circuit actions ---
    const QString editSubcircuitText(QObject::tr("Edit sub-circuit"));
    const QString embedSubcircuitText(QObject::tr("Embed sub-circuit"));
    const QString extractToFileText(QObject::tr("Extract to file"));

    if (onEditSubcircuit && (caps.isEmbedded || caps.isFileBacked)) {
        menu.addAction(editSubcircuitText)->setData(editSubcircuitText);
    }
    if (onEmbedSubcircuit && caps.isFileBacked) {
        menu.addAction(embedSubcircuitText)->setData(embedSubcircuitText);
    }
    if (onExtractToFile && caps.isEmbedded) {
        menu.addAction(extractToFileText)->setData(extractToFileText);
    }

    menu.addSeparator();

    if (caps.hasElements) {
        QAction *copyAction = menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/copy.svg")), QObject::tr("Copy"));
        QAction *cutAction  = menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/cut.svg")),  QObject::tr("Cut"));
        QObject::connect(copyAction, &QAction::triggered, scene, &Scene::copyAction);
        QObject::connect(cutAction,  &QAction::triggered, scene, &Scene::cutAction);
    }

    QAction *deleteAction = menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/delete.svg")), QObject::tr("Delete"));
    QObject::connect(deleteAction, &QAction::triggered, scene, &Scene::deleteAction);

    QAction *action = menu.exec(screenPos);

    if (!action) {
        return;
    }

    const QString actionData = action->data().toString();
    const QString actionText = action->text();

    if (actionData == renameText)      { onRename();       return; }
    if (actionData == triggerText)     { onTriggerChange(); return; }
    if (actionText == changeSkinText)  { onSkinChange();   return; }
    if (actionText == revertSkinText)  { onSkinRevert();   return; }
    if (actionData == frequencyText)   { onFrequencyFocus(); return; }
    if (actionData == editSubcircuitText)  { onEditSubcircuit(); return; }
    if (actionData == embedSubcircuitText) { onEmbedSubcircuit(); return; }
    if (actionData == extractToFileText)   { onExtractToFile(); return; }

    if (actionData == rotateLeftText) {
        sendCommand(new RotateCommand(elements, -90.0, scene));
        return;
    }

    if (actionData == rotateRightText) {
        sendCommand(new RotateCommand(elements, 90.0, scene));
        return;
    }

    if (submenuMorph && submenuMorph->actions().contains(action)) {
        if (auto type = static_cast<ElementType>(action->data().toInt()); type != ElementType::Unknown) {
            sendCommand(new MorphCommand(elements, type, scene));
        }
        return;
    }

    if (submenuColors && submenuColors->actions().contains(action)) {
        colorCombo->setCurrentText(action->text());
        return;
    }

    if ((actionText == QObject::tr("Copy")) || (actionText == QObject::tr("Cut")) || (actionText == QObject::tr("Delete"))) {
        return;
    }

    throw PANDACEPTION_WITH_CONTEXT("ElementContextMenu", "Unknown context menu option.");
}

