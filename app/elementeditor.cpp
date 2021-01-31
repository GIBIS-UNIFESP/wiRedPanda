// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "elementeditor.h"

#include <cmath>

#include <QDebug>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMenu>

#include "commands.h"
#include "editor.h"
#include "elementfactory.h"
#include "ui_elementeditor.h"
#include "scene.h"

ElementEditor::ElementEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ElementEditor)
{
    _manyLabels = tr("<Many labels>");
    _manyColors = tr("<Many colors>");
    _manyIS = tr("<Many values>");
    _manyFreq = tr("<Many values>");
    _manyTriggers = tr("<Many triggers>");
    _manyAudios = tr("<Many sounds>");
    m_defaultSkin = true;

    ui->setupUi(this);
    setEnabled(false);
    setVisible(false);

    ui->lineEditTrigger->setValidator(new QRegExpValidator(QRegExp("[a-z]| |[A-Z]|[0-9]"), this));
    fillColorComboBox();
    ui->lineEditElementLabel->installEventFilter(this);
    ui->lineEditTrigger->installEventFilter(this);
    ui->comboBoxColor->installEventFilter(this);
    ui->comboBoxInputSz->installEventFilter(this);
    ui->doubleSpinBoxFrequency->installEventFilter(this);
    ui->comboBoxAudio->installEventFilter(this);

    connect(ui->lineEditElementLabel, &QLineEdit::editingFinished,        this, &ElementEditor::apply);
    connect(ui->doubleSpinBoxFrequency, &QDoubleSpinBox::editingFinished, this, &ElementEditor::apply);
    connect(ui->lineEditTrigger, &QLineEdit::editingFinished,             this, &ElementEditor::apply);
    connect(ui->lineEditTrigger, &QLineEdit::textChanged,                 this, &ElementEditor::triggerChanged);
    connect(ui->pushButtonChangeSkin, &QPushButton::clicked,              this, &ElementEditor::updateElementSkin);
    connect(ui->pushButtonDefaultSkin, &QPushButton::clicked,             this, &ElementEditor::defaultSkin);

    connect(ui->comboBoxColor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ElementEditor::apply);
    connect(ui->comboBoxAudio, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ElementEditor::apply);
    connect(ui->comboBoxInputSz, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ElementEditor::inputIndexChanged);
}

ElementEditor::~ElementEditor()
{
    delete ui;
}

