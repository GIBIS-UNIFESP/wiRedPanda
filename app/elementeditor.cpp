// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementeditor.h"
#include "ui_elementeditor.h"

#include "audiobox.h"
#include "commands.h"
#include "common.h"
#include "elementfactory.h"
#include "inputrotary.h"
#include "node.h"
#include "qneconnection.h"
#include "scene.h"
#include "serialization.h"
#include "thememanager.h"
#include "truth_table.h"

#include <QDebug>
#include <QFileDialog>
#include <QImageReader>
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
    m_ui->doubleSpinBoxDelay->installEventFilter(this);
    m_ui->doubleSpinBoxFrequency->installEventFilter(this);
    m_ui->lineEditElementLabel->installEventFilter(this);
    m_ui->lineEditTrigger->installEventFilter(this);
    m_ui->spinBoxPriority->installEventFilter(this);

    m_tableBox = new QDialog(this);
    m_tableBox->setWindowFlags(Qt::Window);
    m_tableBox->setWindowTitle(tr("Truth Table"));
    m_tableBox->setModal(true);
    auto *tableLayout = new QGridLayout(m_tableBox);
    m_table = new QTableWidget(this);
    tableLayout->addWidget(m_table);
    m_tableBox->setLayout(tableLayout);

    connect(m_table,                      &QTableWidget::cellDoubleClicked,                 this, &ElementEditor::setTruthTableProposition);
    connect(m_ui->checkBoxLocked,         &QCheckBox::clicked,                              this, &ElementEditor::inputLocked);
    connect(m_ui->comboBoxAudio,          qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::apply);
    connect(m_ui->comboBoxColor,          qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::apply);
    connect(m_ui->comboBoxInputSize,      qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::inputIndexChanged);
    connect(m_ui->comboBoxNode,           &QComboBox::currentTextChanged,                   this, &ElementEditor::connectNode);
    connect(m_ui->comboBoxOutputSize,     qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::outputIndexChanged);
    connect(m_ui->comboBoxValue,          &QComboBox::currentTextChanged,                   this, &ElementEditor::outputValueChanged);
    connect(m_ui->doubleSpinBoxDelay,     qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ElementEditor::apply);
    connect(m_ui->doubleSpinBoxFrequency, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ElementEditor::apply);
    connect(m_ui->lineEditElementLabel,   &QLineEdit::editingFinished,                      this, &ElementEditor::mapNode);
    connect(m_ui->lineEditElementLabel,   &QLineEdit::textChanged,                          this, &ElementEditor::apply);
    connect(m_ui->lineEditTrigger,        &QLineEdit::textChanged,                          this, &ElementEditor::triggerChanged);
    connect(m_ui->pushButtonAudioBox,     &QPushButton::clicked,                            this, &ElementEditor::audioBox);
    connect(m_ui->pushButtonChangeSkin,   &QPushButton::clicked,                            this, &ElementEditor::updateElementSkin);
    connect(m_ui->pushButtonDefaultSkin,  &QPushButton::clicked,                            this, &ElementEditor::defaultSkin);
    connect(m_ui->pushButtonTruthTable,   &QPushButton::clicked,                            this, &ElementEditor::truthTable);
    connect(m_ui->spinBoxPriority,        qOverload<int>(&QSpinBox::valueChanged),          this, &ElementEditor::priorityChanged);
}

ElementEditor::~ElementEditor()
{
    delete m_ui;
}

