// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementeditor.h"
#include "ui_elementeditor.h"

#include "commands.h"
#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "inputrotary.h"
#include "scene.h"

#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMenu>
#include <cmath>

ElementEditor::ElementEditor(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::ElementEditor)
{
    m_ui->setupUi(this);
    setEnabled(false);
    hide();

    m_ui->lineEditTrigger->setValidator(new QRegularExpressionValidator(QRegularExpression("[a-z]| |[A-Z]|[0-9]"), this));
    fillColorComboBox();

    m_ui->checkBoxLocked->installEventFilter(this);
    m_ui->comboBoxAudio->installEventFilter(this);
    m_ui->comboBoxColor->installEventFilter(this);
    m_ui->comboBoxInputSize->installEventFilter(this);
    m_ui->comboBoxOutputSize->installEventFilter(this);
    m_ui->comboBoxValue->installEventFilter(this);
    m_ui->doubleSpinBoxFrequency->installEventFilter(this);
    m_ui->lineEditElementLabel->installEventFilter(this);
    m_ui->lineEditTrigger->installEventFilter(this);

    connect(m_ui->checkBoxLocked,         &QCheckBox::clicked,                              this, &ElementEditor::inputLocked);
    connect(m_ui->comboBoxAudio,          qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::apply);
    connect(m_ui->comboBoxColor,          qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::apply);
    connect(m_ui->comboBoxInputSize,      qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::inputIndexChanged);
    connect(m_ui->comboBoxOutputSize,     qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::outputIndexChanged);
    connect(m_ui->comboBoxValue,          &QComboBox::currentTextChanged,                   this, &ElementEditor::outputValueChanged);
    connect(m_ui->doubleSpinBoxFrequency, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ElementEditor::apply);
    connect(m_ui->lineEditElementLabel,   &QLineEdit::textChanged,                          this, &ElementEditor::apply);
    connect(m_ui->lineEditTrigger,        &QLineEdit::textChanged,                          this, &ElementEditor::triggerChanged);
    connect(m_ui->pushButtonChangeSkin,   &QPushButton::clicked,                            this, &ElementEditor::updateElementSkin);
    connect(m_ui->pushButtonDefaultSkin,  &QPushButton::clicked,                            this, &ElementEditor::defaultSkin);
}

ElementEditor::~ElementEditor()
{
    delete m_ui;
}

QAction *addElementAction(QMenu *menu, GraphicElement *selectedElm, ElementType type, const bool hasSameType)
{
    if (hasSameType && selectedElm->elementType() == type) {
        return nullptr;
    }

    QAction *action = menu->addAction(QIcon(ElementFactory::pixmap(type)), ElementFactory::translatedName(type));
    action->setData(static_cast<int>(type));
    return action;
}

