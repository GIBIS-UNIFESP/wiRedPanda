// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ElementEditor.h"

#include <cmath>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QImageReader>
#include <QMessageBox>
#include <QSize>
#include <QToolButton>

#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/UI/ElementContextMenu.h"
#include "App/UI/ElementEditorUI.h"
#include "App/UI/ElementTabNavigator.h"
#include "App/UI/FileDialogProvider.h"
#include "App/UI/LabeledSlider.h"
#include "App/UI/SelectionCapabilities.h"
#include "App/Wiring/Connection.h"

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
    m_ui->sliderVolume->installEventFilter(m_tabNavigator);
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
    connect(m_ui->comboBoxInputSize,      qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::inputIndexChanged);
    connect(m_ui->comboBoxOutputSize,     qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::outputIndexChanged);
    connect(m_ui->comboBoxValue,          &QComboBox::currentTextChanged,                   this, &ElementEditor::outputValueChanged);
    connect(m_ui->lineEditTrigger,        &QLineEdit::textChanged,                          this, &ElementEditor::triggerChanged);
    connect(m_ui->lineEditBlobName,       &QLineEdit::editingFinished,                      this, &ElementEditor::blobNameEditingFinished);
    connect(m_ui->pushButtonAudioBox,     &QPushButton::clicked,                            this, &ElementEditor::audioBox);
    connect(m_ui->pushButtonChangeAppearance,   &QPushButton::clicked,                            this, &ElementEditor::updateElementAppearance);
    connect(m_ui->pushButtonDefaultAppearance,  &QPushButton::clicked,                            this, &ElementEditor::defaultAppearance);
    connect(m_ui->pushButtonTruthTable,   &QPushButton::clicked,                            this, &ElementEditor::truthTable);

    // Each field sets its own property of the selection. One typed into or dragged through is one undo step until it
    // is left or released (endEditSession()).
    connect(m_ui->lineEditElementLabel,   &QLineEdit::textChanged,                          this, &ElementEditor::labelChanged);
    connect(m_ui->comboBoxColor,          qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::colorChanged);
    connect(m_ui->comboBoxAudio,          qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::audioChanged);
    connect(m_ui->doubleSpinBoxFrequency, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ElementEditor::frequencyChanged);
    connect(m_ui->sliderDelay,            qOverload<int>(&QSlider::valueChanged),           this, &ElementEditor::delayChanged);
    connect(m_ui->sliderVolume,           qOverload<int>(&QSlider::valueChanged),           this, &ElementEditor::volumeChanged);
    connect(m_ui->comboBoxWirelessMode,   qOverload<int>(&QComboBox::currentIndexChanged),  this, &ElementEditor::wirelessModeChanged);
    connect(m_ui->lineEditElementLabel,   &QLineEdit::editingFinished,                      this, &ElementEditor::endEditSession);
    connect(m_ui->lineEditTrigger,        &QLineEdit::editingFinished,                      this, &ElementEditor::endEditSession);
    connect(m_ui->doubleSpinBoxFrequency, &QDoubleSpinBox::editingFinished,                 this, &ElementEditor::endEditSession);
    connect(m_ui->sliderDelay,            &QSlider::sliderReleased,                         this, &ElementEditor::endEditSession);
    connect(m_ui->sliderVolume,           &QSlider::sliderReleased,                         this, &ElementEditor::endEditSession);
}