QAction *addElementAction(QMenu *menu, GraphicElement *selectedElm, ElementType type, const bool hasSameType)
{
    if (hasSameType && (selectedElm->elementType() == type)) {
        return nullptr;
    }

    auto *action = menu->addAction(QIcon(ElementFactory::pixmap(type)), ElementFactory::translatedName(type));
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
    QString priorityText(tr("Change priority"));
    QString renameText(tr("Rename"));
    QString revertSkinText(tr("Set skin to default"));
    QString rotateLeftText(tr("Rotate left"));
    QString rotateRightText(tr("Rotate right"));
    QString triggerText(tr("Change trigger"));

    menu.addAction(priorityText)->setData(priorityText);

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

    menu.addAction(QIcon(QPixmap(":/toolbar/rotateL.svg")), rotateLeftText)->setData(rotateLeftText);
    menu.addAction(QIcon(QPixmap(":/toolbar/rotateR.svg")), rotateRightText)->setData(rotateRightText);

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
                break;
            }

            if (selectedElm->inputSize() == 4) {
                addElementAction(submenuMorph, selectedElm, ElementType::DFlipFlop, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::TFlipFlop, m_hasSameType);
                break;
            }

            if (selectedElm->inputSize() == 5) {
                addElementAction(submenuMorph, selectedElm, ElementType::JKFlipFlop, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::SRFlipFlop, m_hasSameType);
            }

            break;
        }

        case ElementGroup::Output: {
            if ((selectedElm->elementType() == ElementType::Display7) || (selectedElm->elementType() == ElementType::Display14) || (selectedElm->elementType() == ElementType::Display16)) {
                addElementAction(submenuMorph, selectedElm, ElementType::Display7, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Display14, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Display16, m_hasSameType);
            } else {
                addElementAction(submenuMorph, selectedElm, ElementType::Buzzer, m_hasSameType);
                addElementAction(submenuMorph, selectedElm, ElementType::Led, m_hasSameType);
            }

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

    if (actionData == priorityText) {
        updatePriorityAction();
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

    if ((actionText == tr("Copy")) || (actionText == tr("Cut")) || (actionText == tr("Delete"))) {
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

void ElementEditor::updatePriorityAction()
{
    m_ui->spinBoxPriority->setFocus();
    m_ui->spinBoxPriority->selectAll();
}

void ElementEditor::updateElementSkin()
{
    QFileDialog fileDialog;
    fileDialog.setObjectName(tr("Open File"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(tr("Images") + " (*." + QImageReader::supportedImageFormats().join(" *.") + ")");

    if (fileDialog.exec() == QDialog::Rejected) {
        return;
    }

    const auto files = fileDialog.selectedFiles();
    QString fileName = files.constFirst();

    if (fileName.isEmpty()) {
        return;
    }

    qCDebug(zero) << "File name: " << fileName;
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
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/led/WhiteLed.svg")),  tr("White"),  "White");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/led/RedLed.svg")),    tr("Red"),    "Red");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/led/GreenLed.svg")),  tr("Green"),  "Green");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/led/BlueLed.svg")),   tr("Blue"),   "Blue");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/led/PurpleLed.svg")), tr("Purple"), "Purple");
}

void ElementEditor::retranslateUi()
{
    m_ui->retranslateUi(this);
    fillColorComboBox();

    if (m_scene) {
        selectionChanged();
    }
}

