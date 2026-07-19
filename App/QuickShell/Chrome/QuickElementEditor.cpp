// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickElementEditor.h"

#include <QBuffer>
#include <QDataStream>
#include <QFileInfo>
#include <QImageReader>
#include <QKeySequence>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/UI/FileDialogProvider.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

QuickElementEditor::QuickElementEditor(QObject *parent)
    : QObject(parent)
{
}

void QuickElementEditor::setCanvas(CanvasItem *canvas)
{
    if (m_canvas) {
        disconnect(m_canvas, &CanvasItem::selectionChanged, this, &QuickElementEditor::onSelectionChanged);
    }
    m_canvas = canvas;
    if (m_canvas) {
        connect(m_canvas, &CanvasItem::selectionChanged, this, &QuickElementEditor::onSelectionChanged);
    }
    onSelectionChanged();
}

void QuickElementEditor::onSelectionChanged()
{
    m_elements = m_canvas ? m_canvas->selectedElements() : QList<GraphicElement *>();
    refresh();
}

QString QuickElementEditor::elementTypeTitle() const
{
    return m_elements.isEmpty() ? QString() : ElementFactory::typeToTitleText(m_caps.elementType);
}

void QuickElementEditor::refresh()
{
    if (m_elements.isEmpty()) {
        m_caps = {};
        m_label.clear();
        m_color.clear();
        m_audio.clear();
        m_trigger.clear();
        m_inputSizeOptions.clear();
        m_outputSizeOptions.clear();
        m_outputValueOptions.clear();
        m_appearanceStates.clear();
        m_appearanceStateIndex = 0;
        emit refreshed();
        return;
    }

    m_caps = computeCapabilities(m_elements);
    auto *firstElement = m_elements.constFirst();
    auto *firstInput = qobject_cast<GraphicElementInput *>(firstElement);

    m_label = m_caps.hasLabel ? firstElement->label() : QString();
    m_labelDirty = false;

    m_color = m_caps.hasColors ? firstElement->color() : QString();
    m_colorDirty = false;

    m_audio = m_caps.hasAudio ? firstElement->audio() : QString();
    m_audioDirty = false;

    m_volume = m_caps.hasVolume ? static_cast<int>(firstElement->volume() * 100.0f) : 100;
    m_volumeDirty = false;

    if (m_caps.hasFrequency) {
        // Buzzer uses audible tone range; Clock uses low oscillation range -- mirrors
        // ElementEditor::applyCapabilitiesToUi()'s identical branch exactly.
        const bool isBuzzer = (m_caps.hasSameType && m_caps.elementType == ElementType::Buzzer);
        m_frequencyMin = isBuzzer ? 20.0 : 0.1;
        m_frequencyMax = isBuzzer ? 20000.0 : 50.0;
        m_frequencyStep = isBuzzer ? 10.0 : 0.1;
        m_frequencyDecimals = isBuzzer ? 0 : 1;
        m_frequency = firstElement->frequency();
    }
    m_frequencyDirty = false;

    m_delaySteps = m_caps.hasDelay ? static_cast<int>(firstElement->delay() * 8.0) : 0;
    m_delayDirty = false;

    m_inputSizeOptions.clear();
    if (m_caps.canChangeInputSize) {
        if (m_caps.hasSameType && m_caps.elementType == ElementType::Mux) {
            // Mux: the user picks data inputs (2-8); select lines are computed as
            // ceil(log2(dataInputs)) -- option label shows data inputs, value carries total.
            for (int dataInputs = 2; dataInputs <= 8; ++dataInputs) {
                int selectLines = 1;
                while ((1 << selectLines) < dataInputs) {
                    ++selectLines;
                }
                const int totalInputs = dataInputs + selectLines;
                if (totalInputs >= m_caps.minimumInputs && totalInputs <= m_caps.maximumInputs) {
                    m_inputSizeOptions.append(SizeOption(QString::number(dataInputs), totalInputs));
                }
            }
        } else {
            for (int port = m_caps.minimumInputs; port <= m_caps.maximumInputs; ++port) {
                m_inputSizeOptions.append(SizeOption(QString::number(port), port));
            }
        }
    }
    m_inputSize = firstElement->inputSize();

    m_outputSizeOptions.clear();
    if (m_caps.canChangeOutputSize) {
        if (m_caps.hasSameType && m_caps.elementType == ElementType::Demux) {
            for (int n = m_caps.minimumOutputs; n <= m_caps.maximumOutputs; ++n) {
                m_outputSizeOptions.append(SizeOption(QString::number(n), n));
            }
        } else {
            for (int n : {2, 3, 4, 6, 8, 10, 12, 16}) {
                m_outputSizeOptions.append(SizeOption(QString::number(n), n));
            }
        }
    }
    m_outputSize = firstElement->outputSize();

    m_outputValueOptions.clear();
    if (m_caps.hasLatchedValue) {
        int maxOut = m_caps.maxCurrentOutputSize;
        if (maxOut == 1) {
            ++maxOut;
        }
        for (int val = 0; val < maxOut; ++val) {
            m_outputValueOptions.append(SizeOption(QString::number(val), val));
        }
    }
    m_outputValue = (m_caps.hasLatchedValue && firstInput) ? firstInput->outputValue() : 0;
    m_outputValueDirty = false;

    if (m_caps.hasOnlyInputs && firstInput) {
        // Qt::CheckState: 0 Unchecked, 1 PartiallyChecked, 2 Checked.
        m_lockedState = m_caps.sameCheckState ? (firstInput->isLocked() ? 2 : 0) : 1;
    } else {
        m_lockedState = 0;
    }
    m_lockedDirty = false;

    m_trigger = m_caps.hasTrigger ? firstElement->trigger().toString() : QString();
    m_triggerDirty = false;

    m_wirelessMode = 0;
    if (m_caps.hasWirelessMode) {
        if (auto *node = qobject_cast<Node *>(firstElement)) {
            m_wirelessMode = static_cast<int>(node->wirelessMode());
        }
    }
    m_wirelessModeDirty = false;

    // Mirrors applyCapabilitiesToUi()'s "Populate the appearance state icon grid" block exactly,
    // including its m_elements.size() == 1 gate -- a multi-selection or a single-state element
    // (appearanceStates().size() == 1) both leave this empty, so QML's own
    // "appearanceStates.length > 1" check decides whether to show the tile grid.
    m_appearanceStates.clear();
    m_appearanceStateIndex = 0;
    if (m_caps.canChangeAppearance && m_elements.size() == 1) {
        const auto states = firstElement->appearanceStates();
        if (states.size() > 1) {
            constexpr QSize kTileIconSize(24, 24);
            for (const auto &[index, label] : states) {
                const QImage image = firstElement->appearancePreviewPixmap(index, kTileIconSize).toImage();
                QByteArray bytes;
                QBuffer buffer(&bytes);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, "PNG");
                const QString url = QStringLiteral("data:image/png;base64,") + QString::fromLatin1(bytes.toBase64());
                m_appearanceStates.append(AppearanceStateOption(index, label, url));
            }
        }
    }

    emit refreshed();
}