void ElementEditor::setScene(Scene *s)
{
    scene = s;
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
    //  if ( !m_defaultSkin )
    //  {
    //      menu.addAction( "Current icon file: ../filename.png" );
    //  }
    if (hasLabel) {
        menu.addAction(QIcon(QPixmap(":/toolbar/rename.png")), renameActionText)->setData(renameActionText);
    }
    if (hasTrigger) {
        menu.addAction(QIcon(ElementFactory::getPixmap(ElementType::BUTTON)), triggerActionText)->setData(triggerActionText);
    }
    if (canChangeSkin) {
        if (m_defaultSkin) {
            // If the icon set is the default one, add the text to
            // change it.
            menu.addAction(changeSkinText);
        } else {
            // Allow the re-changing of icon
            menu.addAction(changeSkinText);
            // .. or setting it to default
            menu.addAction(revertSkinText);
        }
    }
    if (hasRotation) {
        menu.addAction(QIcon(QPixmap(":/toolbar/rotateR.png")), rotateActionText)->setData(rotateActionText);
    }
    if (hasFrequency) {
        menu.addAction(QIcon(ElementFactory::getPixmap(ElementType::CLOCK)), freqActionText)->setData(freqActionText);
    }
    QMenu *submenucolors = nullptr;
    if (hasColors) {
        submenucolors = menu.addMenu(colorMenuText);
        for (int i = 0; i < ui->comboBoxColor->count(); ++i) {
            if (ui->comboBoxColor->currentIndex() != i) {
                submenucolors->addAction(ui->comboBoxColor->itemIcon(i), ui->comboBoxColor->itemText(i));
            }
        }
    }
    QMenu *submenumorph = nullptr;
    if (canMorph) {
        submenumorph = menu.addMenu(morphMenuText);
        GraphicElement *firstElm = m_elements.front();
        switch (firstElm->elementGroup()) {
        case ElementGroup::GATE: {
            if (firstElm->inputSize() == 1) {
                addElementAction(submenumorph, firstElm, ElementType::NOT, hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::NODE, hasSameType);
            } else {
                addElementAction(submenumorph, firstElm, ElementType::AND, hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::OR, hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::NAND, hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::NOR, hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::XOR, hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::XNOR, hasSameType);
            }
            break;
        }
        case ElementGroup::STATICINPUT:
        case ElementGroup::INPUT: {
            addElementAction(submenumorph, firstElm, ElementType::BUTTON, hasSameType);
            addElementAction(submenumorph, firstElm, ElementType::SWITCH, hasSameType);
            addElementAction(submenumorph, firstElm, ElementType::CLOCK, hasSameType);
            addElementAction(submenumorph, firstElm, ElementType::VCC, hasSameType);
            addElementAction(submenumorph, firstElm, ElementType::GND, hasSameType);
            break;
        }
        case ElementGroup::MEMORY: {
            if (firstElm->inputSize() == 2) {
                //          addElementAction( submenumorph, firstElm, ElementType::TLATCH, hasSameType );
                addElementAction(submenumorph, firstElm, ElementType::DLATCH, hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::JKLATCH, hasSameType);
            } else if (firstElm->inputSize() == 4) {
                addElementAction(submenumorph, firstElm, ElementType::DFLIPFLOP, hasSameType);
                addElementAction(submenumorph, firstElm, ElementType::TFLIPFLOP, hasSameType);
            }
            break;
        }
        case ElementGroup::OUTPUT: {
            addElementAction(submenumorph, firstElm, ElementType::LED, hasSameType);
            addElementAction(submenumorph, firstElm, ElementType::BUZZER, hasSameType);
            break;
        }

        case ElementGroup::IC:
        case ElementGroup::MUX:
        case ElementGroup::OTHER:
        case ElementGroup::UNKNOWN:
            break;
        }
        if (submenumorph->actions().size() == 0) {
            menu.removeAction(submenumorph->menuAction());
        }
    }
    menu.addSeparator();
    if (hasElements) {
        QAction *copyAction = menu.addAction(QIcon(QPixmap(":/toolbar/copy.png")), tr("Copy"));
        QAction *cutAction = menu.addAction(QIcon(QPixmap(":/toolbar/cut.png")), tr("Cut"));

        connect(copyAction, &QAction::triggered, editor, &Editor::copyAction);
        connect(cutAction, &QAction::triggered, editor, &Editor::cutAction);
    }
    QAction *deleteAction = menu.addAction(QIcon(QPixmap(":/toolbar/delete.png")), tr("Delete"));
    connect(deleteAction, &QAction::triggered, editor, &Editor::deleteAction);

    QAction *a = menu.exec(screenPos);
    if (a) {
        if (a->data().toString() == renameActionText) {
            renameAction();
        } else if (a->data().toString() == rotateActionText) {
            emit sendCommand(new RotateCommand(m_elements.toList(), 90.0));
        } else if (a->data().toString() == triggerActionText) {
            changeTriggerAction();
        } else if (a->text() == changeSkinText) {
            // Reads a new sprite and applies it to the element
            this->updateElementSkin();
        } else if (a->text() == revertSkinText) {
            // Reset the icon to its default
            m_defaultSkin = true;
            this->apply();
        } else if (a->data().toString() == freqActionText) {
            ui->doubleSpinBoxFrequency->setFocus();
        } else if (submenumorph && submenumorph->actions().contains(a)) {
            ElementType type = static_cast<ElementType>(a->data().toInt());
            if (type != ElementType::UNKNOWN) {
                emit sendCommand(new MorphCommand(m_elements, type, editor));
            }
        } else if (submenucolors && submenucolors->actions().contains(a)) {
            ui->comboBoxColor->setCurrentText(a->text());
        } else {
            fprintf(stderr, "In elementeditor: uncaught text \"%s\"\n", a->text().toStdString().c_str());
        }
    }
}

void ElementEditor::renameAction()
{
    ui->lineEditElementLabel->setFocus();
    ui->lineEditElementLabel->selectAll();
}

void ElementEditor::changeTriggerAction()
{
    ui->lineEditTrigger->setFocus();
    ui->lineEditTrigger->selectAll();
}

void ElementEditor::updateElementSkin()
{
    const QString homeDir = QDir::homePath();
    QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), homeDir, tr("Images (*.png *.gif *.jpg)"));
    if (fname.isEmpty()) {
        return;
    }
    m_skinName = fname;
    m_defaultSkin = false;
    apply();
}

void ElementEditor::fillColorComboBox()
{
    ui->comboBoxColor->clear();
    ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/GreenLedOn.png")), tr("Green"), "Green");
    ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/BlueLedOn.png")), tr("Blue"), "Blue");
    ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/PurpleLedOn.png")), tr("Purple"), "Purple");
    ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/RedLedOn.png")), tr("Red"), "Red");
    ui->comboBoxColor->addItem(QIcon(QPixmap(":/output/WhiteLedOn.png")), tr("White"), "White");
}