ElementEditor::~ElementEditor()
{
    // A focused field reports that it was left (editingFinished) as it is destroyed, after this editor's part of the
    // object is gone: the fields stop reaching it first
    const auto fields = findChildren<QWidget *>();
    for (auto *field : fields) {
        field->disconnect(this);
    }
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
        [this] { updateElementAppearance(); },
        [this] { applyAppearance(true, {}); },
        [this] { m_ui->doubleSpinBoxFrequency->setFocus(); },
        // IC sub-circuit actions
        [this] {
            if (m_elements.isEmpty()) return;
            auto *elm = m_elements.first();
            if (elm->isEmbedded()) {
                emit editSubcircuitRequested(elm->blobName(), elm->id());
            } else if (elm->elementType() == ElementType::IC) {
                auto *ic = static_cast<IC *>(elm);
                emit openSubcircuitFileRequested(ic->file());
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

void ElementEditor::rebuildAppearanceStateTiles(const QList<std::pair<int, QString>> &states)
{
    // Drop the previous element's tiles; QButtonGroup auto-detaches destroyed buttons.
    while (auto *item = m_ui->gridLayoutAppearanceStates->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    constexpr int columns = 4;
    constexpr QSize tileSize(32, 32);
    constexpr QSize iconSize(24, 24);

    for (int i = 0; i < states.size(); ++i) {
        const auto &[index, label] = states[i];
        auto *tile = new QToolButton(m_ui->widgetAppearanceStates);
        tile->setCheckable(true);
        tile->setAutoRaise(true);
        tile->setFixedSize(tileSize);
        tile->setIconSize(iconSize);
        tile->setIcon(QIcon(m_elements[0]->appearancePreviewPixmap(index, iconSize)));
        tile->setToolTip(label);
        tile->setProperty("appearanceStateIndex", index);
        m_ui->buttonGroupAppearanceStates->addButton(tile);
        m_ui->gridLayoutAppearanceStates->addWidget(tile, i / columns, i % columns);
    }

    // Default-select the first tile, matching the combobox's old implicit currentIndex 0.
    if (auto *item = m_ui->gridLayoutAppearanceStates->itemAtPosition(0, 0)) {
        if (auto *firstTile = qobject_cast<QToolButton *>(item->widget())) {
            firstTile->setChecked(true);
        }
    }
}

void ElementEditor::updateElementAppearance()
{
    sentryBreadcrumb("ui", QStringLiteral("Element appearance dialog"));
    const QString imageFilter = tr("Images") + " (*." + QImageReader::supportedImageFormats().join(" *.") + ")";
    const QString fileName = FileDialogs::provider()->getOpenFileName(this, tr("Open File"), QString(), imageFilter);

    if (fileName.isEmpty()) {
        return;
    }

    qCDebug(zero) << "File name: " << fileName;

    // If a specific appearance state is selected in the tile grid, use setAppearanceAt()
    // to target that index directly instead of relying on the element's current state.
    if (m_ui->widgetAppearanceStates->isVisible() && m_ui->buttonGroupAppearanceStates->checkedButton()) {
        const int appearanceIndex = m_ui->buttonGroupAppearanceStates->checkedButton()->property("appearanceStateIndex").toInt();

        // Snapshot and apply via undo command
        const bool needsMacro = m_scene && m_elements.size() > 1;
        if (needsMacro) {
            m_scene->undoStack()->beginMacro(tr("Change appearance")); // LCOV_EXCL_LINE — the tile grid (widgetAppearanceStates) is only ever made visible for a single-element selection (see applyCapabilitiesToUi()'s `m_elements.size() == 1` guard), so entering this branch structurally guarantees m_elements.size() == 1, making needsMacro permanently false.
        }
        for (auto *elm : std::as_const(m_elements)) {
            QByteArray oldData;
            {
                QDataStream stream(&oldData, QIODevice::WriteOnly);
                Serialization::writePandaHeader(stream);
                elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
            }
            elm->setAppearanceAt(appearanceIndex, fileName);
            if (m_scene) {
                m_scene->receiveCommand(new UpdateCommand({elm}, oldData, m_scene));
            }
        }
        if (needsMacro) {
            m_scene->undoStack()->endMacro(); // LCOV_EXCL_LINE — see the matching beginMacro() exclusion above: needsMacro is permanently false here.
        }

        // The tile icons were built from the pre-change appearance; refresh the edited tile in
        // place so it shows the new image immediately. Without this it keeps the old preview
        // until the element is reselected (which rebuilds the whole grid).
        if (auto *checked = qobject_cast<QToolButton *>(m_ui->buttonGroupAppearanceStates->checkedButton())) {
            checked->setIcon(QIcon(m_elements[0]->appearancePreviewPixmap(appearanceIndex, checked->iconSize())));
        }
        return;
    }

    applyAppearance(false, fileName);
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
    // Filling the fields for a selection starts another undo step
    endEditSession();
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
        m_ui->comboBoxAudio->setCurrentText(firstElement->audio()); // LCOV_EXCL_LINE — c.hasAudio is always false: no element type currently sets hasAudio=true in its ElementInfo (see App/Scene/Commands.cpp's identical exclusion), so prepareCombo() never returns true here.
    }

    /* Volume */
    setSection(c.hasVolume, m_ui->labelVolume, m_ui->sliderVolume);
    if (c.hasVolume && c.hasSameVolume) {
        m_ui->sliderVolume->setValue(qRound(firstElement->volume() * 100.0f));
    }

    /* Frequency */
    setSection(c.hasFrequency, m_ui->labelFrequency, m_ui->doubleSpinBoxFrequency);
    if (c.hasFrequency) {
        // Buzzer uses audible tone range; Clock uses low oscillation range.
        const bool isBuzzer = (c.hasSameType && c.elementType == ElementType::Buzzer);
        const double minFreq = isBuzzer ? 20.0 : 0.1;
        const double maxFreq = isBuzzer ? 20000.0 : 50.0;
        const double step    = isBuzzer ? 10.0 : 0.1;
        const int decimals   = isBuzzer ? 0 : 1;
        const QString suffix = isBuzzer ? tr(" Hz") : tr(" Hz");

        m_ui->doubleSpinBoxFrequency->setDecimals(decimals);
        m_ui->doubleSpinBoxFrequency->setSingleStep(step);
        m_ui->doubleSpinBoxFrequency->setMaximum(maxFreq);
        m_ui->doubleSpinBoxFrequency->setSuffix(suffix);

        if (c.hasSameFrequency) {
            m_ui->doubleSpinBoxFrequency->setMinimum(minFreq);
            m_ui->doubleSpinBoxFrequency->setSpecialValueText({});
            m_ui->doubleSpinBoxFrequency->setValue(firstElement->frequency());
        } else {
            m_ui->doubleSpinBoxFrequency->setMinimum(0.0);
            m_ui->doubleSpinBoxFrequency->setSpecialValueText(m_manyFreq);
            m_ui->doubleSpinBoxFrequency->setValue(0.0);
        }
    }

    /* Delay */
    setSection(c.hasDelay, m_ui->labelDelay, m_ui->sliderDelay);
    if (c.hasDelay) {
        // Convert delay value (fraction of period, -0.5 to 0.5) to slider value (-4 to 4).
        m_ui->sliderDelay->setValue(c.hasSameDelay ? static_cast<int>(firstElement->delay() * 8.0) : 0);
    }

    /* Input size */
    {
        // Block signals while rebuilding so the synthetic addItem/setCurrentIndex
        // calls don't fire inputIndexChanged — that slot would push spurious
        // ChangePortSizeCommands and (more importantly here) flood the Sentry
        // breadcrumb buffer with "Input count: -1 / 0" entries that crowd out
        // the user's real actions.
        QSignalBlocker blocker(m_ui->comboBoxInputSize);
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
    }

    /* Output size */
    {
        QSignalBlocker blocker(m_ui->comboBoxOutputSize);
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

    /* Appearance */
    m_ui->pushButtonChangeAppearance->setVisible(c.canChangeAppearance);
    m_ui->pushButtonDefaultAppearance->setVisible(c.canChangeAppearance);

    // Populate the appearance state icon grid for multi-state elements
    if (c.canChangeAppearance && m_elements.size() == 1) {
        const auto states = m_elements[0]->appearanceStates();
        const bool multiState = states.size() > 1;
        m_ui->labelAppearanceState->setVisible(multiState);
        m_ui->widgetAppearanceStates->setVisible(multiState);
        if (multiState) {
            rebuildAppearanceStateTiles(states);
        }
    } else {
        m_ui->labelAppearanceState->setVisible(false);
        m_ui->widgetAppearanceStates->setVisible(false);
    }

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

    /* Section visibility — hide a section's group box entirely when none of its
       rows apply to the current selection, so the panel doesn't show empty
       titled boxes. */
    m_ui->groupBoxIdentity->setVisible(c.hasLabel || c.hasColors);
    m_ui->groupBoxPorts->setVisible(c.canChangeInputSize || c.canChangeOutputSize || c.hasLatchedValue || c.hasOnlyInputs);
    m_ui->groupBoxTiming->setVisible(c.hasFrequency || c.hasDelay);
    m_ui->groupBoxSound->setVisible(c.hasAudio || c.hasAudioBox || c.hasVolume);
    m_ui->groupBoxInteraction->setVisible(c.hasTrigger || c.hasTruthTable || c.hasWirelessMode);
    m_ui->groupBoxAppearanceSection->setVisible(c.canChangeAppearance || c.isEmbedded);
}

void ElementEditor::selectionChanged()
{
    setCurrentElements(m_scene->selectedElements());
}

bool ElementEditor::editable() const
{
    // m_scene can be null while m_elements still holds the previous tab's
    // selection (setScene(nullptr) on tab change).
    return !m_elements.isEmpty() && isEnabled() && m_scene;
}

void ElementEditor::setProperty(const ElementProperty property, const QVariant &value, const bool merges)
{
    if (!editable()) {
        return;
    }
    sentryBreadcrumb("ui", QStringLiteral("Element properties applied"));
    qCDebug(three) << "Apply.";

    if (auto *command = SetPropertyCommand::apply(m_elements, property, value, m_scene, merges ? m_editSession : -1)) {
        m_scene->receiveCommand(command);
    }
}

void ElementEditor::endEditSession()
{
    ++m_editSession;
}

bool ElementEditor::refusesTxChannel(const std::optional<QString> &label, const std::optional<WirelessMode> mode)
{
    // A Tx node's label is its channel name — two Tx nodes on the same label
    // means one silently receives no signal, which is always a user error.
    if (!m_caps.hasWirelessMode) {
        return false;
    }

    for (auto *elm : std::as_const(m_elements)) {
        auto *node = qobject_cast<Node *>(elm);
        if (!node) continue;

        const QString candidateLabel = label.value_or(node->label());
        const WirelessMode candidateMode = mode.value_or(node->wirelessMode());

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
                return true;
            }
        }
    }
    return false;
}

void ElementEditor::labelChanged(const QString &label)
{
    if (!editable() || (label == m_manyLabels) || refusesTxChannel(label, std::nullopt)) {
        return;
    }
    setProperty(ElementProperty::Label, label, true);
}

void ElementEditor::colorChanged()
{
    // The item data is the untranslated colour name; the placeholder of a selection with many colours has none
    const QVariant color = m_ui->comboBoxColor->currentData();
    if (color.isValid()) {
        setProperty(ElementProperty::Color, color.toString(), false);
    }
}

// LCOV_EXCL_START — no element type currently sets hasAudio=true in its ElementInfo (see
// App/Scene/Commands.cpp's identical exclusion), so the sound field is never shown.
void ElementEditor::audioChanged()
{
    if (m_ui->comboBoxAudio->currentText() != m_manyAudios) {
        setProperty(ElementProperty::Audio, m_ui->comboBoxAudio->currentText(), false);
    }
}
// LCOV_EXCL_STOP

void ElementEditor::frequencyChanged(const double frequency)
{
    if (m_ui->doubleSpinBoxFrequency->text() != m_manyFreq) {
        setProperty(ElementProperty::Frequency, frequency, true);
    }
}

void ElementEditor::delayChanged(const int eighths)
{
    // The slider's -4 to 4 is the phase delay's -0.5 to 0.5 of the period
    setProperty(ElementProperty::Delay, eighths / 8.0, true);
}

void ElementEditor::volumeChanged(const int percent)
{
    setProperty(ElementProperty::Volume, static_cast<float>(percent) / 100.0f, true);
}

void ElementEditor::wirelessModeChanged(const int index)
{
    if (!editable() || (m_ui->comboBoxWirelessMode->currentText() == m_manyWirelessModes)) {
        return;
    }
    const auto mode = static_cast<WirelessMode>(index);
    if (refusesTxChannel(std::nullopt, mode)) {
        return;
    }

    // setWirelessMode() hides the replaced port but does not delete connections
    // itself — that is handled here via DeleteItemsCommand so undo can restore them.
    QList<QGraphicsItem *> wirelessConnsToDelete;
    for (auto *elm : std::as_const(m_elements)) {
        auto *node = qobject_cast<Node *>(elm);
        if (!node) continue;

        Port *port = (mode == WirelessMode::Rx) ? static_cast<Port *>(node->inputPort())
                     : (mode == WirelessMode::Tx) ? static_cast<Port *>(node->outputPort())
                     : nullptr;
        if (port) {
            for (auto *conn : port->connections()) {
                wirelessConnsToDelete.append(static_cast<QGraphicsItem *>(conn));
            }
        }
    }

    // When the mode severs connections, the mode and the delete are one undo step.
    const bool needsMacro = !wirelessConnsToDelete.isEmpty();
    if (needsMacro) {
        m_scene->undoStack()->beginMacro(tr("Change wireless mode"));
    }

    setProperty(ElementProperty::Wireless, index, false);

    if (needsMacro) {
        m_scene->receiveCommand(new DeleteItemsCommand(wirelessConnsToDelete, m_scene));
        m_scene->undoStack()->endMacro();
    }
}

void ElementEditor::applyAppearance(const bool defaultAppearance, const QString &fileName)
{
    if (!editable()) {
        return;
    }
    sentryBreadcrumb("ui", QStringLiteral("Element properties applied"));

    // An appearance is no property SetPropertyCommand sets: the step restores the elements' saved state
    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    for (auto *elm : std::as_const(m_elements)) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
        if (elm->canChangeAppearance()) {
            elm->setAppearance(defaultAppearance, fileName);
        }
    }
    m_scene->receiveCommand(new UpdateCommand(m_elements, oldData, m_scene));
}

void ElementEditor::blobNameEditingFinished()
{
    if (m_elements.isEmpty() || !m_caps.isEmbedded) {
        return;
    }

    const QString newBlobName = m_ui->lineEditBlobName->text().trimmed();
    const QString oldBlobName = m_elements.first()->blobName();

    // editingFinished fires on plain focus-out too, not just on an actual edit — guard against
    // the multi-selection placeholder as well as an empty/unchanged field so clicking into and
    // out of the field without typing anything never renames.
    if (newBlobName.isEmpty() || newBlobName == oldBlobName || newBlobName == m_manyLabels) {
        return;
    }

    // Reject a rename that collides with a different, already-registered blob: ICRegistry::
    // renameBlob() would silently overwrite that other blob's bytes with this IC's circuit,
    // permanently destroying it with no undo path back. Refuse before pushing anything, mirroring
    // the duplicate-wireless-Tx-channel guard above.
    if (m_scene->icRegistry()->hasBlob(newBlobName)) {
        QMessageBox::warning(this,
            tr("Duplicate IC Name"),
            tr("An embedded IC named \"%1\" already exists.\n"
               "Choose a different name.").arg(newBlobName));
        return;
    }

    // Renaming is its own independent, immediately-applied action — pushed standalone rather
    // than folded into apply()'s UpdateCommand, so undo/redo never has to reconcile a generic
    // property snapshot against the IC registry's shared blob-name state.
    m_scene->receiveCommand(new RenameBlobCommand(oldBlobName, newBlobName, m_scene));
}

void ElementEditor::inputIndexChanged(const int index)
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }
    sentryBreadcrumb("ui", QStringLiteral("Input count: %1").arg(index));

    if (m_caps.canChangeInputSize && (m_ui->comboBoxInputSize->currentText() != m_manyIS)) {
        emit sendCommand(new ChangePortSizeCommand(m_elements, m_ui->comboBoxInputSize->currentData().toInt(), m_scene, true));
    }

    qCDebug(zero) << "Input size changed to " << index;
    update();
}