void ElementEditor::contextMenu(QPoint screenPos, QGraphicsItem *itemAtMouse)
{
    QMenu menu;
    QString changeSkinText(tr("Change skin to ..."));
    QString colorMenuText(tr("Change color to..."));
    QString frequencyText(tr("Change frequency"));
    QString morphMenuText(tr("Morph to..."));
    QString renameText(tr("Rename"));
    QString revertSkinText(tr("Set skin to default"));
    QString rotateLeftText(tr("Rotate left"));
    QString rotateRightText(tr("Rotate right"));
    QString triggerText(tr("Change trigger"));

    if (m_hasLabel) {
        menu.addAction(QIcon(QPixmap(":/toolbar/rename.svg")), renameText)->setData(renameText);
    }

    if (m_hasTrigger) {
        menu.addAction(QIcon(ElementFactory::pixmap(ElementType::InputButton)), triggerText)->setData(triggerText);
    }

    if (m_canChangeSkin) {
        menu.addAction(changeSkinText);
        menu.addAction(revertSkinText);
    }

    if (m_hasRotation) {
        menu.addAction(QIcon(QPixmap(":/toolbar/rotateL.svg")), rotateLeftText)->setData(rotateLeftText);
        menu.addAction(QIcon(QPixmap(":/toolbar/rotateR.svg")), rotateRightText)->setData(rotateRightText);
    }

    if (m_hasFrequency) {
        menu.addAction(QIcon(ElementFactory::pixmap(ElementType::Clock)), frequencyText)->setData(frequencyText);
    }

    QMenu *submenuColors = nullptr;

    if (m_hasColors) {
        submenuColors = menu.addMenu(colorMenuText);
        for (int i = 0; i < m_ui->comboBoxColor->count(); ++i) {
            if (m_ui->comboBoxColor->currentIndex() != i) {
                submenuColors->addAction(m_ui->comboBoxColor->itemIcon(i), m_ui->comboBoxColor->itemText(i));
            }
        }
    }

    QMenu *submenuMorph = nullptr;

    if (auto *selectedElm = dynamic_cast<GraphicElement *>(itemAtMouse); selectedElm && m_canMorph) {
        submenuMorph = menu.addMenu(morphMenuText);

        switch (selectedElm->elementGroup()) {
        case ElementGroup::Gate: {
            if (selectedElm->inputSize() == 1) {
                addElementAction(submenuMorph, selectedElm, ElementType::Node, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Not, m_hasSameType);
            } else {
                addElementAction(submenuMorph, selectedElm, ElementType::And, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Nand, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Nor, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Or, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Xnor, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Xor, m_hasSameType);
            }
            break;
        }

        case ElementGroup::StaticInput: [[fallthrough]];
        case ElementGroup::Input: {
            addElementAction(submenuMorph, selectedElm, ElementType::Clock, m_hasSameType);
            addElementAction(submenuMorph, selectedElm, ElementType::InputButton, m_hasSameType);
            addElementAction(submenuMorph, selectedElm, ElementType::InputGnd, m_hasSameType);
            addElementAction(submenuMorph, selectedElm, ElementType::InputRotary, m_hasSameType);
            addElementAction(submenuMorph, selectedElm, ElementType::InputSwitch, m_hasSameType);
            addElementAction(submenuMorph, selectedElm, ElementType::InputVcc, m_hasSameType);
            break;
        }

        case ElementGroup::Memory: {
            if (selectedElm->inputSize() == 2) {
                addElementAction(submenuMorph, selectedElm, ElementType::DLatch, m_hasSameType);
            } else if (selectedElm->inputSize() == 4) {
                addElementAction(submenuMorph, selectedElm, ElementType::DFlipFlop, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::TFlipFlop, m_hasSameType);
            }
            break;
        }

        case ElementGroup::Output: {
            addElementAction(submenuMorph, selectedElm, ElementType::Buzzer, m_hasSameType);
            addElementAction(submenuMorph, selectedElm, ElementType::Led, m_hasSameType);
            break;
        }

        case ElementGroup::IC:      [[fallthrough]];
        case ElementGroup::Mux:     [[fallthrough]];
        case ElementGroup::Other:   [[fallthrough]];
        case ElementGroup::Unknown: break;
        }

        if (submenuMorph->actions().empty()) {
            menu.removeAction(submenuMorph->menuAction());
        }
    }

    menu.addSeparator();

    if (m_hasElements) {
        QAction *copyAction = menu.addAction(QIcon(QPixmap(":/toolbar/copy.svg")), tr("Copy"));
        QAction *cutAction = menu.addAction(QIcon(QPixmap(":/toolbar/cut.svg")), tr("Cut"));

        connect(copyAction, &QAction::triggered, m_scene, &Scene::copyAction);
        connect(cutAction,  &QAction::triggered, m_scene, &Scene::cutAction);
    }

    QAction *deleteAction = menu.addAction(QIcon(QPixmap(":/toolbar/delete.svg")), tr("Delete"));
    connect(deleteAction, &QAction::triggered, m_scene, &Scene::deleteAction);

    QAction *action = menu.exec(screenPos);

    if (!action) {
        return;
    }

    const QString actionData = action->data().toString();
    const QString actionText = action->text();

    if (actionData == renameText) {
        renameAction();
        return;
    }

    if (actionData == rotateLeftText) {
        emit sendCommand(new RotateCommand(m_elements, -90.0, m_scene));
        return;
    }

    if (actionData == rotateRightText) {
        emit sendCommand(new RotateCommand(m_elements, 90.0, m_scene));
        return;
    }

    if (actionData == triggerText) {
        changeTriggerAction();
        return;
    }

    if (actionText == changeSkinText) {
        updateElementSkin();
        return;
    }

    if (actionText == revertSkinText) {
        m_isDefaultSkin = true;
        m_isUpdatingSkin = true;
        apply();
        return;
    }

    if (actionData == frequencyText) {
        m_ui->doubleSpinBoxFrequency->setFocus();
        return;
    }

    if (submenuMorph && submenuMorph->actions().contains(action)) {
        if (auto type = static_cast<ElementType>(action->data().toInt()); type != ElementType::Unknown) {
            emit sendCommand(new MorphCommand(m_elements, type, m_scene));
        }
        return;
    }

    if (submenuColors && submenuColors->actions().contains(action)) {
        m_ui->comboBoxColor->setCurrentText(action->text());
        return;
    }

    if (actionText == tr("Copy") || actionText == tr("Cut") || actionText == tr("Delete")) {
        return;
    }

    throw Pandaception(tr("Unknown context menu option."));
}

