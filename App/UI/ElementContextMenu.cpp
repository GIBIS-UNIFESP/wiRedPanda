// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ElementContextMenu.h"

#include <QAction>
#include <QComboBox>
#include <QMenu>
#include <QPixmap>
#include <QScopeGuard>

#include <algorithm>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ConnectionManager.h"
#include "App/Scene/Scene.h"
#include "App/UI/SelectionCapabilities.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

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

namespace ElementContextMenu {

QList<InputPort *> freeInputPorts(GraphicElement *elm)
{
    QList<InputPort *> result;
    for (int i = 0; i < elm->inputSize(); ++i) {
        if (auto *port = elm->inputPort(i); port->connections().isEmpty()) {
            result.append(port);
        }
    }
    return result;
}

QList<OutputPort *> allOutputPorts(GraphicElement *elm)
{
    QList<OutputPort *> result;
    result.reserve(elm->outputSize());
    for (int i = 0; i < elm->outputSize(); ++i) {
        result.append(elm->outputPort(i));
    }
    return result;
}

QList<QGraphicsItem *> buildCorrespondingConnections(const QList<OutputPort *> &outputs,
                                                      const QList<InputPort *> &inputs)
{
    QList<QGraphicsItem *> result;
    const int count = std::min(outputs.size(), inputs.size());
    result.reserve(count);

    for (int i = 0; i < count; ++i) {
        if (!ConnectionManager::connectionRejectionReason(outputs[i], inputs[i]).isEmpty()) {
            continue;
        }
        auto *conn = new Connection();
        conn->setStartPort(outputs[i]);
        conn->setEndPort(inputs[i]);
        conn->updatePath();
        result.append(conn);
    }
    return result;
}

} // namespace ElementContextMenu

void ElementContextMenu::exec(QPoint screenPos,
                              QGraphicsItem *itemAtMouse,
                              const SelectionCapabilities &caps,
                              const QList<GraphicElement *> &elements,
                              QComboBox *colorCombo,
                              Scene *scene,
                              const std::function<void(QUndoCommand *)> &sendCommand,
                              const std::function<void()> &onRename,
                              const std::function<void()> &onTriggerChange,
                              const std::function<void()> &onAppearanceChange,
                              const std::function<void()> &onAppearanceRevert,
                              const std::function<void()> &onFrequencyFocus,
                              const std::function<void()> &onEditSubcircuit,
                              const std::function<void()> &onEmbedSubcircuit,
                              const std::function<void()> &onExtractToFile)
{
    QMenu menu;
    const QString changeAppearanceText(QObject::tr("Change appearance to ..."));
    const QString colorMenuText(QObject::tr("Change color to..."));
    const QString flipHorizontalText(QObject::tr("Flip horizontally"));
    const QString flipVerticalText(QObject::tr("Flip vertically"));
    const QString frequencyText(QObject::tr("Change frequency"));
    const QString morphMenuText(QObject::tr("Morph to..."));
    const QString renameText(QObject::tr("Rename"));
    const QString revertAppearanceText(QObject::tr("Restore default appearance"));
    const QString rotateLeftText(QObject::tr("Rotate left"));
    const QString rotateRightText(QObject::tr("Rotate right"));
    const QString triggerText(QObject::tr("Change trigger"));

    if (caps.hasLabel) {
        menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/rename.svg")), renameText)->setData(renameText);
    }

    if (caps.hasTrigger) {
        menu.addAction(QIcon(ElementFactory::pixmap(ElementType::InputButton)), triggerText)->setData(triggerText);
    }

    if (caps.canChangeAppearance) {
        menu.addAction(changeAppearanceText);
        menu.addAction(revertAppearanceText);
    }

    menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/rotateL.svg")), rotateLeftText)->setData(rotateLeftText);
    menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/rotateR.svg")), rotateRightText)->setData(rotateRightText);
    menu.addAction(flipHorizontalText)->setData(flipHorizontalText);
    menu.addAction(flipVerticalText)->setData(flipVerticalText);

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

    // --- Bulk-connect corresponding ports between exactly two selected elements ---
    // (e.g. an IC's output ports wired one-by-one to a Display14's input ports --
    // connect every matched pair in one action instead of drawing each wire by hand.)
    constexpr auto kConnectFirstToSecond = "connectFirstToSecond";
    constexpr auto kConnectSecondToFirst = "connectSecondToFirst";
    QList<QGraphicsItem *> connectFirstToSecondItems;
    QList<QGraphicsItem *> connectSecondToFirstItems;
    // Whichever list (if either) isn't handed to a command below stays owned here and must
    // be freed -- a menu dismissal or an unrelated action picked instead must not leak the
    // Connection objects built (and port-attached) speculatively above.
    const auto freeUnusedConnections = qScopeGuard([&connectFirstToSecondItems, &connectSecondToFirstItems] {
        qDeleteAll(connectFirstToSecondItems);
        qDeleteAll(connectSecondToFirstItems);
    });

    if (elements.size() == 2) {
        auto *first = elements[0];
        auto *second = elements[1];
        const auto firstOutputs = allOutputPorts(first);
        const auto secondOutputs = allOutputPorts(second);
        const auto firstFreeInputs = freeInputPorts(first);
        const auto secondFreeInputs = freeInputPorts(second);

        connectFirstToSecondItems = buildCorrespondingConnections(firstOutputs, secondFreeInputs);
        connectSecondToFirstItems = buildCorrespondingConnections(secondOutputs, firstFreeInputs);

        const QString firstName = ElementFactory::translatedName(first->elementType());
        const QString secondName = ElementFactory::translatedName(second->elementType());

        if (!connectFirstToSecondItems.isEmpty()) {
            const QString text = QObject::tr("Connect corresponding ports (%1 → %2)").arg(firstName, secondName);
            menu.addAction(text)->setData(QString::fromLatin1(kConnectFirstToSecond));
        }
        if (!connectSecondToFirstItems.isEmpty()) {
            const QString text = QObject::tr("Connect corresponding ports (%1 → %2)").arg(secondName, firstName);
            menu.addAction(text)->setData(QString::fromLatin1(kConnectSecondToFirst));
        }
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
    if (actionText == changeAppearanceText)  { onAppearanceChange();   return; }
    if (actionText == revertAppearanceText)  { onAppearanceRevert();   return; }
    if (actionData == frequencyText)   { onFrequencyFocus(); return; }
    if (actionData == editSubcircuitText)  { onEditSubcircuit(); return; }
    if (actionData == embedSubcircuitText) { onEmbedSubcircuit(); return; }
    if (actionData == extractToFileText)   { onExtractToFile(); return; }

    if (actionData == QLatin1String(kConnectFirstToSecond)) {
        sendCommand(new AddItemsCommand(connectFirstToSecondItems, scene));
        connectFirstToSecondItems.clear(); // ownership transferred to the command
        return;
    }

    if (actionData == QLatin1String(kConnectSecondToFirst)) {
        sendCommand(new AddItemsCommand(connectSecondToFirstItems, scene));
        connectSecondToFirstItems.clear(); // ownership transferred to the command
        return;
    }

    if (actionData == rotateLeftText) {
        sendCommand(new RotateCommand(elements, -90.0, scene));
        return;
    }

    if (actionData == rotateRightText) {
        sendCommand(new RotateCommand(elements, 90.0, scene));
        return;
    }

    if (actionData == flipHorizontalText) {
        sendCommand(new FlipCommand(elements, 0, scene));
        return;
    }

    if (actionData == flipVerticalText) {
        sendCommand(new FlipCommand(elements, 1, scene));
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
