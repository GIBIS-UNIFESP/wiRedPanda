// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ElementEditor.h"

#include <cmath>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QImageReader>
#include <QKeyEvent>
#include <QMenu>

#include "App/Core/Common.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/UI/ElementEditorUI.h"
#include "App/UI/LabeledSlider.h"

ElementEditor::ElementEditor(QWidget *parent)
    : QWidget(parent)
    , m_ui(std::make_unique<ElementEditorUi>())
{
    m_ui->setupUi(this);
    // Start hidden/disabled; setCurrentElements() enables and shows it once a
    // selection is made in the scene.
    setEnabled(false);
    hide();

    // Trigger key accepts a single alphanumeric character (keyboard hotkey for
    // InputButton elements).  The validator prevents multi-char entries.
    m_ui->lineEditTrigger->setValidator(new QRegularExpressionValidator(QRegularExpression("[a-z]| |[A-Z]|[0-9]"), this));
    fillColorComboBox();

    // Install an event filter on every interactive widget so Tab/Shift+Tab
    // cycle through scene elements rather than moving focus between UI fields.
    m_ui->checkBoxLocked->installEventFilter(this);
    m_ui->comboBoxAudio->installEventFilter(this);
    m_ui->comboBoxColor->installEventFilter(this);
    m_ui->comboBoxInputSize->installEventFilter(this);
    m_ui->comboBoxOutputSize->installEventFilter(this);
    m_ui->comboBoxValue->installEventFilter(this);
    m_ui->sliderDelay->installEventFilter(this);
    m_ui->doubleSpinBoxFrequency->installEventFilter(this);
    m_ui->lineEditElementLabel->installEventFilter(this);
    m_ui->lineEditTrigger->installEventFilter(this);
    // The truth table editor is a floating dialog rather than embedded in the
    // panel so it doesn't push other properties off-screen for large tables.

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
    connect(m_ui->comboBoxOutputSize,     qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::outputIndexChanged);
    connect(m_ui->comboBoxValue,          &QComboBox::currentTextChanged,                   this, &ElementEditor::outputValueChanged);
    connect(m_ui->sliderDelay,            qOverload<int>(&QSlider::valueChanged),           this, &ElementEditor::apply);
    connect(m_ui->doubleSpinBoxFrequency, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ElementEditor::apply);
    connect(m_ui->lineEditElementLabel,   &QLineEdit::textChanged,                          this, &ElementEditor::apply);
    connect(m_ui->lineEditTrigger,        &QLineEdit::textChanged,                          this, &ElementEditor::triggerChanged);
    connect(m_ui->pushButtonAudioBox,     &QPushButton::clicked,                            this, &ElementEditor::audioBox);
    connect(m_ui->pushButtonChangeSkin,   &QPushButton::clicked,                            this, &ElementEditor::updateElementSkin);
    connect(m_ui->pushButtonDefaultSkin,  &QPushButton::clicked,                            this, &ElementEditor::defaultSkin);
    connect(m_ui->pushButtonTruthTable,   &QPushButton::clicked,                            this, &ElementEditor::truthTable);
}

ElementEditor::~ElementEditor()
{
}