void ElementEditor::retranslateUi()
{
    ui->retranslateUi(this);
    fillColorComboBox();
}

void ElementEditor::setCurrentElements(const QVector<GraphicElement *> &elms)
{
    m_elements = elms;
    hasLabel = hasColors = hasFrequency = canChangeInputSize = hasTrigger = hasAudio = false;
    hasRotation = hasSameLabel = hasSameColors = hasSameFrequency = hasSameAudio = false;
    hasSameInputSize = hasSameTrigger = canMorph = hasSameType = false;
    hasElements = false;
    if (!elms.isEmpty()) {
        hasLabel = hasColors = hasAudio = hasFrequency = canChangeInputSize = hasTrigger = true;
        hasRotation = canChangeSkin = true;
        setVisible(true);
        setEnabled(false);
        int minimum = 0, maximum = 100000000;
        hasSameLabel = hasSameColors = hasSameFrequency = true;
        hasSameInputSize = hasSameTrigger = canMorph = true;
        hasSameAudio = true;
        hasSameType = true;
        hasElements = true;
        GraphicElement *firstElement = m_elements.front();
        for (GraphicElement *elm : qAsConst(m_elements)) {
            hasLabel &= elm->hasLabel();
            canChangeSkin &= elm->canChangeSkin();
            hasColors &= elm->hasColors();
            hasAudio &= elm->hasAudio();
            hasFrequency &= elm->hasFrequency();
            minimum = std::max(minimum, elm->minInputSz());
            maximum = std::min(maximum, elm->maxInputSz());
            hasTrigger &= elm->hasTrigger();
            hasRotation &= elm->rotatable();

            hasSameLabel &= elm->getLabel() == firstElement->getLabel();
            hasSameColors &= elm->getColor() == firstElement->getColor();
            hasSameFrequency &= qFuzzyCompare(elm->getFrequency(), firstElement->getFrequency());
            hasSameInputSize &= elm->inputSize() == firstElement->inputSize();
            hasSameTrigger &= elm->getTrigger() == firstElement->getTrigger();
            hasSameType &= elm->elementType() == firstElement->elementType();
            hasSameAudio &= elm->getAudio() == firstElement->getAudio();
            canMorph &= elm->inputSize() == firstElement->inputSize();
            canMorph &= elm->outputSize() == firstElement->outputSize();

            bool sameElementGroup = elm->elementGroup() == firstElement->elementGroup();
            sameElementGroup |= (elm->elementGroup() == ElementGroup::INPUT && firstElement->elementGroup() == ElementGroup::STATICINPUT);
            sameElementGroup |= (elm->elementGroup() == ElementGroup::STATICINPUT && firstElement->elementGroup() == ElementGroup::INPUT);
            canMorph &= sameElementGroup;
        }
        canChangeInputSize = (minimum < maximum);

        /* Labels */
        ui->lineEditElementLabel->setVisible(hasLabel);
        ui->lineEditElementLabel->setEnabled(hasLabel);
        ui->label_labels->setVisible(hasLabel);
        if (hasLabel) {
            if (hasSameLabel) {
                ui->lineEditElementLabel->setText(firstElement->getLabel());
            } else {
                ui->lineEditElementLabel->setText(_manyLabels);
            }
        }
        /* Color */
        ui->label_color->setVisible(hasColors);
        ui->comboBoxColor->setVisible(hasColors);
        ui->comboBoxColor->setEnabled(hasColors);
        if (ui->comboBoxColor->findText(_manyColors) == -1) {
            ui->comboBoxColor->addItem(_manyColors);
        }
        if (hasColors) {
            if (hasSameColors) {
                ui->comboBoxColor->removeItem(ui->comboBoxColor->findText(_manyColors));
                ui->comboBoxColor->setCurrentIndex(ui->comboBoxColor->findData(firstElement->getColor()));
            } else {
                ui->comboBoxColor->setCurrentText(_manyColors);
            }
        }
        /* Sound */
        ui->label_audio->setVisible(hasAudio);
        ui->comboBoxAudio->setVisible(hasAudio);
        ui->comboBoxAudio->setEnabled(hasAudio);
        if (ui->comboBoxAudio->findText(_manyAudios) == -1) {
            ui->comboBoxAudio->addItem(_manyAudios);
        }
        if (hasAudio) {
            if (hasSameAudio) {
                ui->comboBoxAudio->removeItem(ui->comboBoxAudio->findText(_manyAudios));
                ui->comboBoxAudio->setCurrentText(firstElement->getAudio());
            } else {
                ui->comboBoxAudio->setCurrentText(_manyAudios);
            }
        }
        /* Frequency */
        ui->doubleSpinBoxFrequency->setVisible(hasFrequency);
        ui->doubleSpinBoxFrequency->setEnabled(hasFrequency);
        ui->label_frequency->setVisible(hasFrequency);
        if (hasFrequency) {
            if (hasSameFrequency) {
                ui->doubleSpinBoxFrequency->setMinimum(0.5);
                ui->doubleSpinBoxFrequency->setSpecialValueText(QString());
                ui->doubleSpinBoxFrequency->setValue(static_cast<double>(firstElement->getFrequency()));
            } else {
                ui->doubleSpinBoxFrequency->setMinimum(0.0);
                ui->doubleSpinBoxFrequency->setSpecialValueText(_manyFreq);
                ui->doubleSpinBoxFrequency->setValue(0.0);
            }
        }
        /* Input size */
        ui->comboBoxInputSz->clear();
        ui->label_inputs->setVisible(canChangeInputSize);
        ui->comboBoxInputSz->setVisible(canChangeInputSize);
        ui->comboBoxInputSz->setEnabled(canChangeInputSize);
        for (int port = minimum; port <= maximum; ++port) {
            ui->comboBoxInputSz->addItem(QString::number(port), port);
        }
        if (ui->comboBoxInputSz->findText(_manyIS) == -1) {
            ui->comboBoxInputSz->addItem(_manyIS);
        }
        if (canChangeInputSize) {
            if (hasSameInputSize) {
                QString inputSz = QString::number(firstElement->inputSize());
                ui->comboBoxInputSz->removeItem(ui->comboBoxInputSz->findText(_manyIS));
                ui->comboBoxInputSz->setCurrentText(inputSz);
            } else {
                ui->comboBoxInputSz->setCurrentText(_manyIS);
            }
        }
        /* Trigger */
        ui->lineEditTrigger->setVisible(hasTrigger);
        ui->lineEditTrigger->setEnabled(hasTrigger);
        ui->label_trigger->setVisible(hasTrigger);
        if (hasTrigger) {
            if (hasSameTrigger) {
                ui->lineEditTrigger->setText(firstElement->getTrigger().toString());
            } else {
                ui->lineEditTrigger->setText(_manyTriggers);
            }
        }
        setEnabled(true);
        setVisible(true);
    } else {
        hasElements = false;
        setVisible(false);
        ui->lineEditElementLabel->setText("");
    }
}

