// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementeditor.h"

#include <cmath>

#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMenu>

#include "commands.h"
#include "common.h"
#include "editor.h"
#include "elementfactory.h"
#include "inputrotary.h"
#include "ui_elementeditor.h"
#include "scene.h"

ElementEditor::ElementEditor(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::ElementEditor)
{
    m_manyLabels = tr("<Many labels>");
    m_manyColors = tr("<Many colors>");
    m_manyIS = tr("<Many values>");
    m_manyOS = tr("<Many values>");
    m_manyOV = tr("<Many values>");
    m_manyFreq = tr("<Many values>");
    m_manyTriggers = tr("<Many triggers>");
    m_manyAudios = tr("<Many sounds>");
    m_defaultSkin = true;
    m_updatingSkin = false;

    m_ui->setupUi(this);
    setEnabled(false);
    setVisible(false);

    m_ui->lineEditTrigger->setValidator(new QRegExpValidator(QRegExp("[a-z]| |[A-Z]|[0-9]"), this));
    fillColorComboBox();
    m_ui->lineEditElementLabel->installEventFilter(this);
    m_ui->lineEditTrigger->installEventFilter(this);
    m_ui->comboBoxColor->installEventFilter(this);
    m_ui->comboBoxInputSz->installEventFilter(this);
    m_ui->comboBoxOutputSz->installEventFilter(this);
    m_ui->comboBoxValue->installEventFilter(this);
    m_ui->doubleSpinBoxFrequency->installEventFilter(this);
    m_ui->comboBoxAudio->installEventFilter(this);
    m_ui->checkBoxLocked->installEventFilter(this);

    connect(m_ui->lineEditElementLabel, &QLineEdit::editingFinished,        this, &ElementEditor::apply);
    connect(m_ui->doubleSpinBoxFrequency, &QDoubleSpinBox::editingFinished, this, &ElementEditor::apply);
    connect(m_ui->lineEditTrigger, &QLineEdit::editingFinished,             this, &ElementEditor::apply);
    connect(m_ui->lineEditTrigger, &QLineEdit::textChanged,                 this, &ElementEditor::triggerChanged);
    connect(m_ui->pushButtonChangeSkin, &QPushButton::clicked,              this, &ElementEditor::updateElementSkin);
    connect(m_ui->pushButtonDefaultSkin, &QPushButton::clicked,             this, &ElementEditor::defaultSkin);
    connect(m_ui->pushButtonCustomConfig, &QPushButton::clicked,             this, &ElementEditor::openCustomConfig);

    connect(m_ui->comboBoxColor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ElementEditor::apply);
    connect(m_ui->comboBoxAudio, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ElementEditor::apply);
    connect(m_ui->comboBoxInputSz, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ElementEditor::inputIndexChanged);
    connect(m_ui->comboBoxOutputSz, &QComboBox::currentTextChanged, this, &ElementEditor::outputIndexChanged);
    connect(m_ui->comboBoxValue, &QComboBox::currentTextChanged, this, &ElementEditor::outputValueChanged);
    connect(m_ui->checkBoxLocked, &QCheckBox::clicked, this, &ElementEditor::inputLocked);
}

ElementEditor::~ElementEditor()
{
    delete m_ui;
}

void ElementEditor::setScene(Scene *s)
{
    m_scene = s;
    connect(s, &QGraphicsScene::selectionChanged, this, &ElementEditor::selectionChanged);
}

QAction *addElementAction(QMenu *menu, GraphicElement *firstElm, ElementType type, bool hasSameType)
{
    if (!hasSameType || (firstElm->elementType() != type)) {
        QAction *action = menu->addAction(QIcon(ElementFactory::getPixmap(type)), ElementFactory::translatedName(type));
        action->setData(static_cast<int>(type));
        return action;
    }
    return nullptr;
}