// Helper: add a morph target to the submenu unless the element already IS that type
// (in which case morphing would be a no-op, so we skip it).
static QAction *addElementAction(QMenu *menu, GraphicElement *selectedElm, ElementType type, const bool hasSameType)
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
    // --- Build context menu ---
    // Items are added conditionally based on the capabilities of the current
    // selection.  String literals are captured for later identity comparison
    // because some menu items don't carry a data payload.
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
        menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/rename.svg")), renameText)->setData(renameText);
    }

    if (m_hasTrigger) {
        menu.addAction(QIcon(ElementFactory::pixmap(ElementType::InputButton)), triggerText)->setData(triggerText);
    }

    if (m_canChangeSkin) {
        menu.addAction(changeSkinText);
        menu.addAction(revertSkinText);
    }

    menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/rotateL.svg")), rotateLeftText)->setData(rotateLeftText);
    menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/rotateR.svg")), rotateRightText)->setData(rotateRightText);

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

    if (auto *selectedElm = qgraphicsitem_cast<GraphicElement *>(itemAtMouse); selectedElm && m_canMorph) {
        submenuMorph = menu.addMenu(morphMenuText);

        // --- Morph targets per element group ---
        // Only offer morph options that are structurally compatible (same input
        // count for gates, same group for inputs, same pin config for flip-flops).
        switch (selectedElm->elementGroup()) {
        case ElementGroup::Gate: {
            // 1-input gates can only morph to other 1-input types (Not / Node).
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
            // Each flip-flop/latch variant has a distinct pin count; morph only
            // to types that have the same number of inputs to preserve connections.
            // 2 inputs → DLatch; 4 inputs → DFF/TFF; 5 inputs → JKFF/SRFF.
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
            // 7/14/16-segment displays share a morph group; LED and Buzzer share
            // another.  Mixing between the two subgroups is not offered because
            // their port counts differ significantly.
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
        case ElementGroup::Other:
        case ElementGroup::Unknown: break;

        default:
            // Handle any unexpected enum values gracefully
            break;
        }

        // If no compatible morph targets were found, remove the empty submenu
        // so it doesn't appear as a greyed-out entry.
        if (submenuMorph->actions().empty()) {
            menu.removeAction(submenuMorph->menuAction());
        }
    }

    menu.addSeparator();

    if (m_hasElements) {
        QAction *copyAction = menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/copy.svg")), tr("Copy"));
        QAction *cutAction = menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/cut.svg")), tr("Cut"));

        connect(copyAction, &QAction::triggered, m_scene, &Scene::copyAction);
        connect(cutAction,  &QAction::triggered, m_scene, &Scene::cutAction);
    }

    QAction *deleteAction = menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/delete.svg")), tr("Delete"));
    connect(deleteAction, &QAction::triggered, m_scene, &Scene::deleteAction);

    // --- Dispatch the chosen action ---
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
        // Signal apply() to call elm->setSkin(true, ...) which resets to the
        // built-in SVG for this element type.
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
        // Changing the combobox text triggers the apply() slot via signal,
        // so no explicit apply() call is needed here.
        m_ui->comboBoxColor->setCurrentText(action->text());
        return;
    }

    // Copy/Cut/Delete are handled by their connected slots; nothing more to do.
    if ((actionText == tr("Copy")) || (actionText == tr("Cut")) || (actionText == tr("Delete"))) {
        return;
    }

    throw PANDACEPTION("Unknown context menu option.");
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
    // Disconnect from the old scene first; passing nullptr is valid and is used
    // when the active tab changes (disconnectTab in MainWindow).
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
    // Block signals while rebuilding to avoid triggering apply() for each
    // addItem() call, which would create spurious undo commands.
    QSignalBlocker blocker(m_ui->comboBoxColor);
    m_ui->comboBoxColor->clear();
    // item data stores the untranslated English colour name used internally
    // by GraphicElement::setColor(); item text is the translated display name.
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/Components/Output/Led/WhiteLed.svg")),  tr("White"),  "White");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/Components/Output/Led/RedLed.svg")),    tr("Red"),    "Red");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/Components/Output/Led/GreenLed.svg")),  tr("Green"),  "Green");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/Components/Output/Led/BlueLed.svg")),   tr("Blue"),   "Blue");
    m_ui->comboBoxColor->addItem(QIcon(QPixmap(":/Components/Output/Led/PurpleLed.svg")), tr("Purple"), "Purple");
}

void ElementEditor::retranslateUi()
{
    m_ui->retranslateUi(this);
    // Color names are translated strings; rebuild the combo box so current-language
    // names are shown.
    fillColorComboBox();

    // Refresh the editor with the current selection in case any visible text
    // (e.g. "Many values") needs to be re-translated.
    if (m_scene) {
        selectionChanged();
    }
}