void ElementEditor::outputIndexChanged(const int index)
{
    if (m_elements.isEmpty() || !isEnabled()) {
        return;
    }
    sentryBreadcrumb("ui", QStringLiteral("Output count: %1").arg(index));

    if (m_caps.canChangeOutputSize && (m_ui->comboBoxOutputSize->currentText() != m_manyOS)) {
        emit sendCommand(new ChangePortSizeCommand(m_elements, m_ui->comboBoxOutputSize->currentData().toInt(), m_scene, false));
    }

    qCDebug(zero) << "Output size changed to " << index;
    update();
}

void ElementEditor::outputValueChanged(const QString &value)
{
    // The placeholder of a selection with many values is no value
    if (value != m_manyOV) {
        setProperty(ElementProperty::Value, value.toInt(), false);
    }
}

void ElementEditor::inputLocked(const bool value)
{
    qCDebug(zero) << "Input locked.";
    setProperty(ElementProperty::Locked, value, false);
}

void ElementEditor::triggerChanged(const QString &cmd)
{
    sentryBreadcrumb("ui", QStringLiteral("Trigger key: %1").arg(cmd));
    // A typed key is uppercased; the placeholder for a selection of different triggers is shown as it is
    if (const QString upper = cmd.toUpper(); (cmd != m_manyTriggers) && (upper != cmd)) {
        // Setting the uppercased text emits textChanged again, and that call applies it
        m_ui->lineEditTrigger->setText(upper);
        return;
    }
    if ((cmd != m_manyTriggers) && (cmd.size() <= 1)) {
        setProperty(ElementProperty::Trigger, cmd, true);
    }
}

