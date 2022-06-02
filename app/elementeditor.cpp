// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
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
    setVisible(false);

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
    connect(m_ui->comboBoxOutputSize,     &QComboBox::currentTextChanged,                   this, &ElementEditor::outputIndexChanged);
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

QAction *addElementAction(QMenu *menu, GraphicElement *firstElm, ElementType type, const bool hasSameType)
{
    if (hasSameType && firstElm->elementType() == type) {
        return nullptr;
    }

    QAction *action = menu->addAction(QIcon(ElementFactory::pixmap(type)), ElementFactory::translatedName(type));
    action->setData(static_cast<int>(type));
    return action;
}

void ElementEditor::contextMenu(QPoint screenPos)
{
    QMenu menu;
    QString changeSkinText(tr("Change skin to ..."));
    QString colorMenuText(tr("Change color to..."));
    QString freqActionText(tr("Change frequency"));
    QString morphMenuText(tr("Morph to..."));
    QString renameActionText(tr("Rename"));
    QString revertSkinText(tr("Set skin to default"));
    QString rotateActionText(tr("Rotate"));
    QString triggerActionText(tr("Change trigger"));

    if (m_hasLabel) {
        menu.addAction(QIcon(QPixmap(":/toolbar/rename.png")), renameActionText)->setData(renameActionText);
    }

    if (m_hasTrigger) {
        menu.addAction(QIcon(ElementFactory::pixmap(ElementType::InputButton)), triggerActionText)->setData(triggerActionText);
    }

    if (m_canChangeSkin) {
        menu.addAction(changeSkinText);
        menu.addAction(revertSkinText);
    }

    if (m_hasRotation) {
        menu.addAction(QIcon(QPixmap(":/toolbar/rotateR.png")), rotateActionText)->setData(rotateActionText);
    }

    if (m_hasFrequency) {
        menu.addAction(QIcon(ElementFactory::pixmap(ElementType::Clock)), freqActionText)->setData(freqActionText);
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

    if (m_canMorph) {
        submenuMorph = menu.addMenu(morphMenuText);
        GraphicElement *firstElm = m_elements.first();

        switch (firstElm->elementGroup()) {
        case ElementGroup::Gate: {
            if (firstElm->inputSize() == 1) {
                addElementAction(submenuMorph, firstElm, ElementType::Node, m_hasSameType);
                addElementAction(submenuMorph, firstElm, ElementType::Not, m_hasSameType);
            } else {
                addElementAction(submenuMorph, firstElm, ElementType::And, m_hasSameType);
                addElementAction(submenuMorph, firstElm, ElementType::Nand, m_hasSameType);
                addElementAction(submenuMorph, firstElm, ElementType::Nor, m_hasSameType);
                addElementAction(submenuMorph, firstElm, ElementType::Or, m_hasSameType);
                addElementAction(submenuMorph, firstElm, ElementType::Xnor, m_hasSameType);
                addElementAction(submenuMorph, firstElm, ElementType::Xor, m_hasSameType);
            }
            break;
        }

        case ElementGroup::StaticInput: [[fallthrough]];
        case ElementGroup::Input: {
            addElementAction(submenuMorph, firstElm, ElementType::Clock, m_hasSameType);
            addElementAction(submenuMorph, firstElm, ElementType::InputButton, m_hasSameType);
            addElementAction(submenuMorph, firstElm, ElementType::InputGnd, m_hasSameType);
            addElementAction(submenuMorph, firstElm, ElementType::InputRotary, m_hasSameType);
            addElementAction(submenuMorph, firstElm, ElementType::InputSwitch, m_hasSameType);
            addElementAction(submenuMorph, firstElm, ElementType::InputVcc, m_hasSameType);
            break;
        }

        case ElementGroup::Memory: {
            if (firstElm->inputSize() == 2) {
                addElementAction(submenuMorph, firstElm, ElementType::DLatch, m_hasSameType);
            } else if (firstElm->inputSize() == 4) {
                addElementAction(submenuMorph, firstElm, ElementType::DFlipFlop, m_hasSameType);
                addElementAction(submenuMorph, firstElm, ElementType::TFlipFlop, m_hasSameType);
            }
            break;
        }

        case ElementGroup::Output: {
            addElementAction(submenuMorph, firstElm, ElementType::Buzzer, m_hasSameType);
            addElementAction(submenuMorph, firstElm, ElementType::Led, m_hasSameType);
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
        QAction *copyAction = menu.addAction(QIcon(QPixmap(":/toolbar/copy.png")), tr("Copy"));
        QAction *cutAction = menu.addAction(QIcon(QPixmap(":/toolbar/cut.png")), tr("Cut"));

        connect(copyAction, &QAction::triggered, m_scene, &Scene::copyAction);
        connect(cutAction,  &QAction::triggered, m_scene, &Scene::cutAction);
    }

    QAction *deleteAction = menu.addAction(QIcon(QPixmap(":/toolbar/delete.png")), tr("Delete"));
    connect(deleteAction, &QAction::triggered, m_scene, &Scene::deleteAction);

    QAction *action = menu.exec(screenPos);

    if (action) {
        if (action->data().toString() == renameActionText) {
            renameAction();
        } else if (action->data().toString() == rotateActionText) {
            emit sendCommand(new RotateCommand(m_elements, 90.0, m_scene));
        } else if (action->data().toString() == triggerActionText) {
            changeTriggerAction();
        } else if (action->text() == changeSkinText) {
            // Reads a new sprite and applies it to the element
            updateElementSkin();
        } else if (action->text() == revertSkinText) {
            // Reset the icon to its default
            m_isDefaultSkin = true;
            m_isUpdatingSkin = true;
            apply();
        } else if (action->data().toString() == freqActionText) {
            m_ui->doubleSpinBoxFrequency->setFocus();
        } else if (submenuMorph && submenuMorph->actions().contains(action)) {
            ElementType type = static_cast<ElementType>(action->data().toInt());
            if (type != ElementType::Unknown) {
                emit sendCommand(new MorphCommand(m_elements, type, m_scene));
            }
        } else if (submenuColors && submenuColors->actions().contains(action)) {
            m_ui->comboBoxColor->setCurrentText(action->text());
        } else {
            qDebug(zero) << "uncaught text " + action->text();
        }
    }
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

    if (!fileDialog.exec()) {
        return;
    }

    auto files = fileDialog.selectedFiles();
    QString fileName = files.first();

    if (fileName.isEmpty()) {
        return;
    }

    qCDebug(zero) << "File name:" << fileName;
    m_isUpdatingSkin = true;
    m_skinName = fileName;
    m_isDefaultSkin = false;
    apply();
}

void ElementEditor::setScene(Scene *scene)
{
    m_scene = scene;
    connect(m_scene, &QGraphicsScene::selectionChanged, this,    &ElementEditor::selectionChanged);
    connect(m_scene, &Scene::contextMenuPos,            this,    &ElementEditor::contextMenu);
    connect(this,    &ElementEditor::sendCommand,       m_scene, &Scene::receiveCommand);
}

void ElementEditor::fillColorComboBox()
{
    m_ui->comboBoxColor->clear();
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/GreenLedOn.png")), tr("Green"), "Green");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/BlueLedOn.png")), tr("Blue"), "Blue");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/PurpleLedOn.png")), tr("Purple"), "Purple");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/RedLedOn.png")), tr("Red"), "Red");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/WhiteLedOn.png")), tr("White"), "White");
}