void ElementEditor::renameAction()
{
    m_ui->lineEditElementLabel->setFocus();
    m_ui->lineEditElementLabel->selectAll();
}

void ElementEditor::changeTriggerAction()
{
    m_ui->lineEditTrigger->setFocus();
    m_ui->lineEditTrigger->selectAll();
}

void ElementEditor::updateElementSkin()
{
    QFileDialog fileDialog;
    fileDialog.setObjectName(tr("Open File"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(tr("Images (*.png *.gif *.jpg *.jpeg)"));

    if (fileDialog.exec() == QDialog::Rejected) {
        return;
    }

    const auto files = fileDialog.selectedFiles();
    QString fileName = files.constFirst();

    if (fileName.isEmpty()) {
        return;
    }

    qCDebug(zero) << tr("File name:") << fileName;
    m_isUpdatingSkin = true;
    m_skinName = fileName;
    m_isDefaultSkin = false;
    apply();
}

void ElementEditor::setScene(Scene *scene)
{
    if (m_scene) {
        disconnect(m_scene, &QGraphicsScene::selectionChanged, this,    &ElementEditor::selectionChanged);
        disconnect(m_scene, &Scene::contextMenuPos,            this,    &ElementEditor::contextMenu);
        disconnect(this,    &ElementEditor::sendCommand,       m_scene, &Scene::receiveCommand);
    }

    m_scene = scene;

    if (m_scene) {
        connect(m_scene, &QGraphicsScene::selectionChanged, this,    &ElementEditor::selectionChanged);
        connect(m_scene, &Scene::contextMenuPos,            this,    &ElementEditor::contextMenu);
        connect(this,    &ElementEditor::sendCommand,       m_scene, &Scene::receiveCommand);
    }
}

void ElementEditor::fillColorComboBox()
{
    QSignalBlocker blocker(m_ui->comboBoxColor);
    m_ui->comboBoxColor->clear();
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/GreenLedOn.svg")), tr("Green"), "Green");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/BlueLedOn.svg")), tr("Blue"), "Blue");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/PurpleLedOn.svg")), tr("Purple"), "Purple");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/RedLedOn.svg")), tr("Red"), "Red");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/WhiteLedOn.svg")), tr("White"), "White");
}

void ElementEditor::retranslateUi()
{
    m_ui->retranslateUi(this);
    fillColorComboBox();
}