void QuickElementEditor::apply()
{
    if (m_elements.isEmpty() || !m_canvas) {
        return;
    }

    // Reject label/mode changes that would create a duplicate wireless Tx channel -- mirrors
    // ElementEditor::apply()'s guard exactly. A Tx node's label is its channel name; two Tx
    // nodes sharing one label means one silently receives no signal.
    if (m_caps.hasWirelessMode) {
        for (auto *elm : std::as_const(m_elements)) {
            auto *node = qobject_cast<Node *>(elm);
            if (!node) {
                continue;
            }

            const QString candidateLabel = m_labelDirty ? m_label : node->label();
            const WirelessMode candidateMode = m_wirelessModeDirty ? static_cast<WirelessMode>(m_wirelessMode) : node->wirelessMode();
            if (candidateMode != WirelessMode::Tx || candidateLabel.isEmpty()) {
                continue;
            }

            for (auto *other : m_canvas->elements()) {
                if (other == elm) {
                    continue;
                }
                auto *otherNode = qobject_cast<Node *>(other);
                if (!otherNode || otherNode->wirelessMode() != WirelessMode::Tx) {
                    continue;
                }
                if (otherNode->label() == candidateLabel) {
                    Dialogs::provider()->choice(
                        tr("Duplicate Wireless Channel"),
                        tr("A Tx node with label \"%1\" already exists.\n"
                           "Each wireless channel must have a unique label.").arg(candidateLabel),
                        {DialogButton::Ok}, DialogButton::Ok);
                    refresh();
                    return;
                }
            }
        }
    }

    // Snapshot old state before any mutation below.
    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    for (auto *elm : std::as_const(m_elements)) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }

    // Collect connections that will be severed by a wireless mode change, before it happens --
    // setWirelessMode() hides the replaced port but does not delete connections itself.
    QList<QGraphicsItem *> wirelessConnsToDelete;
    if (m_caps.hasWirelessMode && m_wirelessModeDirty) {
        const auto newMode = static_cast<WirelessMode>(m_wirelessMode);
        for (auto *elm : std::as_const(m_elements)) {
            auto *node = qobject_cast<Node *>(elm);
            if (!node) {
                continue;
            }
            Port *port = (newMode == WirelessMode::Rx) ? static_cast<Port *>(node->inputPort())
                         : (newMode == WirelessMode::Tx) ? static_cast<Port *>(node->outputPort())
                         : nullptr;
            if (port) {
                for (auto *conn : port->connections()) {
                    wirelessConnsToDelete.append(static_cast<QGraphicsItem *>(conn));
                }
            }
        }
    }

    for (auto *elm : std::as_const(m_elements)) {
        applyProperty(elm);
    }

    // Reset the one-shot appearance-update flag after applying to all elements -- mirrors
    // ElementEditor::apply()'s identical reset.
    if (m_isUpdatingAppearance) {
        m_isUpdatingAppearance = false;
    }

    const bool needsMacro = !wirelessConnsToDelete.isEmpty();
    if (needsMacro) {
        m_canvas->undoStack()->beginMacro(tr("Change wireless mode"));
    }

    m_canvas->receiveCommand(new CanvasUpdateCommand(m_elements, oldData, m_canvas));

    if (needsMacro) {
        m_canvas->receiveCommand(new CanvasDeleteItemsCommand(wirelessConnsToDelete, m_canvas));
        m_canvas->undoStack()->endMacro();
    }
}