void ElementEditor::truthTable()
{
    sentryBreadcrumb("ui", QStringLiteral("Truth table editor"));
    if (!m_caps.hasTruthTable || m_elements.isEmpty()) return;

    // Only a single TruthTable element is supported at a time.
    auto *truthtable = qobject_cast<TruthTable *>(m_elements[0]);
    if (!truthtable) {
        return; // LCOV_EXCL_LINE — m_caps.hasTruthTable is AND-merged across the selection and only TruthTable::hasTruthTable() ever returns true, so hasTruthTable implies m_elements[0] already IS a TruthTable.
    }

    // --- Build / refresh table ---
    int nInputs = truthtable->inputSize();
    int nOutputs = truthtable->outputSize();

    // Columns: nInputs input columns (A, B, C, …) + nOutputs output columns (S0, S1, …).
    QStringList inputLabels;
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

        // The key QBitArray stores all outputs interleaved: output z at row i
        // lives at index 256*z + i (256 rows max per output).
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
    // size() != 1 also rejects an empty selection — `> 1` passed for empty
    // and then indexed m_elements[0].
    if (m_elements.size() != 1) return;

    // Input columns (< nInputs) are read-only; only output columns are editable.
    if (column < m_elements[0]->inputSize()) return;

    // Toggle the cell text immediately for visual feedback before the command
    // propagates through the undo stack.
    auto cellItem = m_table->item(row, column);
    cellItem->setText((cellItem->text() == "0") ? "1" : "0");

    // Compute the flat bit index using the same layout as truthTable(): 256*outputCol + row.
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
    sentryBreadcrumb("ui", QStringLiteral("Audio file dialog"));
    if (m_elements.isEmpty()) {
        return;
    }
    auto *audiobox = qobject_cast<AudioBox *>(m_elements[0]);
    if (!audiobox) {
        return;
    }

    const QString filePath = FileDialogs::provider()->getOpenFileName(this, tr("Select any audio"),
                                                    QString(), tr("Audio") + " (*.mp3 *.mp4 *.wav *.ogg)");

    if (filePath.isEmpty()) {
        return;
    }

    // Snapshot and apply via undo command, as the appearance does
    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        audiobox->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }
    audiobox->setAudio(filePath);
    if (m_scene) {
        m_scene->receiveCommand(new UpdateCommand({audiobox}, oldData, m_scene));
    }
    m_ui->lineCurrentAudioBox->setText(QFileInfo(filePath).fileName());
}

void ElementEditor::defaultAppearance()
{
    sentryBreadcrumb("ui", QStringLiteral("Element appearance reset"));
    applyAppearance(true, {});
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