void ElementEditor::setCurrentElements(const QList<GraphicElement *> &elms)
{
    m_elements = elms;
    m_hasLabel = m_hasColors = m_hasAudio = m_hasFrequency = m_canChangeInputSize = m_canChangeOutputSize = m_hasTrigger = false;
    m_hasRotation = m_hasSameLabel = m_hasSameColors = m_hasSameFrequency = m_hasSameAudio = m_hasOnlyInputs = false;
    m_hasSameInputSize = m_hasSameOutputSize = m_hasSameOutputValue = m_hasSameTrigger = m_canMorph = m_hasSameType = false;
    m_canChangeSkin = false;
    m_hasElements = false;

    if (elms.isEmpty()) {
        hide();
        m_ui->lineEditElementLabel->setText("");
        return;
    }

    bool sameCheckState = true;
    m_hasLabel = m_hasColors = m_hasAudio = m_hasFrequency = m_canChangeInputSize = m_canChangeOutputSize = m_hasTrigger = true;
    m_hasSameInputSize = m_hasSameOutputSize = m_hasSameOutputValue = m_hasSameTrigger = m_canMorph = m_hasSameType = true;
    m_hasRotation = m_hasSameLabel = m_hasSameColors = m_hasSameFrequency = m_hasSameAudio = m_hasOnlyInputs = true;
    m_canChangeSkin = true;
    m_hasElements = true;
    show();
    setEnabled(false);
    int minimumInputs = 0;
    int maximumInputs = 100000000;
    int minimumOutputs = 0;
    int maximumOutputs = 100000000;
    int maxCurrentOutputSize = 100000000;
    auto *firstElement = m_elements.constFirst();
    auto *firstInput = qobject_cast<GraphicElementInput *>(firstElement);
    auto elementType = firstElement->elementType();

    for (auto *elm : qAsConst(m_elements)) {
        const auto group = elm->elementGroup();
        const auto firstGroup = firstElement->elementGroup();

        m_hasLabel &= elm->hasLabel();
        m_canChangeSkin &= elm->canChangeSkin();
        m_hasColors &= elm->hasColors();
        m_hasAudio &= elm->hasAudio();
        m_hasFrequency &= elm->hasFrequency();
        minimumInputs = std::max(minimumInputs, elm->minInputSize());
        maximumInputs = std::min(maximumInputs, elm->maxInputSize());
        minimumOutputs = std::max(minimumOutputs, elm->minOutputSize());
        maximumOutputs = std::min(maximumOutputs, elm->maxOutputSize());
        m_hasTrigger &= elm->hasTrigger();
        m_hasRotation &= elm->isRotatable();

        m_hasSameLabel &= (elm->label() == firstElement->label());
        m_hasSameColors &= (elm->color() == firstElement->color());
        m_hasSameFrequency &= qFuzzyCompare(elm->frequency(), firstElement->frequency());
        m_hasSameInputSize &= (elm->inputSize() == firstElement->inputSize());
        m_hasSameOutputSize &= (elm->outputSize() == firstElement->outputSize());
        maxCurrentOutputSize = std::min(maxCurrentOutputSize, elm->outputSize());

        if (auto *elmInput = qobject_cast<GraphicElementInput *>(elm);
                elmInput && (group == ElementGroup::Input) && (firstElement->elementGroup() == ElementGroup::Input)) {
            m_hasSameOutputValue &= (elmInput->outputValue() == firstInput->outputValue());
            sameCheckState &= (elmInput->isLocked() == firstInput->isLocked());
        }

        m_hasSameTrigger &= (elm->trigger() == firstElement->trigger());
        m_hasSameType &= (elm->elementType() == firstElement->elementType());
        m_hasSameAudio &= (elm->audio() == firstElement->audio());
        m_canMorph &= (elm->inputSize() == firstElement->inputSize());
        m_canMorph &= (elm->outputSize() == firstElement->outputSize());

        bool sameElementGroup = (group == firstGroup);
        sameElementGroup |= (group == ElementGroup::Input && firstGroup == ElementGroup::StaticInput);
        sameElementGroup |= (group == ElementGroup::StaticInput && firstGroup == ElementGroup::Input);
        m_hasOnlyInputs &= (group == ElementGroup::Input);
        m_canMorph &= sameElementGroup;
    }

    if (!m_hasSameType) {
        elementType = ElementType::Unknown;
    }

    m_canChangeInputSize = (minimumInputs < maximumInputs);
    m_canChangeOutputSize = (minimumOutputs < maximumOutputs);
    /* Element type */
    m_ui->labelType->setText(ElementFactory::typeToTitleText(elementType));
    /* Labels */
    m_ui->lineEditElementLabel->setVisible(m_hasLabel);
    m_ui->lineEditElementLabel->setEnabled(m_hasLabel);
    m_ui->labelLabels->setVisible(m_hasLabel);

    if (m_hasLabel) {
        m_ui->lineEditElementLabel->setText(m_hasSameLabel ? firstElement->label() : m_manyLabels);
    }

    /* Color */
    m_ui->labelColor->setVisible(m_hasColors);
    m_ui->comboBoxColor->setVisible(m_hasColors);
    m_ui->comboBoxColor->setEnabled(m_hasColors);

    if (m_ui->comboBoxColor->findText(m_manyColors) == -1) {
        m_ui->comboBoxColor->addItem(m_manyColors);
    }

    if (m_hasColors) {
        if (m_hasSameColors) {
            m_ui->comboBoxColor->removeItem(m_ui->comboBoxColor->findText(m_manyColors));
            m_ui->comboBoxColor->setCurrentIndex(m_ui->comboBoxColor->findData(firstElement->color()));
        } else {
            m_ui->comboBoxColor->setCurrentText(m_manyColors);
        }
    }

    /* Sound */
    m_ui->labelAudio->setVisible(m_hasAudio);
    m_ui->comboBoxAudio->setVisible(m_hasAudio);
    m_ui->comboBoxAudio->setEnabled(m_hasAudio);

    if (m_ui->comboBoxAudio->findText(m_manyAudios) == -1) {
        m_ui->comboBoxAudio->addItem(m_manyAudios);
    }

    if (m_hasAudio) {
        if (m_hasSameAudio) {
            m_ui->comboBoxAudio->removeItem(m_ui->comboBoxAudio->findText(m_manyAudios));
            m_ui->comboBoxAudio->setCurrentText(firstElement->audio());
        } else {
            m_ui->comboBoxAudio->setCurrentText(m_manyAudios);
        }
    }

    /* Frequency */
    m_ui->doubleSpinBoxFrequency->setVisible(m_hasFrequency);
    m_ui->doubleSpinBoxFrequency->setEnabled(m_hasFrequency);
    m_ui->labelFrequency->setVisible(m_hasFrequency);

    if (m_hasFrequency) {
        if (m_hasSameFrequency) {
            m_ui->doubleSpinBoxFrequency->setMinimum(0.1);
            m_ui->doubleSpinBoxFrequency->setSpecialValueText({});
            m_ui->doubleSpinBoxFrequency->setValue(static_cast<double>(firstElement->frequency()));
        } else {
            m_ui->doubleSpinBoxFrequency->setMinimum(0.0);
            m_ui->doubleSpinBoxFrequency->setSpecialValueText(m_manyFreq);
            m_ui->doubleSpinBoxFrequency->setValue(0.0);
        }
    }

    /* Input size */
    m_ui->comboBoxInputSize->clear();
    m_ui->labelInputs->setVisible(m_canChangeInputSize);
    m_ui->comboBoxInputSize->setVisible(m_canChangeInputSize);
    m_ui->comboBoxInputSize->setEnabled(m_canChangeInputSize);

    for (int port = minimumInputs; port <= maximumInputs; ++port) {
        m_ui->comboBoxInputSize->addItem(QString::number(port), port);
    }

    if (m_ui->comboBoxInputSize->findText(m_manyIS) == -1) {
        m_ui->comboBoxInputSize->addItem(m_manyIS);
    }

    if (m_canChangeInputSize) {
        if (m_hasSameInputSize) {
            m_ui->comboBoxInputSize->removeItem(m_ui->comboBoxInputSize->findText(m_manyIS));
            QString inputSize = QString::number(firstElement->inputSize());
            m_ui->comboBoxInputSize->setCurrentText(inputSize);
        } else {
            m_ui->comboBoxInputSize->setCurrentText(m_manyIS);
        }
    }

    /* Output size */
    m_ui->comboBoxOutputSize->clear();
    m_ui->labelOutputs->setVisible(m_canChangeOutputSize);
    m_ui->comboBoxOutputSize->setVisible(m_canChangeOutputSize);
    m_ui->comboBoxOutputSize->setEnabled(m_canChangeOutputSize);

    if (m_canChangeOutputSize) {
        m_ui->comboBoxOutputSize->addItem("2", 2);
        m_ui->comboBoxOutputSize->addItem("3", 3);
        m_ui->comboBoxOutputSize->addItem("4", 4);
        m_ui->comboBoxOutputSize->addItem("6", 6);
        m_ui->comboBoxOutputSize->addItem("8", 8);
        m_ui->comboBoxOutputSize->addItem("10", 10);
        m_ui->comboBoxOutputSize->addItem("12", 12);
        m_ui->comboBoxOutputSize->addItem("16", 16);
    }

    if (m_ui->comboBoxOutputSize->findText(m_manyOS) == -1) {
        m_ui->comboBoxOutputSize->addItem(m_manyOS);
    }

    if (m_canChangeOutputSize) {
        if (m_hasSameOutputSize) {
            m_ui->comboBoxOutputSize->removeItem(m_ui->comboBoxOutputSize->findText(m_manyOS));
            QString outputSize = QString::number(firstElement->outputSize());
            m_ui->comboBoxOutputSize->setCurrentText(outputSize);
        } else {
            m_ui->comboBoxOutputSize->setCurrentText(m_manyOS);
        }
    }

    /* Output value */
    m_ui->comboBoxValue->clear();
    m_ui->labelValue->setVisible(m_hasOnlyInputs);
    m_ui->comboBoxValue->setVisible(m_hasOnlyInputs);
    m_ui->comboBoxValue->setEnabled(m_hasOnlyInputs);

    if (m_hasOnlyInputs) {
        if (maxCurrentOutputSize == 1) {
            maxCurrentOutputSize++;
        }

        for (int val = 0; val < maxCurrentOutputSize; ++val) {
            m_ui->comboBoxValue->addItem(QString::number(val), val);
        }
    }

    if (m_ui->comboBoxValue->findText(m_manyOV) == -1) {
        m_ui->comboBoxValue->addItem(m_manyOV);
    }

    if (m_hasOnlyInputs) {
        if (m_hasSameOutputValue) {
            m_ui->comboBoxValue->removeItem(m_ui->comboBoxValue->findText(m_manyOV));
            QString outputValue = QString::number(firstInput->outputValue());
            m_ui->comboBoxValue->setCurrentText(outputValue);
        } else {
            m_ui->comboBoxValue->setCurrentText(m_manyOV);
        }
    }

    /* Input locked */
    m_ui->labelLocked->setVisible(m_hasOnlyInputs);
    m_ui->checkBoxLocked->setVisible(m_hasOnlyInputs);
    m_ui->checkBoxLocked->setEnabled(m_hasOnlyInputs);

    if (m_hasOnlyInputs) {
        if (sameCheckState) {
            m_ui->checkBoxLocked->setCheckState(firstInput->isLocked() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
        } else {
            m_ui->checkBoxLocked->setCheckState(Qt::CheckState::PartiallyChecked);
        }
    }

    /* Trigger */
    m_ui->lineEditTrigger->setVisible(m_hasTrigger);
    m_ui->lineEditTrigger->setEnabled(m_hasTrigger);
    m_ui->labelTrigger->setVisible(m_hasTrigger);

    if (m_hasTrigger) {
        m_ui->lineEditTrigger->setText(m_hasSameTrigger ? firstElement->trigger().toString() : m_manyTriggers);
    }

    setEnabled(true);
    show();
    /* Skin */
    m_ui->pushButtonChangeSkin->setVisible(m_canChangeSkin);
    m_ui->pushButtonDefaultSkin->setVisible(m_canChangeSkin);
}

void ElementEditor::disable()
{
    m_hasElements = false;
    hide();
    m_ui->lineEditElementLabel->setText("");
}

void ElementEditor::selectionChanged()
{
    setCurrentElements(m_scene->selectedElements());
}

void ElementEditor::apply()
{
    qCDebug(three) << tr("Apply.");

    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);

    for (auto *elm : qAsConst(m_elements)) {
        elm->save(stream);

        if (elm->hasColors() && (m_ui->comboBoxColor->currentData().isValid())) {
            elm->setColor(m_ui->comboBoxColor->currentData().toString());
        }

        if (elm->hasAudio() && (m_ui->comboBoxAudio->currentText() != m_manyAudios)) {
            elm->setAudio(m_ui->comboBoxAudio->currentText());
        }

        if (elm->hasLabel() && (m_ui->lineEditElementLabel->text() != m_manyLabels)) {
            elm->setLabel(m_ui->lineEditElementLabel->text());
        }

        if (elm->hasFrequency() && (m_ui->doubleSpinBoxFrequency->text() != m_manyFreq)) {
            elm->setFrequency(static_cast<float>(m_ui->doubleSpinBoxFrequency->value()));
        }

        if (elm->hasTrigger() && (m_ui->lineEditTrigger->text() != m_manyTriggers)) {
            if (m_ui->lineEditTrigger->text().size() <= 1) {
                elm->setTrigger(QKeySequence(m_ui->lineEditTrigger->text()));
            }
        }

        if (m_isUpdatingSkin) {
            elm->setSkin(m_isDefaultSkin, m_skinName);
        }
    }

    if (m_isUpdatingSkin) {
        m_isUpdatingSkin = false;
    }

    emit sendCommand(new UpdateCommand(m_elements, itemData, m_scene));
}

