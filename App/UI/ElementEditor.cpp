// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ElementEditor.h"

#include <cmath>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QImageReader>
#include <QMessageBox>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/Element/ICRegistry.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/UI/ElementContextMenu.h"
#include "App/UI/ElementEditorUI.h"
#include "App/UI/ElementTabNavigator.h"
#include "App/UI/LabeledSlider.h"
#include "App/UI/MainWindow.h"
#include "App/UI/SelectionCapabilities.h"

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

    // Install the tab navigator on every interactive widget so Tab/Shift+Tab
    // cycle through scene elements rather than moving focus between UI fields.
    m_tabNavigator = new ElementTabNavigator(this, this);
    m_ui->checkBoxLocked->installEventFilter(m_tabNavigator);
    m_ui->comboBoxAudio->installEventFilter(m_tabNavigator);
    m_ui->comboBoxColor->installEventFilter(m_tabNavigator);
    m_ui->comboBoxInputSize->installEventFilter(m_tabNavigator);
    m_ui->comboBoxOutputSize->installEventFilter(m_tabNavigator);
    m_ui->comboBoxValue->installEventFilter(m_tabNavigator);
    m_ui->sliderDelay->installEventFilter(m_tabNavigator);
    m_ui->doubleSpinBoxFrequency->installEventFilter(m_tabNavigator);
    m_ui->comboBoxWirelessMode->installEventFilter(m_tabNavigator);
    m_ui->lineEditElementLabel->installEventFilter(m_tabNavigator);
    m_ui->lineEditTrigger->installEventFilter(m_tabNavigator);
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
    connect(m_ui->comboBoxWirelessMode,   qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::apply);
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

// Helper: set visibility/enabled state of a label+widget pair together.
static void setSection(bool show, QWidget *label, QWidget *widget)
{
    label->setVisible(show);
    widget->setVisible(show);
    widget->setEnabled(show);
}

// Helper: manage the "many values" placeholder in a QComboBox.
// Always ensures the placeholder item exists.  If hasFeature && hasSame, removes
// the placeholder and returns true (caller should then set the real value).
// If hasFeature && !hasSame, selects the placeholder text and returns false.
static bool prepareCombo(QComboBox *cb, bool hasFeature, bool hasSame, const QString &many)
{
    if (cb->findText(many) == -1) {
        cb->addItem(many);
    }

    if (!hasFeature) {
        return false;
    }

    if (hasSame) {
        cb->removeItem(cb->findText(many));
        return true;
    }

    cb->setCurrentText(many);
    return false;
}

void ElementEditor::contextMenu(QPoint screenPos, QGraphicsItem *itemAtMouse)
{
    ElementContextMenu::exec(
        screenPos, itemAtMouse, m_caps, m_elements, m_ui->comboBoxColor, m_scene,
        [this](QUndoCommand *cmd) { emit sendCommand(cmd); },
        [this] { renameAction(); },
        [this] { changeTriggerAction(); },
        [this] { updateElementSkin(); },
        [this] { m_isDefaultSkin = true; m_isUpdatingSkin = true; apply(); },
        [this] { m_ui->doubleSpinBoxFrequency->setFocus(); },
        // IC sub-circuit actions
        [this] {
            if (m_elements.isEmpty()) return;
            auto *elm = m_elements.first();
            if (elm->isEmbeddedIC()) {
                emit editSubcircuitRequested(elm->blobName(), elm->id());
            } else if (elm->elementType() == ElementType::IC) {
                auto *ic = static_cast<IC *>(elm);
                Application::instance()->mainWindow()->loadPandaFile(ic->icFile());
            }
        },
        [this] { emit embedSubcircuitRequested(); },
        [this] { emit extractToFileRequested(); }
    );
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
        disconnect(m_scene, &QGraphicsScene::selectionChanged,   this,    &ElementEditor::selectionChanged);
        disconnect(m_scene, &Scene::contextMenuPos,              this,    &ElementEditor::contextMenu);
        disconnect(m_scene, &Scene::truthTableElementChanged,    this,    &ElementEditor::truthTable);
        disconnect(this,    &ElementEditor::sendCommand,         m_scene, &Scene::receiveCommand);
    }

    m_scene = scene;

    if (m_scene) {
        connect(m_scene, &QGraphicsScene::selectionChanged,   this,    &ElementEditor::selectionChanged);
        connect(m_scene, &Scene::contextMenuPos,              this,    &ElementEditor::contextMenu);
        connect(m_scene, &Scene::truthTableElementChanged,    this,    [this](GraphicElement *) { truthTable(); });
        connect(this,    &ElementEditor::sendCommand,         m_scene, &Scene::receiveCommand);
    }
}