void QuickElementEditor::applyProperty(GraphicElement *elm)
{
    if (m_labelDirty && m_caps.hasLabel) {
        elm->setLabel(m_label);
    }
    if (m_colorDirty && m_caps.hasColors) {
        elm->setColor(m_color);
    }
    if (m_frequencyDirty && m_caps.hasFrequency) {
        elm->setFrequency(m_frequency);
    }
    if (m_delayDirty && m_caps.hasDelay) {
        elm->setDelay(static_cast<double>(m_delaySteps) / 8.0);
    }
    if (m_audioDirty && m_caps.hasAudio) {
        elm->setAudio(m_audio);
    }
    if (m_volumeDirty && m_caps.hasVolume) {
        elm->setVolume(static_cast<float>(m_volume) / 100.0f);
    }
    if (m_triggerDirty && m_caps.hasTrigger) {
        elm->setTrigger(QKeySequence(m_trigger));
    }
    if (m_isUpdatingAppearance) {
        elm->setAppearance(m_isDefaultAppearance, m_appearanceName);
    }
    if (m_wirelessModeDirty && m_caps.hasWirelessMode) {
        if (auto *node = qobject_cast<Node *>(elm)) {
            node->setWirelessMode(static_cast<WirelessMode>(m_wirelessMode));
        }
    }
    if (m_lockedDirty && m_caps.hasOnlyInputs) {
        if (auto *input = qobject_cast<GraphicElementInput *>(elm)) {
            input->setLocked(m_lockedState == 2);
        }
    }
    if (m_outputValueDirty && m_caps.hasLatchedValue) {
        if (elm->elementType() == ElementType::InputRotary) {
            if (auto *inputRotary = qobject_cast<InputRotary *>(elm)) {
                inputRotary->setOn(true, m_outputValue);
            }
        } else if (auto *input = qobject_cast<GraphicElementInput *>(elm)) {
            input->setOn(static_cast<bool>(m_outputValue));
        }
    }
}