void ElementEditor::inputIndexChanged(const int index)
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    if (m_canChangeInputSize && (m_ui->comboBoxInputSize->currentText() != m_manyIS)) {
        emit sendCommand(new ChangeInputSizeCommand(m_elements, m_ui->comboBoxInputSize->currentData().toInt(), m_scene));
    }

    qCDebug(zero) << tr("Input size changed to") << index;
    update();
}

void ElementEditor::outputIndexChanged(const int index)
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    if (m_canChangeOutputSize && (m_ui->comboBoxOutputSize->currentText() != m_manyOS)) {
        emit sendCommand(new ChangeOutputSizeCommand(m_elements, m_ui->comboBoxOutputSize->currentData().toInt(), m_scene));
    }

    qCDebug(zero) << tr("Output size changed to") << index;
    update();
}

void ElementEditor::outputValueChanged(const QString &value)
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    const int newValue = value.toInt();

    for (auto *elm : qAsConst(m_elements)) {
        if (elm->elementType() == ElementType::InputRotary) {
            if (auto *input = qobject_cast<InputRotary *>(elm)) {
                input->setOn(true, newValue);
            }
        } else {
            if (auto *input = qobject_cast<GraphicElementInput *>(elm)) {
                input->setOn(static_cast<bool>(newValue));
            }
        }
    }

    apply();
}