void ElementEditor::fillColorComboBox()
{
    QSignalBlocker blocker(m_ui->comboBoxColor);
    m_ui->comboBoxColor->clear();
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

    if (elements.isEmpty()) {
        m_caps = {};
        hide();
        m_ui->lineEditElementLabel->setText("");
        return;
    }

    show();
    // Disable during update to prevent partial changes firing as undo commands.
    setEnabled(false);
    m_caps = ::computeCapabilities(m_elements);
    applyCapabilitiesToUi();
    setEnabled(true);
}

void ElementEditor::applyCapabilitiesToUi()
{
    const auto &c      = m_caps;
    auto *firstElement = m_elements.constFirst();
    auto *firstInput   = qobject_cast<GraphicElementInput *>(firstElement);

    /* Element type title */
    m_ui->groupBox->setTitle(ElementFactory::typeToTitleText(c.elementType));

    /* Label */
    setSection(c.hasLabel, m_ui->labelLabels, m_ui->lineEditElementLabel);
    if (c.hasLabel) {
        m_ui->lineEditElementLabel->setText(c.hasSameLabel ? firstElement->label() : m_manyLabels);
    }

    /* Color */
    setSection(c.hasColors, m_ui->labelColor, m_ui->comboBoxColor);
    if (prepareCombo(m_ui->comboBoxColor, c.hasColors, c.hasSameColors, m_manyColors)) {
        m_ui->comboBoxColor->setCurrentIndex(m_ui->comboBoxColor->findData(firstElement->color()));
    }

    /* Sound */
    setSection(c.hasAudio, m_ui->labelAudio, m_ui->comboBoxAudio);
    if (prepareCombo(m_ui->comboBoxAudio, c.hasAudio, c.hasSameAudio, m_manyAudios)) {
        m_ui->comboBoxAudio->setCurrentText(firstElement->audio());
    }

    /* Frequency */
    setSection(c.hasFrequency, m_ui->labelFrequency, m_ui->doubleSpinBoxFrequency);
    if (c.hasFrequency) {
        if (c.hasSameFrequency) {
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

    /* Delay */
    setSection(c.hasDelay, m_ui->labelDelay, m_ui->sliderDelay);
    if (c.hasDelay) {
        // Convert delay value (fraction of period, -0.5 to 0.5) to slider value (-4 to 4).
        m_ui->sliderDelay->setValue(c.hasSameDelay ? static_cast<int>(firstElement->delay() * 8.0f) : 0);
    }

    /* Input size */
    m_ui->comboBoxInputSize->clear();
    setSection(c.canChangeInputSize, m_ui->labelInputs, m_ui->comboBoxInputSize);
    if (c.canChangeInputSize && c.hasSameType && c.elementType == ElementType::Mux) {
        // For Mux, the user picks the number of data inputs (2–8); select lines
        // are computed as ceil(log2(dataInputs)) and added internally.  The item
        // display text shows data inputs while the item data carries the total.
        for (int dataInputs = 2; dataInputs <= 8; ++dataInputs) {
            int selectLines = 1;
            while ((1 << selectLines) < dataInputs) { ++selectLines; }
            const int totalInputs = dataInputs + selectLines;
            if (totalInputs >= c.minimumInputs && totalInputs <= c.maximumInputs) {
                m_ui->comboBoxInputSize->addItem(QString::number(dataInputs), totalInputs);
            }
        }
    } else {
        for (int port = c.minimumInputs; port <= c.maximumInputs; ++port) {
            m_ui->comboBoxInputSize->addItem(QString::number(port), port);
        }
    }
    if (prepareCombo(m_ui->comboBoxInputSize, c.canChangeInputSize, c.hasSameInputSize, m_manyIS)) {
        const int idx = m_ui->comboBoxInputSize->findData(firstElement->inputSize());
        if (idx >= 0) { m_ui->comboBoxInputSize->setCurrentIndex(idx); }
    }

    /* Output size */
    m_ui->comboBoxOutputSize->clear();
    setSection(c.canChangeOutputSize, m_ui->labelOutputs, m_ui->comboBoxOutputSize);
    if (c.canChangeOutputSize) {
        if (c.hasSameType && c.elementType == ElementType::Demux) {
            // Demux: show 2-8 output options, mirroring Mux's 2-8 data inputs.
            for (int n = c.minimumOutputs; n <= c.maximumOutputs; ++n) {
                m_ui->comboBoxOutputSize->addItem(QString::number(n), n);
            }
        } else {
            // For generic multi-output elements (e.g. Node) offer a curated set
            // of common bus widths rather than every value in [min, max].
            for (int n : {2, 3, 4, 6, 8, 10, 12, 16}) {
                m_ui->comboBoxOutputSize->addItem(QString::number(n), n);
            }
        }
    }
    if (prepareCombo(m_ui->comboBoxOutputSize, c.canChangeOutputSize && !c.hasTruthTable,
                     c.hasSameOutputSize, m_manyOS)) {
        const int idx = m_ui->comboBoxOutputSize->findData(firstElement->outputSize());
        if (idx >= 0) { m_ui->comboBoxOutputSize->setCurrentIndex(idx); }
    }

    /* Output value (hidden for momentary inputs like InputButton) */
    m_ui->comboBoxValue->clear();
    setSection(c.hasLatchedValue, m_ui->labelValue, m_ui->comboBoxValue);
    if (c.hasLatchedValue) {
        // Binary inputs have outputSize == 1 but must offer values 0 and 1,
        // so bump the upper bound to 2 in that case.
        int maxOut = c.maxCurrentOutputSize;
        if (maxOut == 1) { ++maxOut; }
        for (int val = 0; val < maxOut; ++val) {
            m_ui->comboBoxValue->addItem(QString::number(val), val);
        }
    }
    if (prepareCombo(m_ui->comboBoxValue, c.hasLatchedValue, c.hasSameOutputValue, m_manyOV)) {
        m_ui->comboBoxValue->setCurrentText(QString::number(firstInput->outputValue()));
    }

    /* Input locked */
    setSection(c.hasOnlyInputs, m_ui->labelLocked, m_ui->checkBoxLocked);
    if (c.hasOnlyInputs) {
        // When a multi-selection has mixed lock states, show a partially-checked
        // state rather than incorrectly representing all as locked or unlocked.
        if (c.sameCheckState) {
            m_ui->checkBoxLocked->setCheckState(firstInput->isLocked() ? Qt::Checked : Qt::Unchecked);
        } else {
            m_ui->checkBoxLocked->setCheckState(Qt::PartiallyChecked);
        }
    }

    /* Trigger */
    setSection(c.hasTrigger, m_ui->labelTrigger, m_ui->lineEditTrigger);
    if (c.hasTrigger) {
        m_ui->lineEditTrigger->setText(c.hasSameTrigger ? firstElement->trigger().toString() : m_manyTriggers);
    }

    /* AudioBox */
    m_ui->pushButtonAudioBox->setVisible(c.hasAudioBox);
    m_ui->pushButtonAudioBox->setEnabled(c.hasAudioBox);
    m_ui->labelAudioBox->setVisible(c.hasAudioBox);
    m_ui->lineCurrentAudioBox->setVisible(c.hasAudioBox);
    if (c.hasAudioBox) {
        m_ui->lineCurrentAudioBox->setText(m_elements.size() > 1 ? m_manyAudios : m_elements[0]->audio());
    }

    /* TruthTable */
    m_ui->pushButtonTruthTable->setVisible(c.hasTruthTable);
    m_ui->pushButtonTruthTable->setEnabled(c.hasTruthTable);

    /* Skin */
    m_ui->pushButtonChangeSkin->setVisible(c.canChangeSkin);
    m_ui->pushButtonDefaultSkin->setVisible(c.canChangeSkin);

    /* Wireless mode — Node elements only */
    setSection(c.hasWirelessMode, m_ui->labelWirelessMode, m_ui->comboBoxWirelessMode);
    if (c.hasWirelessMode && !m_elements.isEmpty()) {
        auto *first = qobject_cast<Node *>(m_elements.constFirst());
        const bool sameMode = std::all_of(m_elements.cbegin(), m_elements.cend(), [&](GraphicElement *elm) {
            const auto *n = qobject_cast<Node *>(elm);
            return n && n->wirelessMode() == first->wirelessMode();
        });
        if (prepareCombo(m_ui->comboBoxWirelessMode, true, sameMode, m_manyWirelessModes) && first) {
            QSignalBlocker blocker(m_ui->comboBoxWirelessMode);
            m_ui->comboBoxWirelessMode->setCurrentIndex(static_cast<int>(first->wirelessMode()));
        }
    }

    /* Blob name — embedded ICs only */
    setSection(c.isEmbedded, m_ui->labelBlobName, m_ui->lineEditBlobName);
    if (c.isEmbedded && !m_elements.isEmpty()) {
        const auto *firstElm = m_elements.constFirst();
        const bool sameBlobName = std::all_of(m_elements.cbegin(), m_elements.cend(), [&](GraphicElement *elm) {
            return elm->blobName() == firstElm->blobName();
        });
        QSignalBlocker blocker(m_ui->lineEditBlobName);
        m_ui->lineEditBlobName->setText(sameBlobName ? firstElement->blobName() : m_manyLabels);
    }
}

void ElementEditor::selectionChanged()
{
    setCurrentElements(m_scene->selectedElements());
}

void ElementEditor::applyProperty(GraphicElement *elm, PropertyDescriptor::Type type)
{
    switch (type) {
    case PropertyDescriptor::Type::Label:
        if (m_ui->lineEditElementLabel->text() != m_manyLabels) {
            elm->setLabel(m_ui->lineEditElementLabel->text());
        }
        break;
    case PropertyDescriptor::Type::Color:
        if (m_ui->comboBoxColor->currentData().isValid()) {
            elm->setColor(m_ui->comboBoxColor->currentData().toString());
        }
        break;
    case PropertyDescriptor::Type::Frequency:
        if (m_ui->doubleSpinBoxFrequency->text() != m_manyFreq) {
            elm->setFrequency(static_cast<float>(m_ui->doubleSpinBoxFrequency->value()));
        }
        break;
    case PropertyDescriptor::Type::Delay: {
        // Convert slider value (-4 to 4) to phase delay (-0.5 to 0.5 as fraction of period).
        const float delayFraction = static_cast<float>(m_ui->sliderDelay->value()) / 8.0f;
        elm->setDelay(delayFraction);
        break;
    }
    case PropertyDescriptor::Type::Audio:
        if (m_ui->comboBoxAudio->currentText() != m_manyAudios) {
            elm->setAudio(m_ui->comboBoxAudio->currentText());
        }
        break;
    case PropertyDescriptor::Type::Trigger:
        if (m_ui->lineEditTrigger->text() != m_manyTriggers && m_ui->lineEditTrigger->text().size() <= 1) {
            elm->setTrigger(QKeySequence(m_ui->lineEditTrigger->text()));
        }
        break;
    case PropertyDescriptor::Type::Skin:
        if (m_isUpdatingSkin) {
            elm->setSkin(m_isDefaultSkin, m_skinName);
        }
        break;
    case PropertyDescriptor::Type::WirelessModeSelector:
        if (m_ui->comboBoxWirelessMode->currentText() != m_manyWirelessModes) {
            if (auto *node = qobject_cast<Node *>(elm)) {
                node->setWirelessMode(static_cast<WirelessMode>(m_ui->comboBoxWirelessMode->currentIndex()));
            }
        }
        break;
    case PropertyDescriptor::Type::AudioBox:
    case PropertyDescriptor::Type::TruthTable:
        // Managed by their own dialogs (audioBox() / truthTable()); not applied here.
        break;
    }
}

void ElementEditor::apply()
{
    qCDebug(three) << "Apply.";

    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    // Reject label changes that would create a duplicate wireless Tx channel.
    // A Tx node's label is its channel name — two Tx nodes on the same label
    // means one silently receives no signal, which is always a user error.
    if (m_scene && m_caps.hasWirelessMode) {
        const QString newLabel = m_ui->lineEditElementLabel->text();
        const bool labelChanging = (newLabel != m_manyLabels);
        const WirelessMode newMode = static_cast<WirelessMode>(m_ui->comboBoxWirelessMode->currentIndex());
        const bool modeChanging = (m_ui->comboBoxWirelessMode->currentText() != m_manyWirelessModes);

        for (auto *elm : std::as_const(m_elements)) {
            auto *node = qobject_cast<Node *>(elm);
            if (!node) continue;

            const QString candidateLabel = labelChanging ? newLabel : node->label();
            const WirelessMode candidateMode = modeChanging ? newMode : node->wirelessMode();

            if (candidateMode != WirelessMode::Tx || candidateLabel.isEmpty()) continue;

            for (auto *other : m_scene->elements()) {
                if (other == elm) continue;
                auto *otherNode = qobject_cast<Node *>(other);
                if (!otherNode || otherNode->wirelessMode() != WirelessMode::Tx) continue;
                if (otherNode->label() == candidateLabel) {
                    QMessageBox::warning(this,
                        tr("Duplicate Wireless Channel"),
                        tr("A Tx node with label \"%1\" already exists.\n"
                           "Each wireless channel must have a unique label.").arg(candidateLabel));
                    update();
                    return;
                }
            }
        }
    }

    // Collect connections that will be severed by wireless mode changes.
    // setWirelessMode() hides the replaced port but does not delete connections
    // itself — that is handled here via DeleteItemsCommand so undo can restore them.
    QList<QGraphicsItem *> wirelessConnsToDelete;
    if (m_scene && m_caps.hasWirelessMode) {
        const WirelessMode newMode = static_cast<WirelessMode>(m_ui->comboBoxWirelessMode->currentIndex());
        const bool modeChanging = (m_ui->comboBoxWirelessMode->currentText() != m_manyWirelessModes);

        if (modeChanging) {
            for (auto *elm : std::as_const(m_elements)) {
                auto *node = qobject_cast<Node *>(elm);
                if (!node) continue;

                QNEPort *port = (newMode == WirelessMode::Rx) ? static_cast<QNEPort *>(node->inputPort())
                             : (newMode == WirelessMode::Tx) ? static_cast<QNEPort *>(node->outputPort())
                             : nullptr;
                if (port) {
                    for (auto *conn : port->connections()) {
                        wirelessConnsToDelete.append(static_cast<QGraphicsItem *>(conn));
                    }
                }
            }
        }
    }

    // Snapshot current state into oldData before modifying anything.
    // UpdateCommand uses oldData for undo and captures the post-modification
    // state internally as newData when it is constructed.
    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    for (auto *elm : std::as_const(m_elements)) {
        elm->save(stream);
        for (const auto &prop : elm->editableProperties()) {
            applyProperty(elm, prop.type);
        }
    }

    // Apply blob name rename via ICRegistry (updates registry key + all instances)
    if (m_caps.isEmbedded && !m_elements.isEmpty()) {
        const QString newBlobName = m_ui->lineEditBlobName->text().trimmed();
        const QString oldBlobName = m_elements.first()->blobName();
        if (!newBlobName.isEmpty() && newBlobName != oldBlobName) {
            m_scene->icRegistry()->renameBlob(oldBlobName, newBlobName);
        }
    }

    // Reset the one-shot skin update flag after applying to all elements.
    if (m_isUpdatingSkin) {
        m_isUpdatingSkin = false;
    }

    // When wireless mode changes sever connections, group the property update
    // and the delete into a single undo macro so both are undone together.
    const bool needsMacro = !wirelessConnsToDelete.isEmpty();
    if (needsMacro) {
        m_scene->undoStack()->beginMacro(tr("Change wireless mode"));
    }

    m_scene->receiveCommand(new UpdateCommand(m_elements, oldData, m_scene));

    if (needsMacro) {
        m_scene->receiveCommand(new DeleteItemsCommand(wirelessConnsToDelete, m_scene));
        m_scene->undoStack()->endMacro();
    }
}

void ElementEditor::inputIndexChanged(const int index)
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }

    if (m_caps.canChangeInputSize && (m_ui->comboBoxInputSize->currentText() != m_manyIS)) {
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

    if (m_caps.canChangeOutputSize && (m_ui->comboBoxOutputSize->currentText() != m_manyOS)) {
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
    m_ui->lineEditTrigger->setText(cmd.toUpper());
    apply();
}

void ElementEditor::truthTable()
{
    if (!m_caps.hasTruthTable) return;

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

    emit sendCommand(new ToggleTruthTableOutputCommand(truthtable, positionToChange, m_scene));

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
    // Tweak the group box border colour to match the active theme.
    // Light theme: rgb(216,216,216) — a light grey; Dark: rgb(66,66,66) — a dark grey.
    const QString borderColor = (ThemeManager::effectiveTheme() == Theme::Light) ? "216" : "66";
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