void ElementEditor::selectionChanged()
{
    QVector<GraphicElement *> elms = scene->selectedElements();
    setCurrentElements(elms);
}

void ElementEditor::apply()
{
    if ((m_elements.isEmpty()) || (!isEnabled())) {
        return;
    }
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    for (GraphicElement *elm : qAsConst(m_elements)) {
        elm->save(dataStream);
        if (elm->hasColors() && (ui->comboBoxColor->currentData().isValid())) {
            elm->setColor(ui->comboBoxColor->currentData().toString());
        }
        if (elm->hasAudio() && (ui->comboBoxAudio->currentText() != _manyAudios)) {
            elm->setAudio(ui->comboBoxAudio->currentText());
        }
        if (elm->hasLabel() && (ui->lineEditElementLabel->text() != _manyLabels)) {
            elm->setLabel(ui->lineEditElementLabel->text());
        }
        if (elm->hasFrequency() && (ui->doubleSpinBoxFrequency->text() != _manyFreq)) {
            elm->setFrequency(ui->doubleSpinBoxFrequency->value());
        }
        if (elm->hasTrigger() && (ui->lineEditTrigger->text() != _manyTriggers)) {
            if (ui->lineEditTrigger->text().size() <= 1) {
                elm->setTrigger(QKeySequence(ui->lineEditTrigger->text()));
            }
        }
        elm->setSkin(m_defaultSkin, m_skinName);
    }
    emit sendCommand(new UpdateCommand(m_elements, itemData, editor));
}

void ElementEditor::setEditor(Editor *value)
{
    editor = value;
}

void ElementEditor::inputIndexChanged(int idx)
{
    if ((m_elements.isEmpty()) || (!isEnabled())) {
        return;
    }
    if (canChangeInputSize && (ui->comboBoxInputSz->currentText() != _manyIS)) {
        emit sendCommand(new ChangeInputSZCommand(m_elements, ui->comboBoxInputSz->currentData().toInt(), editor));
    }
}

void ElementEditor::triggerChanged(const QString &cmd)
{
    ui->lineEditTrigger->setText(cmd.toUpper());
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
            QVector<GraphicElement *> elms = scene->getVisibleElements();
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
            scene->clearSelection();
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
    m_defaultSkin = true;
    apply();
}