void ElementEditor::setCurrentElements(const QList<GraphicElement *> &elements)
{
    m_elements = elements;
    // --- Reset capability flags ---
    // All flags start false; if elements is non-empty, they are set true and
    // then AND-reduced over the selection below.
    m_hasAudioBox = m_hasTruthTable = m_hasLabel = m_hasColors = m_hasAudio = m_hasFrequency = m_hasDelay = m_canChangeInputSize = m_canChangeOutputSize = m_hasTrigger = false;
    m_hasRotation = m_hasSameLabel = m_hasSameColors = m_hasSameFrequency = m_hasSameDelay = m_hasSameAudio = m_hasOnlyInputs = m_hasLatchedValue = false;
    m_hasSameInputSize = m_hasSameOutputSize = m_hasSameOutputValue = m_hasSameTrigger = m_canMorph = m_hasSameType = false;
    m_canChangeSkin = false;
    m_hasElements = false;

    if (elements.isEmpty()) {
        hide();
        m_ui->lineEditElementLabel->setText("");
        return;
    }

    bool sameCheckState = true;
    // Start all capabilities as true; each element in the loop ANDs them down.
    m_hasTruthTable = m_hasLabel = m_hasColors = m_hasAudio = m_hasFrequency = m_hasDelay = m_canChangeInputSize = m_canChangeOutputSize = m_hasTrigger = true;
    m_hasRotation = m_hasSameLabel = m_hasSameColors = m_hasSameFrequency = m_hasSameDelay = m_hasSameAudio = m_hasOnlyInputs = m_hasLatchedValue = true;
    m_hasSameInputSize = m_hasSameOutputSize = m_hasSameOutputValue = m_hasSameTrigger = m_canMorph = m_hasSameType = true;
    m_canChangeSkin = true;
    m_hasElements = true;
    m_hasAudioBox = true;

    show();
    // Disable during the update pass to prevent partial changes from being
    // emitted as undo commands while we're computing capabilities.
    setEnabled(false);
    // minimumInputs/Outputs accumulates the highest lower-bound across elements;
    // maximumInputs/Outputs accumulates the lowest upper-bound.  The intersection
    // of those ranges is the set of sizes valid for ALL selected elements.
    int minimumInputs = 0;
    // Start at a very high "infinity" so std::min finds the real maximum.
    int maximumInputs = 100000000;
    int minimumOutputs = 0;
    int maximumOutputs = 100000000;
    // maxCurrentOutputSize tracks the smallest current outputSize in the
    // selection; used to bound the "value" combobox for rotary inputs.
    int maxCurrentOutputSize = 100000000;
    auto *firstElement = m_elements.constFirst();
    auto *firstInput = qobject_cast<GraphicElementInput *>(firstElement);
    auto elementType = firstElement->elementType();

    for (auto *elm : std::as_const(m_elements)) {
        const auto group = elm->elementGroup();
        const auto firstGroup = firstElement->elementGroup();

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

        // Static inputs (Vcc/Gnd) and regular inputs share morph compatibility
        // because they occupy the same pin role in a circuit.
        bool sameElementGroup = (group == firstGroup);
        sameElementGroup |= (group == ElementGroup::Input && (firstGroup == ElementGroup::StaticInput));
        sameElementGroup |= (group == ElementGroup::StaticInput && (firstGroup == ElementGroup::Input));
        m_hasOnlyInputs &= (group == ElementGroup::Input);
        m_hasLatchedValue &= (group == ElementGroup::Input) && (elm->elementType() != ElementType::InputButton);
        m_canMorph &= sameElementGroup;
    }

    if (!m_hasSameType) {
        elementType = ElementType::Unknown;
    }

    // A size combobox is only useful when there is a valid range of options.
    m_canChangeInputSize = (minimumInputs < maximumInputs);
    // TruthTable output count is controlled by its own dialog, not this combobox.
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

    // Keep the placeholder "many colors" item in reserve so we can display it
    // when elements have different colors.  Remove it only when all match.
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

    m_ui->sliderDelay->setVisible(m_hasDelay);
    m_ui->sliderDelay->setEnabled(m_hasDelay);
    m_ui->labelDelay->setVisible(m_hasDelay);

    if (m_hasFrequency) {
        if (m_hasSameFrequency) {
            // Minimum 0.1 Hz prevents a frequency of 0, which would mean the
            // clock never toggles.  0.0 is only used as the sentinel value for
            // the "many frequencies" placeholder text below.
            m_ui->doubleSpinBoxFrequency->setMinimum(0.1);
            m_ui->doubleSpinBoxFrequency->setSpecialValueText({});
            m_ui->doubleSpinBoxFrequency->setValue(static_cast<double>(firstElement->frequency()));
        } else {
            // Lower the minimum to 0.0 so setValue(0.0) triggers the special
            // value text "many frequencies" without violating the validator.
            m_ui->doubleSpinBoxFrequency->setMinimum(0.0);
            m_ui->doubleSpinBoxFrequency->setSpecialValueText(m_manyFreq);
            m_ui->doubleSpinBoxFrequency->setValue(0.0);
        }
    }

    if (m_hasDelay) {
        if (m_hasSameDelay) {
            // Convert delay value (in fraction of period, -0.5 to 0.5) to slider value (-4 to 4)
            // Each step represents 1/8 of a period
            const int sliderValue = static_cast<int>(firstElement->delay() * 8.0f);
            m_ui->sliderDelay->setValue(sliderValue);
        } else {
            m_ui->sliderDelay->setValue(0);
        }
    }

    // For Demux, input size is derived from output size - hide input combobox
    if (m_hasSameType && elementType == ElementType::Demux) {
        m_canChangeInputSize = false;
    }

    /* Input size */
    m_ui->comboBoxInputSize->clear();
    m_ui->labelInputs->setVisible(m_canChangeInputSize);
    m_ui->comboBoxInputSize->setVisible(m_canChangeInputSize);
    m_ui->comboBoxInputSize->setEnabled(m_canChangeInputSize);

    if (m_canChangeInputSize && m_hasSameType && elementType == ElementType::Mux) {
        // For Mux, show data input count (select lines are auto-calculated)
        for (int dataInputs = 2; dataInputs <= 8; ++dataInputs) {
            int selectLines = 1;
            while ((1 << selectLines) < dataInputs) {
                selectLines++;
            }
            int totalInputs = dataInputs + selectLines;
            if (totalInputs >= minimumInputs && totalInputs <= maximumInputs) {
                m_ui->comboBoxInputSize->addItem(QString::number(dataInputs), totalInputs);
            }
        }
    } else {
        for (int port = minimumInputs; port <= maximumInputs; ++port) {
            m_ui->comboBoxInputSize->addItem(QString::number(port), port);
        }
    }

    if (m_ui->comboBoxInputSize->findText(m_manyIS) == -1) {
        m_ui->comboBoxInputSize->addItem(m_manyIS);
    }

    if (m_canChangeInputSize) {
        if (m_hasSameInputSize) {
            m_ui->comboBoxInputSize->removeItem(m_ui->comboBoxInputSize->findText(m_manyIS));
            const int idx = m_ui->comboBoxInputSize->findData(firstElement->inputSize());
            if (idx >= 0) {
                m_ui->comboBoxInputSize->setCurrentIndex(idx);
            }
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
        if (m_hasSameType && elementType == ElementType::Demux) {
            // Demux: show 2-8 output options, mirroring Mux's 2-8 data inputs
            for (int n = minimumOutputs; n <= maximumOutputs; ++n) {
                m_ui->comboBoxOutputSize->addItem(QString::number(n), n);
            }
        } else {
            m_ui->comboBoxOutputSize->addItem("2", 2);
            m_ui->comboBoxOutputSize->addItem("3", 3);
            m_ui->comboBoxOutputSize->addItem("4", 4);
            m_ui->comboBoxOutputSize->addItem("6", 6);
            m_ui->comboBoxOutputSize->addItem("8", 8);
            m_ui->comboBoxOutputSize->addItem("10", 10);
            m_ui->comboBoxOutputSize->addItem("12", 12);
            m_ui->comboBoxOutputSize->addItem("16", 16);
        }
    }

    if (m_ui->comboBoxOutputSize->findText(m_manyOS) == -1) {
        m_ui->comboBoxOutputSize->addItem(m_manyOS);
    }

    if (m_canChangeOutputSize && !m_hasTruthTable) {
        if (m_hasSameOutputSize) {
            m_ui->comboBoxOutputSize->removeItem(m_ui->comboBoxOutputSize->findText(m_manyOS));
            const int idx = m_ui->comboBoxOutputSize->findData(firstElement->outputSize());
            if (idx >= 0) {
                m_ui->comboBoxOutputSize->setCurrentIndex(idx);
            }
        } else {
            m_ui->comboBoxOutputSize->setCurrentText(m_manyOS);
        }
    }

    /* Output value (hidden for momentary inputs like InputButton) */
    m_ui->comboBoxValue->clear();
    m_ui->labelValue->setVisible(m_hasLatchedValue);
    m_ui->comboBoxValue->setVisible(m_hasLatchedValue);
    m_ui->comboBoxValue->setEnabled(m_hasLatchedValue);

    if (m_hasLatchedValue) {
        // Binary inputs have outputSize == 1 but must offer values 0 and 1,
        // so bump the upper bound to 2 in that case.
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

    if (m_hasLatchedValue) {
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
        // When a multi-selection has mixed lock states, show a partially-checked
        // state rather than incorrectly representing all as locked or unlocked.
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
        } else if (elements.size() == 1) {
            m_ui->lineCurrentAudioBox->setText(elements[0]->audio());
        }
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

    // Guard against recursive calls: signals from UI widgets (e.g. comboBox
    // currentIndexChanged) can fire during setCurrentElements() while the
    // widget is disabled, which would produce spurious undo entries.
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    // Snapshot current state into oldData before modifying anything.
    // UpdateCommand uses oldData for undo and captures the post-modification
    // state internally as newData when it is constructed.
    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    for (auto *elm : std::as_const(m_elements)) {
        // Save the pre-edit state of every element into the stream so the undo
        // command can restore all of them atomically.
        elm->save(stream);

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

        if (elm->hasDelay()) {
            // Convert slider value (-4 to 4) to phase delay (-0.5 to 0.5 as fraction of period)
            // Each step represents 1/8 of a period
            const float delayFraction = static_cast<float>(m_ui->sliderDelay->value()) / 8.0f;
            elm->setDelay(delayFraction);
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

    // Reset the one-shot skin update flag after applying to all elements.
    if (m_isUpdatingSkin) {
        m_isUpdatingSkin = false;
    }

    // sendCommand routes through the scene's undo stack; the scene emits
    // selectionChanged which calls update() automatically, so no explicit
    // refresh call is needed here.
    emit sendCommand(new UpdateCommand(m_elements, oldData, m_scene));
}

void ElementEditor::inputIndexChanged(const int index)
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    // Ignore selection of the placeholder "many input sizes" item so we don't
    // accidentally resize all selected elements to an invalid value.
    if (m_canChangeInputSize && (m_ui->comboBoxInputSize->currentText() != m_manyIS)) {
        emit sendCommand(new ChangeInputSizeCommand(m_elements, m_ui->comboBoxInputSize->currentData().toInt(), m_scene));
    }

    qCDebug(zero) << "Input size changed to " << index;
    // Refresh the editor because changing input count may affect which rows are
    // visible (e.g. the value combobox depends on output count).
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
            // InputRotary carries a multi-bit integer value; keep it enabled
            // (first arg true) and update the numeric output.
            if (auto *input = qobject_cast<InputRotary *>(elm)) {
                input->setOn(true, newValue);
            }
        } else {
            // Regular binary inputs treat any non-zero value as "on".
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
    // Force uppercase so the stored key sequence matches Qt's key names and is
    // compatible across keyboard layouts.
    m_ui->lineEditTrigger->setText(cmd.toUpper());
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
                // --- Tab navigation between elements ---
                // Tab/Shift+Tab while an editor field is focused cycles focus
                // through visible scene elements in reading order (left-to-right
                // then top-to-bottom), rather than between UI widgets.
                auto *elm = m_elements.constFirst();
                auto elements = m_scene->visibleElements();

                // Sort by Y first, then stable-sort by X so the net order is
                // row-by-row left-to-right (stable_sort preserves Y order within
                // the same X column after the second sort).
                std::stable_sort(elements.begin(), elements.end(), [](const auto &elm1, const auto &elm2) {
                    return elm1->pos().ry() < elm2->pos().ry();
                });

                std::stable_sort(elements.begin(), elements.end(), [](const auto &elm1, const auto &elm2) {
                    return elm1->pos().rx() < elm2->pos().rx();
                });

                const int elmPos = static_cast<int>(elements.indexOf(elm));
                qCDebug(zero) << "Pos = " << elmPos << " from " << elements.size();
                int step = 1;

                if (moveBack) {
                    step = -1;
                }

                // obj is one of the filtered child widgets; cast to QWidget so
                // we can later check isEnabled() against the same widget after
                // setCurrentElements() potentially hides/disables it.
                auto *widget = qobject_cast<QWidget *>(obj);
                // +elements.size() before the mod avoids negative modulo for step=-1.
                int pos = static_cast<int>((elements.size() + elmPos + step) % elements.size());

                // Advance until we find an element whose editor field is actually
                // enabled (some elements don't expose the field that currently
                // has focus, so we skip them).
                for (; pos != elmPos; pos = static_cast<int>((elements.size() + pos + step) % elements.size())) {
                    qCDebug(zero) << "Pos = " << pos;
                    elm = elements.at(pos);

                    setCurrentElements({elm});

                    if (widget->isEnabled()) {
                        break;
                    }
                }

                m_scene->clearSelection();

                // If no suitable element was found, stay on the original one.
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

    auto *truthtable = qobject_cast<TruthTable *>(m_elements[0]);
    if (!truthtable) {
        return;
    }

    // Only a single TruthTable element is supported at a time.

    int nInputs = truthtable->inputSize();
    int nOutputs = truthtable->outputSize();

    // --- Build / refresh table ---
    QStringList inputLabels;
    // Columns: nInputs input columns (A, B, C, …) + nOutputs output columns (S0, S1, …).
    m_table->setColumnCount(nInputs + nOutputs);
    // Rows: one per input combination — 2^nInputs.
    m_table->setRowCount(static_cast<int>(std::pow(2, nInputs)));

    for (int i = 0; i < nInputs; ++i) {
        inputLabels.append(QChar::fromLatin1(static_cast<char>('A' + i)));
    }

    for (int i = 0; i < truthtable->outputSize(); ++i) {
        inputLabels.append("S" + QString::number(i));
        m_table->setColumnWidth(nInputs + i, 14);
    }

    m_table->setHorizontalHeaderLabels(inputLabels);

    for (int i = 0; i < std::pow(2, nInputs); ++i) {
        for (int j = 0; j < nInputs; ++j) {
            m_table->setColumnWidth(j, 14);
            // Convert row index to binary string, zero-padded to nInputs digits.
            auto newItemValue = QString::number(i, 2);

            if (newItemValue.size() < nInputs) {
                newItemValue = newItemValue.rightJustified(nInputs, '0');
            }

            // Reuse existing items to avoid re-allocating on every refresh.
            if (m_table->item(i, j) == nullptr) {
                auto *newItem = new QTableWidgetItem(newItemValue.at(j), QTableWidgetItem::Type);
                newItem->setTextAlignment(Qt::AlignCenter);
                m_table->setItem(i, j, newItem);
                // Input columns are read-only; only output cells can be toggled.
                m_table->item(i, j)->setFlags(Qt::ItemIsEnabled);
            }

            m_table->item(i, j)->setText(newItemValue.at(j));
        }

        auto bitArray = truthtable->key();

        for (int z = 0; z < nOutputs; ++z) {
            // The key QBitArray stores all outputs interleaved: output z at row i
            // lives at index 256*z + i (256 rows max per output).
            const int output = bitArray.at(256 * z + i);

            if (m_table->item(i, nInputs + z) == nullptr) {
                auto *newOutItem = new QTableWidgetItem(QString(QChar::fromLatin1(static_cast<char>('0' + output))));
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

    // Input columns (< nInputs) are read-only; only output columns are editable.
    if (column < m_elements[0]->inputSize()) return;

    // Toggle the cell text immediately for visual feedback before the command
    // propagates through the undo stack.
    auto cellItem = m_table->item(row, column);
    cellItem->setText((cellItem->text() == "0") ? "1" : "0");

    auto truthtable = m_elements[0];
    const int nInputs = truthtable->inputSize();
    // Compute the flat bit index using the same layout as truthTable(): 256*outputCol + row.
    const int positionToChange = 256 * (column - nInputs) + row;

    emit sendCommand(new ToggleTruthTableOutputCommand(truthtable, positionToChange, m_scene, this));

    // Re-render the table so the model state and displayed text stay in sync.
    ElementEditor::truthTable();

    update();

    m_scene->setCircuitUpdateRequired();
}

void ElementEditor::audioBox()
{
    auto *audiobox = qobject_cast<AudioBox *>(m_elements[0]);
    if (!audiobox) {
        return;
    }

    const QString filePath = QFileDialog::getOpenFileName(this, tr("Select any audio"),
                                                    QString(), tr("Audio (*.mp3 *.mp4 *.wav *.ogg)"));

    if (filePath.isEmpty()) {
        return;
    }

    // Mirror the skin-copy pattern: if the chosen file lives outside the project
    // directory, copy it alongside the .panda file so the project is self-contained.
    QFileInfo fileInfo(filePath);
    QString audioPath = filePath;
    const QString ctxDir = m_scene ? m_scene->contextDir() : QString();
    if (!ctxDir.isEmpty() &&
        (fileInfo.absoluteDir() != QDir(ctxDir))) {
        const QString dest = ctxDir + "/" + fileInfo.fileName();
        if (!QFile::exists(dest)) {
            QFile::copy(filePath, dest);
        }
        audioPath = fileInfo.fileName();  // store only filename; resolved on load
    }

    audiobox->setAudio(audioPath);
    m_ui->lineCurrentAudioBox->setText(fileInfo.fileName());
}

void ElementEditor::defaultSkin()
{
    // Set flags before calling apply() so apply() sees them during the loop
    // and passes true/empty to elm->setSkin() for each selected element.
    m_isUpdatingSkin = true;
    m_isDefaultSkin = true;
    apply();
}

void ElementEditor::update()
{
    // Re-run setCurrentElements on the existing selection to refresh all
    // visibility flags and widget values after an external state change.
    setCurrentElements(m_elements);
}

void ElementEditor::updateTheme()
{
    // Tweak the group box border colour to match the active theme.
    // Light theme: rgb(216,216,216) — a light grey; Dark: rgb(66,66,66) — a dark grey.
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