void ElementEditor::inputLocked(const bool value)
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    for (auto *elm : qAsConst(m_elements)) {
        if (auto *input = qobject_cast<GraphicElementInput *>(elm)) {
            input->setLocked(value);
        }
    }

    qCDebug(zero) << tr("Input locked.");
    apply();
}

void ElementEditor::triggerChanged(const QString &cmd)
{
    m_ui->lineEditTrigger->setText(cmd.toUpper());
    apply();
}

bool ElementEditor::eventFilter(QObject *obj, QEvent *event)
{
    auto *widget = qobject_cast<QWidget *>(obj);
    auto *keyEvent = dynamic_cast<QKeyEvent *>(event);

    if ((event->type() == QEvent::KeyPress) && keyEvent && (m_elements.size() == 1)) {
        bool moveFwd = (keyEvent->key() == Qt::Key_Tab);
        bool moveBack = (keyEvent->key() == Qt::Key_Backtab);

        if (moveBack || moveFwd) {
            auto *elm = m_elements.constFirst();
            auto elms = m_scene->visibleElements();

            std::stable_sort(elms.begin(), elms.end(), [](const auto &elm1, const auto &elm2) {
                return elm1->pos().ry() < elm2->pos().ry();
            });

            std::stable_sort(elms.begin(), elms.end(), [](const auto &elm1, const auto &elm2) {
                return elm1->pos().rx() < elm2->pos().rx();
            });

            int elmPos = elms.indexOf(elm);
            qCDebug(zero) << tr("Pos =") << elmPos << tr("from") << elms.size();
            int step = 1;

            if (moveBack) {
                step = -1;
            }

            int pos = (elms.size() + elmPos + step) % elms.size();

            for (; pos != elmPos; pos = ((elms.size() + pos + step) % elms.size())) {
                qCDebug(zero) << tr("Pos =") << pos;
                elm = elms.at(pos);

                setCurrentElements({elm});

                if (widget->isEnabled()) {
                    break;
                }
            }

            m_scene->clearSelection();

            if (!widget->isEnabled()) {
                elm = elms.at(elmPos);
            }

            elm->setSelected(true);
            elm->ensureVisible();
            widget->setFocus();
            event->accept();
            return true;
        }
    }

    /* pass the event on to the parent class */
    return QWidget::eventFilter(obj, event);
}

void ElementEditor::defaultSkin()
{
    m_isUpdatingSkin = true;
    m_isDefaultSkin = true;
    apply();
}

void ElementEditor::update()
{
    setCurrentElements(m_elements);
}