void ElementEditor::contextMenu(QPoint screenPos)
{
    QMenu menu;
    QString renameActionText(tr("Rename"));
    QString rotateActionText(tr("Rotate"));
    QString freqActionText(tr("Change frequency"));
    QString colorMenuText(tr("Change color to..."));
    QString changeSkinText(tr("Change skin to ..."));
    QString revertSkinText(tr("Set skin to default"));
    QString triggerActionText(tr("Change trigger"));
    QString morphMenuText(tr("Morph to..."));
    QString remoteConfigMenuText( tr( "Config" ) );
    if (m_hasLabel) {
        menu.addAction(QIcon(QPixmap(":/toolbar/rename.png")), renameActionText)->setData(renameActionText);
    }
    if (m_hasTrigger) {
        menu.addAction(QIcon(ElementFactory::getPixmap(ElementType::BUTTON)), triggerActionText)->setData(triggerActionText);
    }
    if ((m_canChangeSkin)&&(GlobalProperties::currentFile != "")) {
        menu.addAction(changeSkinText);
        menu.addAction(revertSkinText);
    }
    if (m_hasRotation) {
        menu.addAction(QIcon(QPixmap(":/toolbar/rotateR.png")), rotateActionText)->setData(rotateActionText);
    }
    if (m_hasFrequency) {
        menu.addAction(QIcon(ElementFactory::getPixmap(ElementType::CLOCK)), freqActionText)->setData(freqActionText);
    }
    QMenu *submenucolors = nullptr;
    if (m_hasColors) {
        submenucolors = menu.addMenu(colorMenuText);
        for (int i = 0; i < m_ui->comboBoxColor->count(); ++i) {
            if (m_ui->comboBoxColor->currentIndex() != i) {
                submenucolors->addAction(m_ui->comboBoxColor->itemIcon(i), m_ui->comboBoxColor->itemText(i));
            }
        }
    }
    QMenu *submenumorph = nullptr;
    if (m_canMorph) {
        submenumorph = menu.addMenu(morphMenuText);
        GraphicElement *firstElm = m_elements.front();
        switch (firstElm->elementGroup()) {
        case ElementGroup::GATE: {
            if (firstElm->inputSize() == 1) {
                addElementAction(submenumorph, firstElm, ElementType::NOT, m_hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::NODE, m_hasSameType);
            } else {
                addElementAction(submenumorph, firstElm, ElementType::AND, m_hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::OR, m_hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::NAND, m_hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::NOR, m_hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::XOR, m_hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::XNOR, m_hasSameType);
            }
            break;
        }
        case ElementGroup::STATICINPUT:
        case ElementGroup::INPUT: {
            addElementAction(submenumorph, firstElm, ElementType::BUTTON, m_hasSameType);
            addElementAction(submenumorph, firstElm, ElementType::SWITCH, m_hasSameType);
            addElementAction(submenumorph, firstElm, ElementType::CLOCK, m_hasSameType);
            addElementAction(submenumorph, firstElm, ElementType::VCC, m_hasSameType);
            addElementAction(submenumorph, firstElm, ElementType::GND, m_hasSameType);
            addElementAction(submenumorph, firstElm, ElementType::ROTARY, m_hasSameType);
            break;
        }
        case ElementGroup::MEMORY: {
            if (firstElm->inputSize() == 2) {
                //          addElementAction( submenumorph, firstElm, ElementType::TLATCH, hasSameType );
                addElementAction(submenumorph, firstElm, ElementType::DLATCH, m_hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::JKLATCH, m_hasSameType);
            } else if (firstElm->inputSize() == 4) {
                addElementAction(submenumorph, firstElm, ElementType::DFLIPFLOP, m_hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::TFLIPFLOP, m_hasSameType);
            }
            break;
        }
        case ElementGroup::OUTPUT: {
            addElementAction(submenumorph, firstElm, ElementType::LED, m_hasSameType);
            addElementAction(submenumorph, firstElm, ElementType::BUZZER, m_hasSameType);
            break;
        }
        case ElementGroup::IC:
        case ElementGroup::MUX:
        case ElementGroup::OTHER:
        case ElementGroup::REMOTE:
        case ElementGroup::UNKNOWN:
            break;
        }
        if (submenumorph->actions().size() == 0) {
            menu.removeAction(submenumorph->menuAction());
        }
    }

    if ( m_hasCustomConfig ) {
      QAction *remoteConfigAction = menu.addAction( remoteConfigMenuText );
      connect( remoteConfigAction, &QAction::triggered, m_editor, &Editor::openConfigAction );
    }

    menu.addSeparator();

    if (m_hasElements) {
        QAction *copyAction = menu.addAction(QIcon(QPixmap(":/toolbar/copy.png")), tr("Copy"));
        QAction *cutAction = menu.addAction(QIcon(QPixmap(":/toolbar/cut.png")), tr("Cut"));

        connect(copyAction, &QAction::triggered, m_editor, &Editor::copyAction);
        connect(cutAction, &QAction::triggered, m_editor, &Editor::cutAction);
    }
    QAction *deleteAction = menu.addAction(QIcon(QPixmap(":/toolbar/delete.png")), tr("Delete"));
    connect(deleteAction, &QAction::triggered, m_editor, &Editor::deleteAction);

    QAction *a = menu.exec(screenPos);
    if (a) {
        if (a->data().toString() == renameActionText) {
            renameAction();
        } else if (a->data().toString() == rotateActionText) {
            emit sendCommand(new RotateCommand(m_elements.toList(), 90.0, m_editor));
        } else if (a->data().toString() == triggerActionText) {
            changeTriggerAction();
        } else if (a->text() == changeSkinText) {
            // Reads a new sprite and applies it to the element
            updateElementSkin();
        } else if (a->text() == revertSkinText) {
            // Reset the icon to its default
            m_defaultSkin = true;
            m_updatingSkin = true;
            apply();
        } else if (a->data().toString() == freqActionText) {
            m_ui->doubleSpinBoxFrequency->setFocus();
        } else if (submenumorph && submenumorph->actions().contains(a)) {
            ElementType type = static_cast<ElementType>(a->data().toInt());
            if (type != ElementType::UNKNOWN) {
                emit sendCommand(new MorphCommand(m_elements, type, m_editor));
            }
        } else if (submenucolors && submenucolors->actions().contains(a)) {
            m_ui->comboBoxColor->setCurrentText(a->text());
        } else {
            fprintf(stderr, "In elementeditor: uncaught text \"%s\"\n", a->text().toStdString().c_str());
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
    const QString homeDir = QFileInfo(GlobalProperties::currentFile).absolutePath();
    COMMENT("Updating skin with home dir: " << homeDir.toStdString(), 0);
    QFileDialog fileDialog;
    fileDialog.setObjectName(tr("Open File"));
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    //fileDialog.setFilter(QDir::Files);
    fileDialog.setNameFilter(tr("Images (*.png *.gif *.jpg *.jpeg)"));
    fileDialog.setDirectory(homeDir);
    connect(&fileDialog, &QFileDialog::directoryEntered, this, [&fileDialog, homeDir](QString new_dir) {
        COMMENT("Changing dir to " << new_dir.toStdString() << ", home: " << homeDir.toStdString(), 0);
        if (new_dir != homeDir) {
            fileDialog.setDirectory(homeDir);
        }
    });
    if (!fileDialog.exec()) {
        return;
    }
    auto files = fileDialog.selectedFiles();
    QString fname = files.first();
    if (fname.isEmpty()) {
        return;
    }
    COMMENT("File name: " << fname.toStdString(), 0);
    m_updatingSkin = true;
    m_skinName = fname;
    m_defaultSkin = false;
    apply();
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

void ElementEditor::setCurrentElements(const QVector<GraphicElement *> &elms)
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
        int minimum_inputs = 0, maximum_inputs = 100000000;
        int minimum_outputs = 0, maximum_outputs = 100000000;
        int max_current_output_size = 100000000;
        m_hasSameLabel = m_hasSameColors = m_hasSameFrequency = true;
        m_hasSameInputSize = m_hasSameOutputSize = m_hasSameOutputValue = m_hasSameTrigger = m_canMorph = true;
        m_hasSameAudio = true;
        m_hasSameType = true;
        m_hasElements = true;
        m_hasCustomConfig = true;
        GraphicElement *firstElement = m_elements.front();
        ElementType element_type = firstElement->elementType();
        bool hasRemoteElement = false;
        for (GraphicElement *elm : qAsConst(m_elements)) {
            if (elm->elementType() != firstElement->elementType()) {
                element_type = ElementType::UNKNOWN;
            }
            m_hasLabel &= elm->hasLabel();
            m_canChangeSkin &= elm->canChangeSkin() & !GlobalProperties::currentFile.isEmpty();
            m_hasColors &= elm->hasColors();
            m_hasAudio &= elm->hasAudio();
            m_hasFrequency &= elm->hasFrequency();
            minimum_inputs = std::max(minimum_inputs, elm->minInputSz());
            maximum_inputs = std::min(maximum_inputs, elm->maxInputSz());
            minimum_outputs = std::max(minimum_outputs, elm->minOutputSz());
            maximum_outputs = std::min(maximum_outputs, elm->maxOutputSz());
            m_hasTrigger &= elm->hasTrigger();
            m_hasRotation &= elm->rotatable();

            m_hasSameLabel &= elm->getLabel() == firstElement->getLabel();
            m_hasSameColors &= elm->getColor() == firstElement->getColor();
            m_hasSameFrequency &= qFuzzyCompare(elm->getFrequency(), firstElement->getFrequency());
            m_hasSameInputSize &= elm->inputSize() == firstElement->inputSize();
            m_hasSameOutputSize &= elm->outputSize() == firstElement->outputSize();
            max_current_output_size = std::min(max_current_output_size, elm->outputSize());
            if ((elm->elementGroup() == ElementGroup::INPUT)&&(firstElement->elementGroup() == ElementGroup::INPUT)) {
                m_hasSameOutputValue &= dynamic_cast<Input*>(elm)->outputValue() == dynamic_cast<Input*>(firstElement)->outputValue();
                sameCheckState &= dynamic_cast<Input*>(elm)->isLocked() == dynamic_cast<Input*>(firstElement)->isLocked();
            }
            m_hasSameTrigger &= elm->getTrigger() == firstElement->getTrigger();
            m_hasSameType &= elm->elementType() == firstElement->elementType();
            m_hasSameAudio &= elm->getAudio() == firstElement->getAudio();
            m_canMorph &= elm->inputSize() == firstElement->inputSize();
            m_canMorph &= elm->outputSize() == firstElement->outputSize();

            bool sameElementGroup = elm->elementGroup() == firstElement->elementGroup();
            sameElementGroup |= (elm->elementGroup() == ElementGroup::INPUT && firstElement->elementGroup() == ElementGroup::STATICINPUT);
            sameElementGroup |= (elm->elementGroup() == ElementGroup::STATICINPUT && firstElement->elementGroup() == ElementGroup::INPUT);
            m_hasOnlyInputs &= elm->elementGroup() == ElementGroup::INPUT;
            m_canMorph &= sameElementGroup;
            hasRemoteElement |= elm->elementGroup() == ElementGroup::REMOTE;

            /* Custom config will appear only if a single object is selected */
            m_hasCustomConfig &= elm->hasCustomConfig();
            m_hasCustomConfig &= m_elements.size() == 1;
        }
        m_canChangeInputSize = (minimum_inputs < maximum_inputs && !hasRemoteElement);
        m_canChangeOutputSize = (minimum_outputs < maximum_outputs && !hasRemoteElement);
        /* Element type */
        m_ui->label_type->setText(ElementFactory::typeToTitleText(element_type));
        /* Labels */
        m_ui->lineEditElementLabel->setVisible(m_hasLabel);
        m_ui->lineEditElementLabel->setEnabled(m_hasLabel);
        m_ui->label_labels->setVisible(m_hasLabel);
        if (m_hasLabel) {
            if (m_hasSameLabel) {
                m_ui->lineEditElementLabel->setText(firstElement->getLabel());
            } else {
                m_ui->lineEditElementLabel->setText(m_manyLabels);
            }
        }
        /* Color */
        m_ui->label_color->setVisible(m_hasColors);
        m_ui->comboBoxColor->setVisible(m_hasColors);
        m_ui->comboBoxColor->setEnabled(m_hasColors);
        if (m_ui->comboBoxColor->findText(m_manyColors) == -1) {
            m_ui->comboBoxColor->addItem(m_manyColors);
        }
        if (m_hasColors) {
            if (m_hasSameColors) {
                m_ui->comboBoxColor->removeItem(m_ui->comboBoxColor->findText(m_manyColors));
                m_ui->comboBoxColor->setCurrentIndex(m_ui->comboBoxColor->findData(firstElement->getColor()));
            } else {
                m_ui->comboBoxColor->setCurrentText(m_manyColors);
            }
        }
        /* Sound */
        m_ui->label_audio->setVisible(m_hasAudio);
        m_ui->comboBoxAudio->setVisible(m_hasAudio);
        m_ui->comboBoxAudio->setEnabled(m_hasAudio);
        if (m_ui->comboBoxAudio->findText(m_manyAudios) == -1) {
            m_ui->comboBoxAudio->addItem(m_manyAudios);
        }
        if (m_hasAudio) {
            if (m_hasSameAudio) {
                m_ui->comboBoxAudio->removeItem(m_ui->comboBoxAudio->findText(m_manyAudios));
                m_ui->comboBoxAudio->setCurrentText(firstElement->getAudio());
            } else {
                m_ui->comboBoxAudio->setCurrentText(m_manyAudios);
            }
        }
        /* Frequency */
        m_ui->doubleSpinBoxFrequency->setVisible(m_hasFrequency);
        m_ui->doubleSpinBoxFrequency->setEnabled(m_hasFrequency);
        m_ui->label_frequency->setVisible(m_hasFrequency);
        if (m_hasFrequency) {
            if (m_hasSameFrequency) {
                m_ui->doubleSpinBoxFrequency->setMinimum(0.5);
                m_ui->doubleSpinBoxFrequency->setSpecialValueText(QString());
                m_ui->doubleSpinBoxFrequency->setValue(static_cast<double>(firstElement->getFrequency()));
            } else {
                m_ui->doubleSpinBoxFrequency->setMinimum(0.0);
                m_ui->doubleSpinBoxFrequency->setSpecialValueText(m_manyFreq);
                m_ui->doubleSpinBoxFrequency->setValue(0.0);
            }
        }
        /* Input size */
        m_ui->comboBoxInputSz->clear();
        m_ui->label_inputs->setVisible(m_canChangeInputSize);
        m_ui->comboBoxInputSz->setVisible(m_canChangeInputSize);
        m_ui->comboBoxInputSz->setEnabled(m_canChangeInputSize);
        for (int port = minimum_inputs; port <= maximum_inputs; ++port) {
            m_ui->comboBoxInputSz->addItem(QString::number(port), port);
        }
        if (m_ui->comboBoxInputSz->findText(m_manyIS) == -1) {
            m_ui->comboBoxInputSz->addItem(m_manyIS);
        }
        if (m_canChangeInputSize) {
            if (m_hasSameInputSize) {
                QString inputSz = QString::number(firstElement->inputSize());
                m_ui->comboBoxInputSz->removeItem(m_ui->comboBoxInputSz->findText(m_manyIS));
                m_ui->comboBoxInputSz->setCurrentText(inputSz);
            } else {
                m_ui->comboBoxInputSz->setCurrentText(m_manyIS);
            }
        }
        /* Output size */
        m_ui->comboBoxOutputSz->clear();
        m_ui->label_outputs->setVisible(m_canChangeOutputSize);
        m_ui->comboBoxOutputSz->setVisible(m_canChangeOutputSize);
        m_ui->comboBoxOutputSz->setEnabled(m_canChangeOutputSize);
        if (m_canChangeOutputSize) {
            m_ui->comboBoxOutputSz->addItem(QString::number(2), 2);
            m_ui->comboBoxOutputSz->addItem(QString::number(4), 4);
            m_ui->comboBoxOutputSz->addItem(QString::number(8), 8);
            m_ui->comboBoxOutputSz->addItem(QString::number(10), 10);
            m_ui->comboBoxOutputSz->addItem(QString::number(16), 16);
        }
        if (m_ui->comboBoxOutputSz->findText(m_manyOS) == -1) {
            m_ui->comboBoxOutputSz->addItem(m_manyOS);
        }
        if (m_canChangeOutputSize) {
            if (m_hasSameOutputSize) {
                QString outputSz = QString::number(firstElement->outputSize());
                m_ui->comboBoxOutputSz->removeItem(m_ui->comboBoxOutputSz->findText(m_manyOS));
                m_ui->comboBoxOutputSz->setCurrentText(outputSz);
            } else {
                m_ui->comboBoxOutputSz->setCurrentText(m_manyOS);
            }
        }
        /* Output value */
        m_ui->comboBoxValue->clear();
        m_ui->label_value->setVisible(m_hasOnlyInputs);
        m_ui->comboBoxValue->setVisible(m_hasOnlyInputs);
        m_ui->comboBoxValue->setEnabled(m_hasOnlyInputs);
        if (m_hasOnlyInputs) {
            if (max_current_output_size == 1)
                max_current_output_size++;
            for (int val=0; val < max_current_output_size; ++val) {
                m_ui->comboBoxValue->addItem(QString::number(val), val);
            }
        }
        if (m_ui->comboBoxValue->findText(m_manyOV) == -1) {
            m_ui->comboBoxValue->addItem(m_manyOV);
        }
        if (m_hasOnlyInputs) {
            if (m_hasSameOutputValue) {
                QString outputValue = QString::number(dynamic_cast<Input*>(firstElement)->outputValue());
                m_ui->comboBoxValue->removeItem(m_ui->comboBoxValue->findText(m_manyOV));
                m_ui->comboBoxValue->setCurrentText(outputValue);
            } else {
                m_ui->comboBoxValue->setCurrentText(m_manyOV);
            }
        }
        /* Input locked */
        m_ui->checkBoxLocked->setVisible(m_hasOnlyInputs);
        m_ui->checkBoxLocked->setEnabled(m_hasOnlyInputs);
        if (m_hasOnlyInputs) {
            if (sameCheckState) {
                if (dynamic_cast<Input*>(firstElement)->isLocked()) {
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
        m_ui->label_trigger->setVisible(m_hasTrigger);
        if (m_hasTrigger) {
            if (m_hasSameTrigger) {
                m_ui->lineEditTrigger->setText(firstElement->getTrigger().toString());
            } else {
                m_ui->lineEditTrigger->setText(m_manyTriggers);
            }
        }

        /* Configuration button */
        m_ui->pushButtonCustomConfig->setVisible( m_hasCustomConfig );

        setEnabled(true);
        setVisible(true);
        /* Skin */
        m_ui->label_skin->setVisible(m_canChangeSkin);
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
    QVector<GraphicElement *> elms = m_scene->selectedElements();
    setCurrentElements(elms);
}

void ElementEditor::apply()
{
    COMMENT("Apply", 3);
    if ((m_elements.isEmpty()) || (!isEnabled())) {
        return;
    }
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    for (GraphicElement *elm : qAsConst(m_elements)) {
        elm->save(dataStream);
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
            elm->setFrequency(m_ui->doubleSpinBoxFrequency->value());
        }
        if (elm->hasTrigger() && (m_ui->lineEditTrigger->text() != m_manyTriggers)) {
            if (m_ui->lineEditTrigger->text().size() <= 1) {
                elm->setTrigger(QKeySequence(m_ui->lineEditTrigger->text()));
            }
        }
        if (m_updatingSkin) {
            elm->setSkin(m_defaultSkin, m_skinName);
        }
    }
    if (m_updatingSkin) {
        m_updatingSkin = false;
    }
    emit sendCommand(new UpdateCommand(m_elements, itemData, m_editor));
}

void ElementEditor::setEditor(Editor *value)
{
    m_editor = value;
}

void ElementEditor::inputIndexChanged(int idx)
{
    Q_UNUSED(idx)
    if ((m_elements.isEmpty()) || (!isEnabled())) {
        return;
    }
    if (m_canChangeInputSize && (m_ui->comboBoxInputSz->currentText() != m_manyIS)) {
        emit sendCommand(new ChangeInputSZCommand(m_elements, m_ui->comboBoxInputSz->currentData().toInt(), m_editor));
    }
}

void ElementEditor::outputIndexChanged(const QString &idx)
{
    Q_UNUSED(idx);
    if ((m_elements.isEmpty()) || (!isEnabled())) {
        return;
    }
    if (m_canChangeOutputSize && (m_ui->comboBoxOutputSz->currentText() != m_manyOS)) {
        emit sendCommand(new ChangeOutputSZCommand(m_elements, m_ui->comboBoxOutputSz->currentData().toInt(), m_editor));
    }
    COMMENT("Output size changed to " << idx.toInt(), 0);
    apply();
}

void ElementEditor::outputValueChanged(const QString &idx)
{
    Q_UNUSED(idx);
    if ((m_elements.isEmpty()) || (!isEnabled())) {
        return;
    }
    int new_value = m_ui->comboBoxValue->currentText().toInt();
    for (int idx=0; idx < m_elements.size(); ++idx) {
        auto elm = m_elements[idx];
        if(elm->elementType() == ElementType::ROTARY) {
            dynamic_cast<InputRotary*>(elm)->setOn(true, new_value);
        }
        else {
            dynamic_cast<Input*>(elm)->setOn(new_value);
        }
    }
    apply();
}

void ElementEditor::inputLocked(const bool value)
{
    if ((m_elements.isEmpty()) || (!isEnabled())) {
        return;
    }
    for (int idx=0; idx < m_elements.size(); ++idx) {
        auto elm = m_elements[idx];
        dynamic_cast<Input*>(elm)->setLocked(value);
    }
    COMMENT("Input locked.", 0);
    apply();
}

void ElementEditor::triggerChanged(const QString &cmd)
{
    m_ui->lineEditTrigger->setText(cmd.toUpper());
}

bool ElementEditor::eventFilter(QObject *obj, QEvent *event)
{
    auto *wgt = dynamic_cast<QWidget *>(obj);
    auto *keyEvent = dynamic_cast<QKeyEvent *>(event);
    if ((event->type() == QEvent::KeyPress) && keyEvent && (m_elements.size() == 1)) {
        bool move_fwd = keyEvent->key() == Qt::Key_Tab;
        bool move_back = keyEvent->key() == Qt::Key_Backtab;
        if (move_back || move_fwd) {
            GraphicElement *elm = m_elements.first();
            QVector<GraphicElement *> elms = m_scene->getVisibleElements();
            std::stable_sort(elms.begin(), elms.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
                return elm1->pos().ry() < elm2->pos().ry();
            });
            std::stable_sort(elms.begin(), elms.end(), [](GraphicElement *elm1, GraphicElement *elm2) {
                return elm1->pos().rx() < elm2->pos().rx();
            });
            apply();
            int elmPos = elms.indexOf(elm);
            qDebug() << "Pos = " << elmPos << " from " << elms.size();
            int step = 1;
            if (move_back) {
                step = -1;
            }
            int pos = (elms.size() + elmPos + step) % elms.size();
            for (; pos != elmPos; pos = ((elms.size() + pos + step) % elms.size())) {
                qDebug() << "Pos = " << pos;
                elm = elms[pos];

                setCurrentElements(QVector<GraphicElement *>({elm}));
                if (wgt->isEnabled()) {
                    break;
                }
            }
            m_scene->clearSelection();
            if (!wgt->isEnabled()) {
                elm = elms[elmPos];
            }
            elm->setSelected(true);
            elm->ensureVisible();
            wgt->setFocus();
            event->accept();
            return true;
        }
    }
    /* pass the event on to the parent class */
    return QWidget::eventFilter(obj, event);
}

void ElementEditor::defaultSkin()
{
    m_updatingSkin = true;
    m_defaultSkin = true;
    apply();
}

void ElementEditor::openCustomConfig()
{
    m_editor->openConfigAction();
}