void QuickElementEditor::setLabel(const QString &value)
{
    if (!m_caps.hasLabel || value == m_label) {
        return;
    }
    m_label = value;
    m_labelDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setColor(const QString &value)
{
    if (!m_caps.hasColors || value == m_color) {
        return;
    }
    m_color = value;
    m_colorDirty = true;
    emit refreshed();
    apply();
}

QList<ColorOption> QuickElementEditor::colorOptions()
{
    // Mirrors ElementEditor::fillColorComboBox() -- name is the internal English identifier
    // GraphicElement::setColor() expects; translatedName is display-only.
    return {
        ColorOption("White", tr("White"), "qrc:/Components/Output/Led/WhiteLed.svg"),
        ColorOption("Red", tr("Red"), "qrc:/Components/Output/Led/RedLed.svg"),
        ColorOption("Green", tr("Green"), "qrc:/Components/Output/Led/GreenLed.svg"),
        ColorOption("Blue", tr("Blue"), "qrc:/Components/Output/Led/BlueLed.svg"),
        ColorOption("Purple", tr("Purple"), "qrc:/Components/Output/Led/PurpleLed.svg"),
    };
}

void QuickElementEditor::setAudio(const QString &value)
{
    if (!m_caps.hasAudio || value == m_audio) {
        return;
    }
    m_audio = value;
    m_audioDirty = true;
    emit refreshed();
    apply();
}

QVariantList QuickElementEditor::audioOptions()
{
    // Mirrors ElementEditorUI's 8 fixed comboBoxAudio notes (C6..C7).
    return {"C6", "D6", "E6", "F6", "G6", "A6", "B6", "C7"};
}

QString QuickElementEditor::audioBoxFileName() const
{
    if (!m_caps.hasAudioBox || m_elements.isEmpty()) {
        return QString();
    }
    // Mirrors applyCapabilitiesToUi()'s unconditional multi-selection placeholder exactly
    // (m_elements.size() > 1 ? m_manyAudios : m_elements[0]->audio()) -- no same-value
    // consensus check, unlike every other field in this class.
    if (m_elements.size() > 1) {
        return tr("<Multiple>");
    }
    return QFileInfo(m_elements.constFirst()->audio()).fileName();
}

void QuickElementEditor::pickAudioBoxFile()
{
    if (m_elements.isEmpty()) {
        return;
    }
    auto *audioBox = qobject_cast<AudioBox *>(m_elements.constFirst());
    if (!audioBox) {
        return;
    }

    const QString filePath = FileDialogs::provider()->getOpenFileName(
        nullptr, tr("Select any audio"), QString(), tr("Audio") + " (*.mp3 *.mp4 *.wav *.ogg)");
    if (filePath.isEmpty()) {
        return;
    }

    // Not wrapped in a CanvasUpdateCommand -- mirrors ElementEditor::audioBox()'s real behavior
    // exactly, a genuine production quirk (this change isn't undoable there either).
    audioBox->setAudio(filePath);
    emit refreshed();
}

void QuickElementEditor::setVolume(int value)
{
    if (!m_caps.hasVolume || value == m_volume) {
        return;
    }
    m_volume = value;
    m_volumeDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setFrequency(double value)
{
    if (!m_caps.hasFrequency || qFuzzyCompare(value, m_frequency)) {
        return;
    }
    m_frequency = value;
    m_frequencyDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setDelaySteps(int value)
{
    if (!m_caps.hasDelay || value == m_delaySteps) {
        return;
    }
    m_delaySteps = value;
    m_delayDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setInputSize(int value)
{
    // Input/output size changes push their own CanvasChangePortSizeCommand, mirroring
    // ElementEditor::inputIndexChanged() -- NOT part of the general UpdateCommand apply()
    // pushes, since PropertyDescriptor::Type has no size-related member (port count is a
    // structural change, not a plain property write).
    if (!m_caps.canChangeInputSize || value == m_inputSize || m_elements.isEmpty() || !m_canvas) {
        return;
    }
    m_canvas->receiveCommand(new CanvasChangePortSizeCommand(m_elements, value, m_canvas, true));
    refresh();
}

void QuickElementEditor::setOutputSize(int value)
{
    if (!m_caps.canChangeOutputSize || value == m_outputSize || m_elements.isEmpty() || !m_canvas) {
        return;
    }
    m_canvas->receiveCommand(new CanvasChangePortSizeCommand(m_elements, value, m_canvas, false));
    refresh();
}

void QuickElementEditor::setOutputValue(int value)
{
    if (!m_caps.hasLatchedValue || value == m_outputValue) {
        return;
    }
    m_outputValue = value;
    m_outputValueDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setLockedState(int value)
{
    if (!m_caps.hasOnlyInputs || value == m_lockedState) {
        return;
    }
    m_lockedState = value;
    m_lockedDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setTrigger(const QString &value)
{
    if (!m_caps.hasTrigger) {
        return;
    }
    // Force uppercase so the stored key sequence matches Qt's key names across layouts --
    // mirrors ElementEditor::triggerChanged(). A single alphanumeric character only.
    const QString upper = value.toUpper();
    if (upper == m_trigger || upper.size() > 1) {
        return;
    }
    m_trigger = upper;
    m_triggerDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setWirelessMode(int value)
{
    if (!m_caps.hasWirelessMode || value == m_wirelessMode) {
        return;
    }
    m_wirelessMode = value;
    m_wirelessModeDirty = true;
    emit refreshed();
    apply();
}

QVariantList QuickElementEditor::wirelessModeOptions()
{
    return {tr("None"), tr("Transmit (Tx)"), tr("Receive (Rx)")};
}

void QuickElementEditor::setAppearanceStateIndex(int index)
{
    if (index == m_appearanceStateIndex) {
        return;
    }
    m_appearanceStateIndex = index;
    emit refreshed();
}

void QuickElementEditor::changeAppearance()
{
    if (m_elements.isEmpty() || !m_canvas) {
        return;
    }

    const QString imageFilter = tr("Images") + " (*." + QImageReader::supportedImageFormats().join(" *.") + ")";
    const QString fileName = FileDialogs::provider()->getOpenFileName(nullptr, tr("Open File"), QString(), imageFilter);
    if (fileName.isEmpty()) {
        return;
    }

    // A tile is selected in a real multi-state grid: apply directly to that state index via a
    // standalone CanvasUpdateCommand, mirroring ElementEditor::updateElementAppearance()'s
    // tile-grid branch exactly -- bypasses the generic apply()/applyProperty() dispatch, which
    // has no per-index concept.
    if (m_appearanceStates.size() > 1) {
        auto *elm = m_elements.constFirst();
        QByteArray oldData;
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
        elm->setAppearanceAt(m_appearanceStateIndex, fileName);
        m_canvas->receiveCommand(new CanvasUpdateCommand({elm}, oldData, m_canvas));
        refresh();
        return;
    }

    // Single-state element: the generic one-shot flag path, applied through the normal
    // apply()/applyProperty() dispatch -- mirrors updateElementAppearance()'s fallback branch.
    m_isUpdatingAppearance = true;
    m_isDefaultAppearance = false;
    m_appearanceName = fileName;
    apply();
}

void QuickElementEditor::resetAppearance()
{
    if (m_elements.isEmpty() || !m_canvas) {
        return;
    }
    // Mirrors ElementEditor::defaultAppearance(): flags set before apply() so it sees them
    // during the per-element loop and passes true/empty to setAppearance().
    m_isUpdatingAppearance = true;
    m_isDefaultAppearance = true;
    apply();
}

void QuickElementEditor::rebuildTruthTableRows(TruthTable *table)
{
    const int nInputs = table->inputSize();
    const int nOutputs = table->outputSize();

    m_truthTableColumnLabels.clear();
    for (int i = 0; i < nInputs; ++i) {
        m_truthTableColumnLabels.append(QChar::fromLatin1(static_cast<char>('A' + i)));
    }
    for (int z = 0; z < nOutputs; ++z) {
        m_truthTableColumnLabels.append(QStringLiteral("S") + QString::number(z));
    }
    m_truthTableInputCount = nInputs;

    const auto &key = table->key();
    m_truthTableRows.clear();
    const int rowCount = 1 << nInputs;
    m_truthTableRows.reserve(rowCount);
    for (int i = 0; i < rowCount; ++i) {
        // Binary representation of the row index, zero-padded to nInputs digits -- matches
        // ElementEditor::truthTable()'s QString::number(i, 2).rightJustified(nInputs, '0').
        QString inputBits = QString::number(i, 2);
        if (inputBits.size() < nInputs) {
            inputBits = inputBits.rightJustified(nInputs, '0');
        }

        QStringList cells;
        cells.reserve(nInputs + nOutputs);
        for (int j = 0; j < nInputs; ++j) {
            cells.append(QString(inputBits.at(j)));
        }
        for (int z = 0; z < nOutputs; ++z) {
            // The key QBitArray stores all outputs interleaved: output z at row i lives at
            // index 256*z + i (256 rows max per output) -- matches TruthTable's own layout.
            cells.append(key.at(256 * z + i) ? QStringLiteral("1") : QStringLiteral("0"));
        }
        m_truthTableRows.append(TruthTableRow(cells));
    }
}

void QuickElementEditor::openTruthTable()
{
    if (!m_caps.hasTruthTable || m_elements.isEmpty()) {
        return;
    }
    // Only a single TruthTable element is supported at a time -- mirrors
    // ElementEditor::truthTable()'s identical restriction.
    auto *table = qobject_cast<TruthTable *>(m_elements.constFirst());
    if (!table) {
        return;
    }

    rebuildTruthTableRows(table);
    emit truthTableChanged();
    emit truthTableRequested();
}

void QuickElementEditor::toggleTruthTableCell(int row, int column)
{
    // Mirrors setTruthTableProposition()'s "size() != 1" guard (also rejects an empty
    // selection, since indexing m_elements[0] below would be invalid otherwise).
    if (m_elements.size() != 1 || !m_canvas) {
        return;
    }
    auto *table = qobject_cast<TruthTable *>(m_elements.constFirst());
    if (!table) {
        return;
    }
    // Input columns are read-only; only output columns are editable.
    if (column < table->inputSize()) {
        return;
    }

    const int positionToChange = 256 * (column - table->inputSize()) + row;
    m_canvas->receiveCommand(new CanvasToggleTruthTableOutputCommand(table, positionToChange, m_canvas));

    rebuildTruthTableRows(table);
    emit truthTableChanged();
}

void QuickElementEditor::prepareContextMenu(GraphicElement *item)
{
    m_morphCandidates.clear();

    if (item && m_caps.canMorph) {
        // hasSameType here is the WHOLE selection's flag (matches addElementAction()'s own
        // "hasSameType && selectedElm->elementType() == type" guard exactly) -- a mixed-type
        // selection excludes nothing, since the exclusion only means "don't offer morphing to
        // the type everything already is."
        const bool hasSameType = m_caps.hasSameType;
        const auto addCandidate = [this, item, hasSameType](ElementType type) {
            if (hasSameType && item->elementType() == type) {
                return;
            }
            m_morphCandidates.append(MorphCandidate(
                static_cast<int>(type), ElementFactory::translatedName(type),
                QStringLiteral("qrc") + ElementMetadataRegistry::metadata(type).pixmapPath()));
        };

        // Mirrors ElementContextMenu::exec()'s per-ElementGroup switch exactly.
        switch (item->elementGroup()) {
        case ElementGroup::Gate:
            if (item->inputSize() == 1) {
                addCandidate(ElementType::Node);
                addCandidate(ElementType::Not);
            } else {
                addCandidate(ElementType::And);
                addCandidate(ElementType::Nand);
                addCandidate(ElementType::Nor);
                addCandidate(ElementType::Or);
                addCandidate(ElementType::Xnor);
                addCandidate(ElementType::Xor);
            }
            break;

        case ElementGroup::StaticInput:
        case ElementGroup::Input:
            addCandidate(ElementType::Clock);
            addCandidate(ElementType::InputButton);
            addCandidate(ElementType::InputGnd);
            addCandidate(ElementType::InputRotary);
            addCandidate(ElementType::InputSwitch);
            addCandidate(ElementType::InputVcc);
            break;

        case ElementGroup::Memory:
            // 2 inputs -> DLatch; 4 inputs -> DFF/TFF; 5 inputs -> JKFF/SRFF.
            if (item->inputSize() == 2) {
                addCandidate(ElementType::DLatch);
            } else if (item->inputSize() == 4) {
                addCandidate(ElementType::DFlipFlop);
                addCandidate(ElementType::TFlipFlop);
            } else if (item->inputSize() == 5) {
                addCandidate(ElementType::JKFlipFlop);
                addCandidate(ElementType::SRFlipFlop);
            }
            break;

        case ElementGroup::Output:
            if (item->elementType() == ElementType::Display7 || item->elementType() == ElementType::Display14
                || item->elementType() == ElementType::Display16) {
                addCandidate(ElementType::Display7);
                addCandidate(ElementType::Display14);
                addCandidate(ElementType::Display16);
            } else {
                addCandidate(ElementType::Buzzer);
                addCandidate(ElementType::Led);
            }
            break;

        case ElementGroup::IC:
        case ElementGroup::Mux:
        case ElementGroup::Other:
        case ElementGroup::Unknown:
            break;

        default:
            break;
        }
    }

    emit refreshed();
}

void QuickElementEditor::morphSelectionTo(int type)
{
    if (!m_canvas) {
        return;
    }
    m_canvas->morphSelectionTo(static_cast<ElementType>(type));
}

QString QuickElementEditor::blobName() const
{
    return (m_caps.isEmbedded && !m_elements.isEmpty()) ? m_elements.constFirst()->blobName() : QString();
}

void QuickElementEditor::commitBlobRename(const QString &newName)
{
    if (!m_canvas || !m_caps.isEmbedded || m_elements.isEmpty()) {
        return;
    }

    const QString trimmed = newName.trimmed();
    const QString oldName = m_elements.constFirst()->blobName();

    // editingFinished fires on plain focus-out too, not just on an actual edit -- mirrors
    // blobNameEditingFinished()'s empty/unchanged guard exactly.
    if (trimmed.isEmpty() || trimmed == oldName) {
        return;
    }

    // A rename colliding with a different, already-registered blob would silently overwrite
    // that other blob's bytes with this IC's circuit, permanently destroying it with no undo
    // path back -- refuse before pushing anything, mirroring the duplicate-wireless-Tx-channel
    // guard in apply() above.
    if (m_canvas->icRegistry()->hasBlob(trimmed)) {
        Dialogs::provider()->choice(
            tr("Duplicate IC Name"),
            tr("An embedded IC named \"%1\" already exists.\n"
               "Choose a different name.").arg(trimmed),
            {DialogButton::Ok}, DialogButton::Ok);
        emit refreshed();
        return;
    }

    m_canvas->receiveCommand(new CanvasRenameBlobCommand(oldName, trimmed, m_canvas));
}