void ElementEditor::setCurrentElements(const QList<GraphicElement *> &elements)
{
    m_elements = elements;
    m_hasAudioBox = m_hasTruthTable = m_hasNode = m_hasLabel = m_hasColors = m_hasAudio = m_hasFrequency = m_hasDelay = m_canChangeInputSize = m_canChangeOutputSize = m_hasTrigger = false;
    m_hasRotation = m_hasSameLabel = m_hasSameColors = m_hasSameFrequency = m_hasSameDelay = m_hasSameAudio = m_hasOnlyInputs = false;
    m_hasSameInputSize = m_hasSameOutputSize = m_hasSameOutputValue = m_hasSameTrigger = m_canMorph = m_hasSameType = false;
    m_canChangeSkin = m_hasSamePriority = false;
    m_hasElements = false;

    if (elements.isEmpty()) {
        hide();
        m_ui->lineEditElementLabel->setText("");
        return;
    }

    bool sameCheckState = true;
    m_hasNode = m_hasTruthTable = m_hasLabel = m_hasColors = m_hasAudio = m_hasFrequency = m_hasDelay = m_canChangeInputSize = m_canChangeOutputSize = m_hasTrigger = true;
    m_hasRotation = m_hasSameLabel = m_hasSameColors = m_hasSameFrequency = m_hasSameDelay = m_hasSameAudio = m_hasOnlyInputs = true;
    m_hasSameInputSize = m_hasSameOutputSize = m_hasSameOutputValue = m_hasSameTrigger = m_canMorph = m_hasSameType = true;
    m_canChangeSkin = m_hasSamePriority = true;
    m_hasElements = true;
    m_hasAudioBox = true;

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

    for (auto *elm : std::as_const(m_elements)) {
        const auto group = elm->elementGroup();
        const auto firstGroup = firstElement->elementGroup();

        m_hasNode &= elm->elementType() == ElementType::Node;
        m_hasTruthTable &= elm->hasTruthTable();
        m_hasLabel &= elm->hasLabel();
        m_canChangeSkin &= elm->canChangeSkin();
        m_hasColors &= elm->hasColors();
        m_hasAudio &= elm->hasAudio();
        m_hasAudioBox &= elm->hasAudioBox();
        m_hasFrequency &= elm->hasFrequency();
        m_hasDelay &= elm->hasDelay();
        minimumInputs = std::max(minimumInputs, elm->minInputSize());
        maximumInputs = std::min(maximumInputs, elm->maxInputSize());
        minimumOutputs = std::max(minimumOutputs, elm->minOutputSize());
        maximumOutputs = std::min(maximumOutputs, elm->maxOutputSize());
        m_hasTrigger &= elm->hasTrigger();
        m_hasRotation &= elm->isRotatable();

        m_hasSameLabel &= (elm->label() == firstElement->label());
        m_hasSameColors &= (elm->color() == firstElement->color());
        m_hasSameFrequency &= qFuzzyCompare(elm->frequency(), firstElement->frequency());
        m_hasSameDelay &= qFuzzyCompare(elm->delay(), firstElement->delay());
        m_hasSameInputSize &= (elm->inputSize() == firstElement->inputSize());
        m_hasSameOutputSize &= (elm->outputSize() == firstElement->outputSize());
        maxCurrentOutputSize = std::min(maxCurrentOutputSize, elm->outputSize());

        if (auto *elmInput = qobject_cast<GraphicElementInput *>(elm); elmInput && (group == ElementGroup::Input) && (firstElement->elementGroup() == ElementGroup::Input)) {
            m_hasSameOutputValue &= (elmInput->outputValue() == firstInput->outputValue());
            sameCheckState &= (elmInput->isLocked() == firstInput->isLocked());
        }

        m_hasSameTrigger &= (elm->trigger() == firstElement->trigger());
        m_hasSameType &= (elm->elementType() == firstElement->elementType());
        m_hasSameAudio &= (elm->audio() == firstElement->audio());
        m_hasSamePriority &= (elm->priority() == firstElement->priority());

        bool sameElementGroup = (group == firstGroup);
        sameElementGroup |= (group == ElementGroup::Input && (firstGroup == ElementGroup::StaticInput));
        sameElementGroup |= (group == ElementGroup::StaticInput && (firstGroup == ElementGroup::Input));
        m_hasOnlyInputs &= (group == ElementGroup::Input);
        m_canMorph &= sameElementGroup;
    }

    if (!m_hasSameType) {
        elementType = ElementType::Unknown;
    }

    m_canChangeInputSize = (minimumInputs < maximumInputs);
    m_canChangeOutputSize = (minimumOutputs < maximumOutputs && !m_hasTruthTable);
    /* Element type */
    m_ui->groupBox->setTitle(ElementFactory::typeToTitleText(elementType));
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

    /* Priority */
    if (m_hasSamePriority) {
        m_ui->spinBoxPriority->setValue(firstElement->priority());
    } else {
        m_ui->spinBoxPriority->setValue(0);
    }

    /* Frequency */
    m_ui->doubleSpinBoxFrequency->setVisible(m_hasFrequency);
    m_ui->doubleSpinBoxFrequency->setEnabled(m_hasFrequency);
    m_ui->labelFrequency->setVisible(m_hasFrequency);

    m_ui->doubleSpinBoxDelay->setVisible(m_hasDelay);
    m_ui->doubleSpinBoxDelay->setEnabled(m_hasDelay);
    m_ui->labelDelay->setVisible(m_hasDelay);

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

    if (m_hasDelay) {
        if (m_hasSameDelay) {
            m_ui->doubleSpinBoxDelay->setMinimum(0.0);
            m_ui->doubleSpinBoxDelay->setSpecialValueText({});
            m_ui->doubleSpinBoxDelay->setValue(static_cast<double>(firstElement->delay()));
        } else {
            m_ui->doubleSpinBoxDelay->setMinimum(0.0);
            m_ui->doubleSpinBoxDelay->setSpecialValueText(m_manyDelay);
            m_ui->doubleSpinBoxDelay->setValue(0.0);
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

    if (m_hasNode) {
        m_ui->comboBoxNode->clear();
        bool hasFoundConnection = false;
        bool isSourceNode = m_scene->nodeMapping.contains(firstElement->id());
        bool bNodeHasOutput = firstElement->outputPort()->connections().size() > 0;
        bool bNodeHasInput = firstElement->inputPort()->connections().size() > 0;


        // Search if node is already wireless conected to source node.
        // If so, set combobox text to source node label.
        // If it is a source node, don't show combobox.

        for (auto key : m_scene->nodeMapping.keys()) {
            auto *element = m_scene->element(key);
            if (element) {
                m_ui->comboBoxNode->addItem(element->label());
            } else {
                qDebug() << "Warning: Null element found in nodeMapping with key:" << key;
            }
        }

        auto set = m_scene->getNodeSet(firstElement->label());

        if (set.size() > 0) {
            for (auto item : set) {
                if (item.nodeId == firstElement->id()) {
                    auto newLabel = firstElement->label();
                    m_ui->comboBoxNode->setCurrentText(newLabel);
                    hasFoundConnection = true;
                    break;
                }
            }
        }

        m_ui->labelNode->setVisible(m_hasNode && !isSourceNode && bNodeHasOutput);
        m_ui->comboBoxNode->setVisible(m_hasNode && !isSourceNode && bNodeHasOutput);
        m_ui->comboBoxNode->setEnabled(m_hasNode && !isSourceNode && bNodeHasOutput);
        m_ui->comboBoxNode->addItem(QString(""));
        m_ui->lineEditElementLabel->setEnabled(!hasFoundConnection && bNodeHasInput && (isSourceNode || !bNodeHasOutput));

        if (!hasFoundConnection) {
            m_ui->comboBoxNode->setCurrentText("");
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

    if (m_canChangeOutputSize && !m_hasTruthTable) {
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
            ++maxCurrentOutputSize;
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

    /* AudioBox */
    m_ui->pushButtonAudioBox->setVisible(m_hasAudioBox);
    m_ui->pushButtonAudioBox->setEnabled(m_hasAudioBox);
    m_ui->labelAudioBox->setVisible(m_hasAudioBox);
    m_ui->lineCurrentAudioBox->setVisible(m_hasAudioBox);

    if (m_hasAudioBox) {
        if (elements.size() > 1) {
            m_ui->lineCurrentAudioBox->setText(m_manyAudios);
        }

        m_ui->lineCurrentAudioBox->setText(elements[0]->audio());
    }

    /* TruthTable */
    m_ui->pushButtonTruthTable->setVisible(m_hasTruthTable);
    m_ui->pushButtonTruthTable->setEnabled(m_hasTruthTable);

    setEnabled(true);
    show();

    /* Skin */
    m_ui->pushButtonChangeSkin->setVisible(m_canChangeSkin);
    m_ui->pushButtonDefaultSkin->setVisible(m_canChangeSkin);
}

void ElementEditor::selectionChanged()
{
    setCurrentElements(m_scene->selectedElements());
}

void ElementEditor::apply()
{
    qCDebug(three) << "Apply.";

    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    for (auto *elm : std::as_const(m_elements)) {
        elm->save(stream);

        elm->setPriority(m_ui->spinBoxPriority->value());

        if (elm->hasColors() && m_ui->comboBoxColor->currentData().isValid()) {
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

        if (elm->hasDelay() && (m_ui->doubleSpinBoxDelay->text() != m_manyDelay)) {
            elm->setDelay(static_cast<float>(m_ui->doubleSpinBoxDelay->value()));
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

    emit sendCommand(new UpdateCommand(m_elements, oldData, m_scene));
}

void ElementEditor::inputIndexChanged(const int index)
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    if (m_canChangeInputSize && (m_ui->comboBoxInputSize->currentText() != m_manyIS)) {
        emit sendCommand(new ChangeInputSizeCommand(m_elements, m_ui->comboBoxInputSize->currentData().toInt(), m_scene));
    }

    qCDebug(zero) << "Input size changed to " << index;
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

    qCDebug(zero) << "Output size changed to " << index;
    update();
}

void ElementEditor::outputValueChanged(const QString &value)
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    const int newValue = value.toInt();

    for (auto *elm : std::as_const(m_elements)) {
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

    for (auto *elm : std::as_const(m_elements)) {
        if (auto *input = qobject_cast<GraphicElementInput *>(elm)) {
            input->setLocked(value);
        }
    }

    qCDebug(zero) << "Input locked.";
    apply();
}

void ElementEditor::triggerChanged(const QString &cmd)
{
    m_ui->lineEditTrigger->setText(cmd.toUpper());
    apply();
}

void ElementEditor::priorityChanged(const int value)
{
    m_ui->spinBoxPriority->setValue(value);
    apply();
}

bool ElementEditor::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = dynamic_cast<QKeyEvent *>(event);

        if (keyEvent && (m_elements.size() == 1)) {
            const bool moveFwd = (keyEvent->key() == Qt::Key_Tab);
            const bool moveBack = (keyEvent->key() == Qt::Key_Backtab);

            if (moveBack || moveFwd) {
                auto *elm = m_elements.constFirst();
                auto elements = m_scene->visibleElements();

                std::stable_sort(elements.begin(), elements.end(), [](const auto &elm1, const auto &elm2) {
                    return elm1->pos().ry() < elm2->pos().ry();
                });

                std::stable_sort(elements.begin(), elements.end(), [](const auto &elm1, const auto &elm2) {
                    return elm1->pos().rx() < elm2->pos().rx();
                });

                const int elmPos = elements.indexOf(elm);
                qCDebug(zero) << "Pos = " << elmPos << " from " << elements.size();
                int step = 1;

                if (moveBack) {
                    step = -1;
                }

                auto *widget = qobject_cast<QWidget *>(obj);
                int pos = (elements.size() + elmPos + step) % elements.size();

                for (; pos != elmPos; pos = ((elements.size() + pos + step) % elements.size())) {
                    qCDebug(zero) << "Pos = " << pos;
                    elm = elements.at(pos);

                    setCurrentElements({elm});

                    if (widget->isEnabled()) {
                        break;
                    }
                }

                m_scene->clearSelection();

                if (!widget->isEnabled()) {
                    elm = elements.at(elmPos);
                }

                elm->setSelected(true);
                elm->ensureVisible();
                widget->setFocus();
                event->accept();
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

void ElementEditor::truthTable()
{
    if (!m_hasTruthTable) return;

    auto *truthtable = dynamic_cast<TruthTable *>(m_elements[0]);

    // Assuming only one element selected for now...

    int nInputs = truthtable->inputSize();
    int nOutputs = truthtable->outputSize();

    QStringList inputLabels;
    m_table->setColumnCount(nInputs + nOutputs);
    m_table->setRowCount(std::pow(2, nInputs));

    for (int i = 0; i < nInputs; ++i) {
        inputLabels.append(QChar::fromLatin1('A' + i));
    }

    for (int i = 0; i < truthtable->outputSize(); ++i) {
        inputLabels.append("S" + QString::number(i));
        m_table->setColumnWidth(nInputs + i, 14);
    }

    m_table->setHorizontalHeaderLabels(inputLabels);

    for (int i = 0; i < std::pow(2, nInputs); ++i) {
        for (int j = 0; j < nInputs; ++j) {
            m_table->setColumnWidth(j, 14);
            auto newItemValue = QString::number(i, 2);

            if (newItemValue.size() < nInputs) {
                newItemValue = newItemValue.rightJustified(nInputs, '0');
            }

            if (m_table->item(i, j) == nullptr) {
                auto *newItem = new QTableWidgetItem(newItemValue.at(j), QTableWidgetItem::Type);
                newItem->setTextAlignment(Qt::AlignCenter);
                m_table->setItem(i, j, newItem);
                m_table->item(i, j)->setFlags(Qt::ItemIsEnabled);
            }

            m_table->item(i, j)->setText(newItemValue.at(j));
        }

        auto bitArray = truthtable->key();

        for (int z = 0; z < nOutputs; ++z) {
            const int output = bitArray.at(256 * z + i);

            if (m_table->item(i, nInputs + z) == nullptr) {
                auto *newOutItem = new QTableWidgetItem(QString(QChar::fromLatin1('0' + output)));
                newOutItem->setTextAlignment(Qt::AlignCenter);
                m_table->setItem(i, nInputs + z, newOutItem);
                m_table->item(i, nInputs + z)->setFlags(Qt::ItemIsEnabled);
            }

            m_table->item(i, nInputs + z)->setText(QString::number(output));
        }
    }

    m_tableBox->show();
}

void ElementEditor::setTruthTableProposition(const int row, const int column)
{
    if (m_elements.size() > 1) return;

    if (column < m_elements[0]->inputSize()) return;

    auto cellItem = m_table->item(row, column);
    cellItem->setText((cellItem->text() == "0") ? "1" : "0");

    auto truthtable = m_elements[0];
    const int nInputs = truthtable->inputSize();
    const int positionToChange = 256 * (column - nInputs) + row;

    emit sendCommand(new ToggleTruthTableOutputCommand(truthtable, positionToChange, m_scene, this));

    ElementEditor::truthTable();

    update();

    m_scene->setCircuitUpdateRequired();
}

void ElementEditor::audioBox()
{
    auto *audiobox = dynamic_cast<AudioBox *>(m_elements[0]);

    const QString filePath = QFileDialog::getOpenFileName(this, tr("Select any audio"),
                                                    QString(), tr("Audio (*.mp3 *.mp4 *.wav *.ogg)"));

    audiobox->setAudio(filePath);
    m_ui->lineCurrentAudioBox->setText(QFileInfo(filePath).fileName());
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

void ElementEditor::updateTheme()
{
    const QString borderColor = (ThemeManager::theme() == Theme::Light) ? "216" : "66";
    const QString styleSheet =
            "QGroupBox {"
            "   border: 1px solid rgb(%1,%1,%1);"
            "   border-radius: 2px;"
            "   margin-top: 8px;"
            "}"
            ""
            "QGroupBox::title {"
            "   subcontrol-origin: margin;"
            "   subcontrol-position: top;"
            "}";

    m_ui->groupBox->setStyleSheet(styleSheet.arg(borderColor));
}

void ElementEditor::mapNode()
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    auto *selectedNode = m_elements[0];

    if (!selectedNode) {
        qDebug() << "Warning: Selected node is null in mapNode()";
        return;
    }

    if (selectedNode->elementType() != ElementType::Node) { 
        return; 
    }

    // If source node already exists and the new label is different, remove and insert new.
    const QString label = m_ui->lineEditElementLabel->text();
    auto thisNodeSet = m_scene->nodeMapping.value(selectedNode->id());
    auto nodeSet = m_scene->getNodeSet(label, {selectedNode->id()});
    const bool isSourceNode = m_scene->isSourceNode(selectedNode);
    const bool isSourceNodeLabelTaken = nodeSet.size() != 0;

    if (isSourceNodeLabelTaken) {
        if (thisNodeSet != nodeSet) {
            m_ui->lineEditElementLabel->setStyleSheet("QLineEdit {color: red}");
        }

        return;
    }

    m_ui->lineEditElementLabel->setStyleSheet("");

    // If new souceNodeText is empty and label is empty, destroy the connection
    if (isSourceNode && label == "") {
        m_scene->nodeMapping.remove(selectedNode->id());
        m_scene->deleteNodeSetConnections(&thisNodeSet);
        m_scene->nodeMapping.insert(selectedNode->id(), thisNodeSet);
        return;
    }

    if (isSourceNode) {
        selectedNode->setLabel(label);
        const auto childNodesSet = m_scene->nodeMapping.value(selectedNode->id());

        // Update labels on all connected child nodes
        QList<GraphicElement*> affectedNodes;
        for (auto &childNodesPair : childNodesSet.values()) {
            const int childNodeId = childNodesPair.nodeId;
            auto *childNode = m_scene->element(childNodeId);
            if (childNode) {
                childNode->setLabel(label);
                affectedNodes.append(childNode);
            }
        }

        // Refresh UI for the source node
        setCurrentElements({selectedNode});
        
        // Update combobox with new label
        refreshWirelessCombobox();
        
        // Notify other ElementEditor instances that nodes have changed
        // This ensures any open editors show the updated labels
        for (auto *affectedNode : affectedNodes) {
            affectedNode->update(); // Trigger visual update
        }
        
        return;
    }

    QSet<Destination> set;

    if (selectedNode->inputPort()->connections().size() > 0) {
        m_scene->nodeMapping.insert(selectedNode->id(), set);
    }

    setCurrentElements({selectedNode});
}

void ElementEditor::connectNode(const QString &label)
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    auto *selectedNode = m_elements[0];
    
    if (!selectedNode) {
        qDebug() << "Warning: Selected node is null in connectNode()";
        return;
    }
    
    if (selectedNode->elementType() != ElementType::Node) {
        qDebug() << "Warning: Selected element is not a Node in connectNode()";
        return;
    }
    
    // Validate label input
    if (label.length() > 50) { // Reasonable max length
        qDebug() << "Warning: Node label too long in connectNode()";
        return;
    }

    if (label != "" && selectedNode->label() == label) {
        setCurrentElements({selectedNode});
        return;
    }

    auto currentNodeSet = m_scene->getNodeSet(selectedNode->label());
    auto nextNodeSet = m_scene->getNodeSet(label);
    int currentSourceNodeId = -1;
    int nextSourceNodeId = -1;

    for (auto sourceNodeId : m_scene->nodeMapping.keys()) {
        auto *sourceElement = m_scene->element(sourceNodeId);
        if (!sourceElement) {
            qDebug() << "Warning: Null element found in nodeMapping with id:" << sourceNodeId;
            continue;
        }
        
        if (sourceElement->label() == label) {
            nextSourceNodeId = sourceNodeId;
        }

        if (sourceElement->label() == selectedNode->label()) {
            currentSourceNodeId = sourceNodeId;
        }
    }

    // auto nodeIds = m_scene->nodeMapping.keys();

    // In the case below, need to destroy connection
    if (selectedNode->label() != "" && (label == "" || label != selectedNode->label())) {
        if (currentNodeSet.size()) {
            m_scene->nodeMapping.remove(currentSourceNodeId);
            m_scene->deleteNodeSetConnections(&currentNodeSet, selectedNode->id());
            m_scene->nodeMapping.insert(currentSourceNodeId, currentNodeSet);
        }

        if (label == "") {
            m_ui->comboBoxNode->setCurrentText("");
            selectedNode->setLabel("");
            setCurrentElements({selectedNode});
            return;
        }
    }

    m_scene->nodeMapping.remove(nextSourceNodeId);
    nextNodeSet.insert(Destination{-1, selectedNode->id()});


    // Create a copy to avoid modifying collection during iteration
    QSet<Destination> nodesToProcess = nextNodeSet;
    
    for (auto pair : nodesToProcess) {
        auto *sourceElement = m_scene->element(nextSourceNodeId);
        auto *destElement = m_scene->element(pair.nodeId);
        
        if (!sourceElement || !destElement) {
            qDebug() << "Warning: Null element found in wireless connection - sourceId:" << nextSourceNodeId << "destId:" << pair.nodeId;
            continue;
        }
        
        auto *sourceNode = qobject_cast<Node *>(sourceElement);
        auto *destNode = qobject_cast<Node *>(destElement);
        
        if (!sourceNode || !destNode) {
            qDebug() << "Warning: Element is not a Node in wireless connection";
            continue;
        }

        if (destNode->inputPort()->connections().size() == 0) {
            auto *connection = new QNEConnection();
            connection->setWireless(true);
            connection->setStartPort(sourceNode->outputPort());
            connection->setEndPort(destNode->inputPort());
            m_scene->makeConnectionNode(connection);

            // Update both port and element wireless flags consistently
            destNode->inputPort()->setHasWirelessConnection(true);
            destNode->setIsWireless(true);
            
            // Safely update the actual set (not the copy)
            const int nodeId = pair.nodeId;
            nextNodeSet.remove(pair);
            nextNodeSet.insert(Destination{connection->id(), nodeId});

            selectedNode->setLabel(label);
        }
    }

    m_scene->nodeMapping.insert(nextSourceNodeId, nextNodeSet);
    setCurrentElements({selectedNode});
}

void ElementEditor::refreshWirelessCombobox()
{
    if (!m_hasNode || !m_ui->comboBoxNode->isVisible()) {
        return;
    }
    
    // Store current selection
    QString currentSelection = m_ui->comboBoxNode->currentText();
    
    // Repopulate combobox
    m_ui->comboBoxNode->clear();
    m_ui->comboBoxNode->addItem("");
    
    for (auto key : m_scene->nodeMapping.keys()) {
        auto *element = m_scene->element(key);
        if (element) {
            QString label = element->label();
            if (!label.isEmpty()) {
                m_ui->comboBoxNode->addItem(label);
            }
        }
    }
    
    // Restore selection if it still exists
    int index = m_ui->comboBoxNode->findText(currentSelection);
    if (index >= 0) {
        m_ui->comboBoxNode->setCurrentIndex(index);
    }
}