void ElementEditor::retranslateUi()
{
    m_ui->retranslateUi(this);
    fillColorComboBox();
}

void ElementEditor::setCurrentElements(const QList<GraphicElement *> &elms)
{
    m_elements = elms;
    m_hasLabel = m_hasColors = m_hasFrequency = m_canChangeInputSize = m_canChangeOutputSize = m_hasTrigger = m_hasAudio = false;
    m_hasRotation = m_hasSameLabel = m_hasSameColors = m_hasSameFrequency = m_hasSameAudio = m_hasOnlyInputs = false;
    m_hasSameInputSize = m_hasSameOutputSize = m_hasSameOutputValue = m_hasSameTrigger = m_canMorph = m_hasSameType = false;
    m_hasElements = false;

    if (!elms.isEmpty()) {
        bool sameCheckState = true;
        m_hasLabel = m_hasColors = m_hasAudio = m_hasFrequency = m_canChangeInputSize = m_canChangeOutputSize = m_hasTrigger = true;
        m_hasRotation = m_canChangeSkin = m_hasOnlyInputs = true;
        setVisible(true);
        setEnabled(false);
        int minimum_inputs = 0;
        int maximum_inputs = 100000000;
        int minimum_outputs = 0;
        int maximum_outputs = 100000000;
        int max_current_output_size = 100000000;
        m_hasSameLabel = m_hasSameColors = m_hasSameFrequency = true;
        m_hasSameInputSize = m_hasSameOutputSize = m_hasSameOutputValue = m_hasSameTrigger = m_canMorph = true;
        m_hasSameAudio = true;
        m_hasSameType = true;
        m_hasElements = true;
        GraphicElement *firstElement = m_elements.first();
        ElementType element_type = firstElement->elementType();

        for (auto *elm : qAsConst(m_elements)) {
            if (elm->elementType() != firstElement->elementType()) {
                element_type = ElementType::Unknown;
                break;
            }

            m_hasLabel &= elm->hasLabel();
            m_canChangeSkin &= elm->canChangeSkin();
            m_hasColors &= elm->hasColors();
            m_hasAudio &= elm->hasAudio();
            m_hasFrequency &= elm->hasFrequency();
            minimum_inputs = std::max(minimum_inputs, elm->minInputSize());
            maximum_inputs = std::min(maximum_inputs, elm->maxInputSize());
            minimum_outputs = std::max(minimum_outputs, elm->minOutputSize());
            maximum_outputs = std::min(maximum_outputs, elm->maxOutputSize());
            m_hasTrigger &= elm->hasTrigger();
            m_hasRotation &= elm->isRotatable();

            m_hasSameLabel &= elm->label() == firstElement->label();
            m_hasSameColors &= elm->color() == firstElement->color();
            m_hasSameFrequency &= qFuzzyCompare(elm->frequency(), firstElement->frequency());
            m_hasSameInputSize &= elm->inputSize() == firstElement->inputSize();
            m_hasSameOutputSize &= elm->outputSize() == firstElement->outputSize();
            max_current_output_size = std::min(max_current_output_size, elm->outputSize());

            if ((elm->elementGroup() == ElementGroup::Input) && (firstElement->elementGroup() == ElementGroup::Input)) {
                m_hasSameOutputValue &= dynamic_cast<Input *>(elm)->outputValue() == dynamic_cast<Input *>(firstElement)->outputValue();
                sameCheckState &= dynamic_cast<Input *>(elm)->isLocked() == dynamic_cast<Input *>(firstElement)->isLocked();
            }

            m_hasSameTrigger &= elm->trigger() == firstElement->trigger();
            m_hasSameType &= elm->elementType() == firstElement->elementType();
            m_hasSameAudio &= elm->audio() == firstElement->audio();
            m_canMorph &= elm->inputSize() == firstElement->inputSize();
            m_canMorph &= elm->outputSize() == firstElement->outputSize();

            bool sameElementGroup = elm->elementGroup() == firstElement->elementGroup();
            sameElementGroup |= (elm->elementGroup() == ElementGroup::Input && firstElement->elementGroup() == ElementGroup::StaticInput);
            sameElementGroup |= (elm->elementGroup() == ElementGroup::StaticInput && firstElement->elementGroup() == ElementGroup::Input);
            m_hasOnlyInputs &= elm->elementGroup() == ElementGroup::Input;
            m_canMorph &= sameElementGroup;
        }

        m_canChangeInputSize = (minimum_inputs < maximum_inputs);
        m_canChangeOutputSize = (minimum_outputs < maximum_outputs);
        /* Element type */
        m_ui->labelType->setText(ElementFactory::typeToTitleText(element_type));
        /* Labels */
        m_ui->lineEditElementLabel->setVisible(m_hasLabel);
        m_ui->lineEditElementLabel->setEnabled(m_hasLabel);
        m_ui->labelLabels->setVisible(m_hasLabel);

        if (m_hasLabel) {
            if (m_hasSameLabel) {
                m_ui->lineEditElementLabel->setText(firstElement->label());
            } else {
                m_ui->lineEditElementLabel->setText(m_manyLabels);
            }
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
                m_ui->doubleSpinBoxFrequency->setMinimum(0.5);
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

        for (int port = minimum_inputs; port <= maximum_inputs; ++port) {
            m_ui->comboBoxInputSize->addItem(QString::number(port), port);
        }

        if (m_ui->comboBoxInputSize->findText(m_manyIS) == -1) {
            m_ui->comboBoxInputSize->addItem(m_manyIS);
        }

        if (m_canChangeInputSize) {
            if (m_hasSameInputSize) {
                QString inputSize = QString::number(firstElement->inputSize());
                m_ui->comboBoxInputSize->removeItem(m_ui->comboBoxInputSize->findText(m_manyIS));
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
            m_ui->comboBoxOutputSize->addItem(QString::number(2), 2);
            m_ui->comboBoxOutputSize->addItem(QString::number(4), 4);
            m_ui->comboBoxOutputSize->addItem(QString::number(8), 8);
            m_ui->comboBoxOutputSize->addItem(QString::number(10), 10);
            m_ui->comboBoxOutputSize->addItem(QString::number(16), 16);
        }

        if (m_ui->comboBoxOutputSize->findText(m_manyOS) == -1) {
            m_ui->comboBoxOutputSize->addItem(m_manyOS);
        }

        if (m_canChangeOutputSize) {
            if (m_hasSameOutputSize) {
                QString outputSize = QString::number(firstElement->outputSize());
                m_ui->comboBoxOutputSize->removeItem(m_ui->comboBoxOutputSize->findText(m_manyOS));
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
            if (max_current_output_size == 1) {
                max_current_output_size++;
            }
            for (int val = 0; val < max_current_output_size; ++val) {
                m_ui->comboBoxValue->addItem(QString::number(val), val);
            }
        }

        if (m_ui->comboBoxValue->findText(m_manyOV) == -1) {
            m_ui->comboBoxValue->addItem(m_manyOV);
        }

        if (m_hasOnlyInputs) {
            if (m_hasSameOutputValue) {
                QString outputValue = QString::number(dynamic_cast<Input *>(firstElement)->outputValue());
                m_ui->comboBoxValue->removeItem(m_ui->comboBoxValue->findText(m_manyOV));
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
                if (dynamic_cast<Input *>(firstElement)->isLocked()) {
                    m_ui->checkBoxLocked->setCheckState(Qt::CheckState::Checked);
                } else {
                    m_ui->checkBoxLocked->setCheckState(Qt::CheckState::Unchecked);
                }
            } else {
                m_ui->checkBoxLocked->setCheckState(Qt::CheckState::PartiallyChecked);
            }
        }

        /* Trigger */
        m_ui->lineEditTrigger->setVisible(m_hasTrigger);
        m_ui->lineEditTrigger->setEnabled(m_hasTrigger);
        m_ui->labelTrigger->setVisible(m_hasTrigger);

        if (m_hasTrigger) {
            if (m_hasSameTrigger) {
                m_ui->lineEditTrigger->setText(firstElement->trigger().toString());
            } else {
                m_ui->lineEditTrigger->setText(m_manyTriggers);
            }
        }

        setEnabled(true);
        setVisible(true);
        /* Skin */
        m_ui->pushButtonChangeSkin->setVisible(m_canChangeSkin);
        m_ui->pushButtonDefaultSkin->setVisible(m_canChangeSkin);
    } else {
        m_hasElements = false;
        setVisible(false);
        m_ui->lineEditElementLabel->setText("");
    }
}

void ElementEditor::disable()
{
    m_hasElements = false;
    setVisible(false);
    m_ui->lineEditElementLabel->setText("");
}

void ElementEditor::selectionChanged()
{
    setCurrentElements(m_scene->selectedElements());
}

void ElementEditor::apply()
{
    qCDebug(three) << "Apply.";

    if ((m_elements.isEmpty()) || (!isEnabled())) {
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
    Q_UNUSED(index);

    if ((m_elements.isEmpty()) || (!isEnabled())) {
        return;
    }

    if (m_canChangeInputSize && (m_ui->comboBoxInputSize->currentText() != m_manyIS)) {
        emit sendCommand(new ChangeInputSizeCommand(m_elements, m_ui->comboBoxInputSize->currentData().toInt(), m_scene));
    }
}

void ElementEditor::outputIndexChanged(const QString &index)
{
    if ((m_elements.isEmpty()) || (!isEnabled())) {
        return;
    }

    if (m_canChangeOutputSize && (m_ui->comboBoxOutputSize->currentText() != m_manyOS)) {
        emit sendCommand(new ChangeOutputSizeCommand(m_elements, m_ui->comboBoxOutputSize->currentData().toInt(), m_scene));
    }

    qCDebug(zero) << "Output size changed to " << index.toInt();
    apply();
}

void ElementEditor::outputValueChanged(const QString &index)
{
    Q_UNUSED(index);

    if ((m_elements.isEmpty()) || (!isEnabled())) {
        return;
    }

    int new_value = m_ui->comboBoxValue->currentText().toInt();

    for (auto *elm : qAsConst(m_elements)) {
        if (elm->elementType() == ElementType::InputRotary) {
            dynamic_cast<InputRotary *>(elm)->setOn(true, new_value);
        } else {
            dynamic_cast<Input *>(elm)->setOn(new_value);
        }
    }

    apply();
}

void ElementEditor::inputLocked(const bool value)
{
    if ((m_elements.isEmpty()) || (!isEnabled())) {
        return;
    }

    for (auto *elm : qAsConst(m_elements)) {
        dynamic_cast<Input *>(elm)->setLocked(value);
    }

    qCDebug(zero) << "Input locked.";
    apply();
}

void ElementEditor::triggerChanged(const QString &cmd)
{
    m_ui->lineEditTrigger->setText(cmd.toUpper());
}

bool ElementEditor::eventFilter(QObject *obj, QEvent *event)
{
    auto *widget = qobject_cast<QWidget *>(obj);
    auto *keyEvent = dynamic_cast<QKeyEvent *>(event);

    if ((event->type() == QEvent::KeyPress) && keyEvent && (m_elements.size() == 1)) {
        bool moveFwd = (keyEvent->key() == Qt::Key_Tab);
        bool moveBack = (keyEvent->key() == Qt::Key_Backtab);

        if (moveBack || moveFwd) {
            auto *elm = m_elements.first();
            auto elms = m_scene->visibleElements();

            std::stable_sort(elms.begin(), elms.end(), [](const auto &elm1, const auto &elm2) {
                return elm1->pos().ry() < elm2->pos().ry();
            });
            std::stable_sort(elms.begin(), elms.end(), [](const auto &elm1, const auto &elm2) {
                return elm1->pos().rx() < elm2->pos().rx();
            });

            apply();

            int elmPos = elms.indexOf(elm);
            qCDebug(zero) << "Pos =" << elmPos << "from" << elms.size();
            int step = 1;

            if (moveBack) {
                step = -1;
            }

            int pos = (elms.size() + elmPos + step) % elms.size();

            for (; pos != elmPos; pos = ((elms.size() + pos + step) % elms.size())) {
                qCDebug(zero) << "Pos =" << pos;
                elm = elms[pos];

                setCurrentElements({elm});
                if (widget->isEnabled()) {
                    break;
                }
            }

            m_scene->clearSelection();

            if (!widget->isEnabled()) {
                elm = elms[elmPos];
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
